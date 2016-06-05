

#include "Parser.h"
using Poco::RegularExpression;

namespace ofx { namespace vezer{
    
#pragma mark - Proc
    Proc::Proc(){
        type = 0;
    }
    
    Proc::Proc(string addr, int f, int32_t val){
        frame = f;
        type = TYPE_INT;
        setAddress(addr);
        addIntArg(val);
    }
    
    Proc::Proc(string addr, int f, double val){
        frame = f;
        type = TYPE_FLOAT;
        setAddress(addr);
        addFloatArg(val);
    }
    
    Proc::Proc(string addr, int f, float r, float g, float b){
        frame = f;
        type = TYPE_FLOAT_COLOR;
        setAddress(addr);
        addFloatArg(r);
        addFloatArg(g);
        addFloatArg(b);
        
    }
    
    Proc::Proc(string addr, int f, string s){
        frame = f;
        type = TYPE_FLAG;
        vector<string> args = ofSplitString(s, ",");
        if ( args.size() == 1 ) {
            setAddress(s);
        } else {
            setAddress(args[0]);
            
            for ( int i=1; i<args.size(); i++ ) {
                //cout << args[i] << "," << isNumberString(args[i]) <<  endl;
                if ( isNumberString(args[i]) ) {
                    float val = ofFromString<float>(args[i]);
                    addFloatArg(val);
                } else {
                    addStringArg(args[i]);
                }
            }
        }
        
    }
    
    bool Proc::isNumberString(const string & s){
        RegularExpression regEx("^[+-]?([0-9]*\.[0-9]+|[0-9]+\.?[0-9]*)([eE][+-]?[0-9]+|)");
        return regEx.match(s);
    }
    
    void Proc::copy( const Proc& other ){
        frame = other.frame;
        type = other.type;
        ofxOscMessage::copy(other);
    }

#pragma mark - Track
    Track::Track(){
        state = true;
        address = "";
        name = "";
        type = OSC_VALUE_INT;
        min = 0.0;
        max = 1.0;
        
        current = -1;
        process.clear();
        
        group_index = 0;
    }
    
    Proc& Track::getCurrentProc(){
        return process[ofClamp(current, 0, process.size()-1)];
    }
    
    void Track::sortProcess(){
        ofSort(process, compareByFrame);
    }
    
    
    bool Track::getProcess(int frame, Proc * proc){
        if ( !state ) return false;
        if ( process.empty() ) return false;
        bool b = false;
        
        //            if ( process.find(frame) != process.end() ) {
        //                proc->copy(process[frame]);
        //                b = true;
        //            }
        
        int index = getProcessIndex(frame);
        if ( index > -1 ) {
            proc->copy(process[index]);
            b = true;
        }
        
        return b;
    }
    
    //        int Track::getProcessIndex(int frame){
    //            if ( process.find(frame) != process.end() ) {
    //                return 0;
    //            } else {
    //                return -1;
    //            }
    //        }
    int Track::getProcessIndex(int frame){
        if ( !state ) return -1;
        if ( process.empty() ) return -1;
        if ( current > frame ) current = -1;
        if ( frame == 0 ) current = -1;
        bool b = false;
        
        int num = process.size();
        for ( int i=num-1; i>=0; i-- ) {
            int f = process[i].frame;
            if ( frame >= f ) {
                if ( current != i  ) {
                    current = i;
                    //cout << _current << endl;
                    b = true;
                }
                break;
            }
        }
        return b ? current : -1;
    }
    
    bool Track::getFirstProcess(Proc * proc){
        if ( process.empty() ) return false;
        else {
            proc->copy(process[0]);
            //                proc->copy(process.begin()->second);
            return true;
        }
    }
    
    bool Track::getLastProcess(Proc * proc){
        if ( process.empty() ) return false;
        else {
            proc->copy(process.back());
            //                proc->copy((process.end()--)->second);
            return true;
        }
    }
    
