#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "ofxOscMessage.h"

#include "Poco/RegularExpression.h"
using Poco::RegularExpression;

namespace ofx { namespace vezer{
    
    class Parser;
    
    const string OSC_VALUE_INT = "OSCValue/int";
    const string OSC_VALUE_FLOAT = "OSCValue/float";
    const string OSC_FLAG = "OSCFlag";
     
    const string OSC_COLOR_STANDARD = "OSCColor/standard";
    const string OSC_COLOR_INTARRAY = "OSCColor/intarray";
    const string OSC_COLOR_FLOATARRAY = "OSCColor/floatarray";
    const string OSC_COLOR_MULTIINT = "OSCColor/multiint";
    const string OSC_COLOR_MULTIFLOAT = "OSCColor/multifloat";
    
    class Proc : public ofxOscMessage {
    public:
        int frame;
        int type;
        
    public:
        friend Parser;
        
        enum TYPE_OF_PARAMES {
            TYPE_INT,
            TYPE_FLOAT,
            TYPE_FLOAT_COLOR,
            TYPE_FLAG,
        };
        
        Proc(){
            type = 0;
        }
        
        Proc(string addr, int f, int32_t val){
            frame = f;
            type = TYPE_INT;
            setAddress(addr);
            addIntArg(val);
        }
        
        Proc(string addr, int f, double val){
            frame = f;
            type = TYPE_FLOAT;
            setAddress(addr);
            addFloatArg(val);
        }
        
        Proc(string addr, int f, float r, float g, float b){
            frame = f;
            type = TYPE_FLOAT_COLOR;
            setAddress(addr);
            addFloatArg(r);
            addFloatArg(g);
            addFloatArg(b);
            
        }
        
        Proc(string addr, int f, string s){
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
        
        bool isNumberString(const string & s){
            RegularExpression regEx("^[+-]?([0-9]*\.[0-9]+|[0-9]+\.?[0-9]*)([eE][+-]?[0-9]+|)");
            return regEx.match(s);
        }
        
        void copy( const Proc& other ){
            frame = other.frame;
            type = other.type;
            ofxOscMessage::copy(other);
        }
    };
        
    class Track{
    public:
        bool isGroup;
        int group_index;
        bool state;
        string address;
        string name;
        string type;
        float min;
        float max;
        
        int current;
        vector<Proc> process;
        
        Track(){
            state = true;
            address = "";
            name = "";
            type = OSC_VALUE_INT;
            min = 0.0;
            max = 1.0;
            
            current = -1;
            process.clear();
            
            isGroup = false;
            group_index = 0;
        }
        
        Proc& getCurrentProc(){
            return process[ofClamp(current, 0, process.size()-1)];
        }
        
        bool getProcess(int frame, Proc * proc){
            if ( !state ) return false;
            if ( process.empty() ) return false;
            bool b = false;
            
            int index = getProcessIndex(frame);
            if ( index > -1 ) {
                proc->copy(process[index]);
                b = true;
            }
            
            return b;
        }
        
        int getProcessIndex(int frame){
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
        
        bool getFirstProcess(Proc * proc){
            if ( process.empty() ) return false;
            else {
                proc->copy(process[0]);
                return true;
            }
        }
        
        bool getLastProcess(Proc * proc){
            if ( process.empty() ) return false;
            else {
                proc->copy(process.back());
                return true;
            }
        }
        
        bool getLazyFrameProcess(int frame, Proc * proc){
            if ( !state ) return false;
            if ( process.empty() ) return false;
            bool b = false;
            
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
    };
        
    class Composition{
    public:
        bool state;
        string name;
        float fps;
        int length;
        float bpm;
        bool loop;
        int start;
        int end;
        vector<Track> tracks;
        vector<Track> tmp_tracks;
        
        vector<Track>& getCurrentTracks(int frame){
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
        
        bool getTrackByName(string name, Track & track){
            for ( int i=0; i<tracks.size(); i++ ) {
                if ( tracks[i].name == name ) {
                    track = tracks[i];
                    return true;
                }
            }
            return false;
        }
    };

    class Parser{
    protected:
        ofFbo fbo;
    public:
        vector<Composition> compositions;
    public:
        
        vector<Composition> & getCompositions(){ return compositions; }
        Composition & getComposition(int index) { return compositions[index]; }
        
        void debugDraw(){
            if ( fbo.isAllocated() ) {
                ofEnableAlphaBlending();
                ofSetColor(255);
                fbo.draw(0,0);
            }
        }
        
        ofFbo redraw(Composition comp){
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
        
        vector<Composition> load(string path){
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
                                    track.address = xml.getValue("address", "/exapmle");
                                    xml.popTag();
                                }
                                //scout << track.address << endl;
                                if ( xml.pushTag("process") ) {
                                    for ( int k=0; k<=comp.length; k++ ) {
                                        string n = "f" + ofToString(k);
                                        if ( xml.tagExists(n) ) {
                                            if ( track.type == OSC_VALUE_INT ) {
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
                    //mergeGroupTracks(comp.tracks);
                    result.push_back(comp);
                }
            }
            compositions = result;
            return result;
        }
        
    protected:
        void mergeGroupTracks(vector<Track> & tracks){//todo
            map<string, Track> temp_tracks;
            vector<Track> use_tracks;
            for ( int i=0; i<tracks.size(); i++ ) {
                Track track = tracks[i];
                vector<string> addr_vals = ofSplitString(track.address, "#");
                if ( addr_vals.size() > 1 ) {
                    int index = ofFromString<int>(addr_vals[1]);
                    cout << index << endl;
                    track.isGroup = true;
                    track.group_index =  index;
                    string addr = addr_vals[0];
                    
                    
                    if ( temp_tracks.find(addr) != temp_tracks.end() ) {
                        Track & find_track = temp_tracks[addr];
                        
                    } else {
                        Track new_track;
                        new_track.address = addr;
                        new_track.process = track.process;
                        new_track.name = track.name;
                        temp_tracks[addr] = new_track;
                        use_tracks.push_back(new_track);
                    }
                    //use_tracks.push_back(track);
                } else {
                    use_tracks.push_back(track);
                }
            }
            tracks = use_tracks;
        }
    };

        
}}

namespace ofxVezer = ofx::vezer;