    bool Track::getLazyFrameProcess(int frame, Proc * proc){
        if ( !state ) return false;
        if ( process.empty() ) return false;
        bool b = false;
        
        //            if ( process.find(frame) != process.end() ) {
        //                proc->copy(process[frame]);
        //                b = true;
        //            } else {
        //                while ( process.find(frame) == process.end() ) {
        //                    frame--;
        //                    if ( frame < 0 ) break;
        //                }
        //                if ( frame >= 0 ) {
        //                    proc->copy(process[frame]);
        //                    b = true;
        //                }
        //            }
        //
        int index = getProcessIndex(frame);
        if ( index > -1 ) {
            proc->copy(process[index]);
            b = true;
        } else {
            while ( index != -1 ) {
                frame--;
                index = getProcessIndex(frame);
                if ( frame < 0 ) break;
            }
            if ( frame >= 0 ) {
                proc->copy(process[index]);
                b = true;
            }
        }
        
        return b;
    }
    
#pragma mark - Composition
    vector<Track>& Composition::getCurrentTracks(int frame){
        tmp_tracks.clear();
        int num = tracks.size();
        for ( int i=0; i<num; i++ ) {
            Track& track = tracks[i];
            if ( track.getProcessIndex(frame) > -1 ) {
                tmp_tracks.push_back(track);
            }
        }
        return tmp_tracks;
    }
    
    bool Composition::getTrackByName(string name, Track & track){
        for ( int i=0; i<tracks.size(); i++ ) {
            if ( tracks[i].name == name ) {
                track = tracks[i];
                return true;
            }
        }
        return false;
    }
    
    bool Composition::combine(const Composition & other, bool priority){
        if ( fps != other.fps ) {
            //dont combine different fps Compositions that change the time speed.
            return false;
        }
        if ( priority ) {
            state = other.state;
            name = other.name;
            bpm = other.bpm;
            loop = other.loop;
        }
        if ( length < other.length ) length = other.length;
        if ( start > other.start ) start = other.start;
        if ( end < other.end ) end = other.end;
        
        for ( int i=0; i<other.tracks.size(); i++ ) {
            tracks.emplace_back(other.tracks[i]);
        }
        
    }

    
#pragma mark - Parser
    Parser::Parser () : midi_as_osc(true){}
    
    vector<Composition> & Parser::getCompositions(){ return compositions; }
    Composition & Parser::getComposition(int index) { return compositions[index]; }

    void Parser::debugDraw(){
        if ( fbo.isAllocated() ) {
            ofEnableAlphaBlending();
            ofSetColor(255);
            fbo.draw(0,0);
        }
    }
    
    ofFbo Parser::redraw(Composition comp){
        float w = ofGetWidth();
        float h = ofGetHeight();
        
        fbo.allocate(w, h, GL_RGBA);
        fbo.begin();
        ofEnableAlphaBlending();
        ofClear(0);
        
        float scale = w / float(comp.length);
        float hh = h / float(comp.tracks.size());
        if ( hh < 40 ) hh = 40;
        
        for ( int i=0; i<comp.tracks.size(); i++ ) {
            ofx::vezer::Track track = comp.tracks[i];
            
            ofFill();
            ofSetColor(0, 255, 0, 60);
            ofRect(0, i*hh + 5, w, (hh - 5));
            ofNoFill();
            ofBeginShape();
            
            for(int j=0; j<track.process.size(); j++) {
                int frame = track.process[j].frame;
                Proc proc = track.process[j];
                if ( proc.type == Proc::TYPE_FLOAT ) {
                    float val = ofMap( proc.getArgAsFloat(0), track.min, track.max, 0., 1.0);
                    ofSetColor(255);
                    float x = frame*scale;
                    float y = (i+1)*hh - val * (hh - 5);
                    ofVertex(x, y);
                    
                    //ofCircle(frame*scale, (i+1)*hh - val * (hh - 5) , 2);
                } else if ( proc.type == Proc::TYPE_INT ) {
                    float val = ofMap( proc.getArgAsInt32(0), track.min, track.max, 0., 1.0);
                    ofSetColor(255);
                    float x = frame*scale;
                    float y = (i+1)*hh - val * (hh - 5);
                    ofVertex(x, y);
                    
                    //ofCircle(frame*scale, (i+1)*hh - val * (hh - 5) , 2);
                } else if ( proc.type == Proc::TYPE_FLAG ) {
                    ofSetColor(255);
                    float x = frame*scale;
                    float y = (i+1)*hh - (hh - 5);
                    ofFill();
                    ofRect(x, y, 1, (hh - 5));
                } else if ( proc.type == Proc::TYPE_FLOAT_COLOR ) {
                    ofSetColor(proc.getArgAsFloat(0)*255, proc.getArgAsFloat(1)*255, proc.getArgAsFloat(2)*255);
                    float x = frame*scale;
                    float y = (i+1)*hh - (hh - 5);
                    ofFill();
                    ofRect(x, y, scale, (hh - 5));
                }
            }
            
            ofEndShape();
            ofSetColor(0,255,0);
            ofDrawBitmapString(track.name + " : " + track.address, 10, i*hh+15);
        }
        
        fbo.end();
        return fbo;
    }
    
    vector<Composition> Parser::load(string path){
        vector<Composition> result;
        ofxXmlSettings xml;
        if ( !xml.load(path) ) return result;
        
        if ( xml.pushTag("compositions") ) {
            int numCompositoin = xml.getNumTags("composition");
            for ( int i=0; i<numCompositoin; i++ ) {
                xml.pushTag("composition", i);
                
                Composition comp;
                comp.state = xml.getValue("state", "on") == "on";
                comp.name = xml.getValue("name", "compositon");
                comp.fps = xml.getValue("fps", 30.0);
                comp.length = xml.getValue("length", 1);
                comp.bpm = xml.getValue("bpm", 120.0);
                comp.loop = xml.getValue("loop", "on") == "on";
                comp.start = xml.getValue("start", 0);
                comp.end = xml.getValue("end", 1);
                //cout << comp.name << endl;
                xml.pushTag("tracks");
                int numTrack = xml.getNumTags("track");
                //cout << numTrack << endl;
                for ( int j=0; j<numTrack; j++ ) {
                    if ( xml.pushTag("track", j) ) {
                        if ( xml.tagExists("state") ) {
                            Track track;
                            track.state = xml.getValue("state", "on") == "on";
                            track.name = xml.getValue("name", "Track");
                            track.type = xml.getValue("type", OSC_VALUE_FLOAT);
                            track.min = xml.getValue("min", 0.0);
                            track.max = xml.getValue("max", 1.0);
                            
                            if ( xml.pushTag("target") ) {
                                if ( midi_as_osc && track.type == MIDI_NOTES ) {
                                    track.address = track.name;
                                } else {
                                    track.address = xml.getValue("address", "/exapmle");
                                }
                                xml.popTag();
                            }
                            //scout << track.address << endl;
                            if ( xml.pushTag("process") ) {
                                for ( int k=0; k<=comp.length; k++ ) {
                                    string n = "f" + ofToString(k);
                                    if ( xml.tagExists(n) ) {
                                        if ( track.type == OSC_VALUE_INT || track.type == MIDI_NOTES ) {
                                            track.process.push_back(Proc(track.address, k, (int)xml.getValue(n, 0) ));
                                        } else if ( track.type == OSC_VALUE_FLOAT ) {
                                            track.process.push_back(Proc(track.address, k, xml.getValue(n, 0.0)));
                                        } else if ( track.type == OSC_FLAG ) {
                                            track.process.push_back(Proc(track.address, k, xml.getValue(n, "")));
                                        } else if ( track.type == OSC_COLOR_STANDARD || track.type == OSC_COLOR_FLOATARRAY || track.type == OSC_COLOR_MULTIFLOAT ) {
                                            string s = xml.getValue(n, "0,0,0");
                                            vector<string> arr = ofSplitString(s, ",");
                                            if ( arr.size() >= 3 ) {
                                                float r = ofToFloat(arr[0]);
                                                float g = ofToFloat(arr[1]);
                                                float b = ofToFloat(arr[2]);
                                                track.process.push_back(Proc(track.address, k, r, g, b ));
                                            }
                                        } else if ( track.type == OSC_COLOR_INTARRAY || track.type == OSC_COLOR_MULTIINT ) {
                                            string s = xml.getValue(n, "0,0,0");
                                            vector<string> arr = ofSplitString(s, ",");
                                            if ( arr.size() >= 3 ) {
                                                float r = ofToFloat(arr[0]) / 255.0;
                                                float g = ofToFloat(arr[1]) / 255.0;
                                                float b = ofToFloat(arr[2]) / 255.0;
                                                track.process.push_back(Proc(track.address, k, r, g, b ));
                                            }
                                        }
                                    }
                                }
                                xml.popTag();
                            }
                            comp.tracks.push_back(track);
                        }
                        xml.popTag();
                    }
                }
                xml.popTag();
                xml.popTag();
                mergeGroupTracks(comp.tracks);
                result.push_back(comp);
            }
        }
        compositions = result;
        return result;
    }

    
    void Parser::mergeGroupTracks(vector<Track> & tracks){//todo
        vector<string> grouped_addresses;
        map<string, vector<Track> > grouped_tracks;
        vector<Track> use_tracks;
        for ( int i=0; i<tracks.size(); i++ ) {
            Track track = tracks[i];
            vector<string> addr_vals = ofSplitString(track.address, "#");
            if ( addr_vals.size() == 2 && !track.process.empty() ) {
                if ( track.state ) {
                    int index = ofFromString<int>(addr_vals[1]);
                    string addr = addr_vals[0];
                    track.group_index = index;
                    track.address = addr;
                    
                    if ( !ofContains(grouped_addresses, addr) ) grouped_addresses.push_back(addr);
                    
                    grouped_tracks[addr].push_back(track);
                }
            } else {
                if ( !track.process.empty() ) use_tracks.push_back(track);
            }
        }
        
        for ( auto s : grouped_addresses ) {
            if ( grouped_tracks[s].size() == 1 ) continue;
            
            ofSort(grouped_tracks[s], compareGroupIndex);
            
            Track & t = grouped_tracks[s][0];
            
            for ( int i=1; i<grouped_tracks[s].size(); i++ ) {
                Track track = grouped_tracks[s][i];
                
                Proc a;
                Proc b;
                if ( t.getFirstProcess(&a) && track.getFirstProcess(&b) ) {
                    if ( a.frame < b.frame ) {
                        while( a.frame < b.frame ){
                            b.frame--;
                            track.process.push_back(b);
                        }
                        t.sortProcess();
                    } else if ( a.frame > b.frame ) {
                        while( a.frame > b.frame ){
                            a.frame--;
                            t.process.push_back(a);
                        }
                        t.sortProcess();
                    }
                }
                if ( t.getLastProcess(&a) && track.getLastProcess(&b) ) {
                    if ( a.frame > b.frame ) {
                        while( a.frame > b.frame ){
                            b.frame++;
                            track.process.push_back(b);
                        }
                        t.sortProcess();
                    } else if ( a.frame < b.frame ) {
                        while( a.frame < b.frame ){
                            a.frame++;
                            t.process.push_back(a);
                        }
                        t.sortProcess();
                    }
                }
            }
            
            for ( int j=0; j<t.process.size(); j++ ) {
                Proc & a = t.process[j];
                a.setAddress(t.address);
                for ( int i=1; i<grouped_tracks[s].size(); i++ ) {
                    Track track = grouped_tracks[s][i];
                    
                    Proc b;
                    if ( track.getProcess(a.frame, &b) ) {
                        if ( b.getArgType(0) == OFXOSC_TYPE_FLOAT ) {
                            a.addFloatArg(b.getArgAsFloat(0));
                        } else if ( b.getArgType(0) == OFXOSC_TYPE_INT32 ) {
                            a.addIntArg(b.getArgAsInt32(0));
                        }
                    }
                    
                }
            }
            
            use_tracks.push_back(t);
        }
        tracks = use_tracks;
    }
    
    
}}

namespace ofxVezer = ofx::vezer;
