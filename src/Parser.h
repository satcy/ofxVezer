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
    const string MIDI_NOTES = "MidiNotes";
    
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
        
        Proc();
        
        Proc(string addr, int f, int32_t val);
        
        Proc(string addr, int f, double val);
        
        Proc(string addr, int f, float r, float g, float b);
        
        Proc(string addr, int f, string s);
        
        bool isNumberString(const string & s);
        
        void copy( const Proc& other );
    };
    
    class Track{
    public:
        int group_index;
        bool state;
        string address;
        string name;
        string type;
        float min;
        float max;
        
        int current;
        vector<Proc> process;
        
        Track();
        
        Proc& getCurrentProc();
        
        void sortProcess();
        
        static inline bool compareByFrame(const Proc & a, const Proc & b){
            return a.frame < b.frame;
        }
        
        bool getProcess(int frame, Proc * proc);
        
        int getProcessIndex(int frame);
        
        bool getFirstProcess(Proc * proc);
        
        bool getLastProcess(Proc * proc);
        
        bool getLazyFrameProcess(int frame, Proc * proc);
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
        
        vector<Track>& getCurrentTracks(int frame);
        
        bool getTrackByName(string name, Track & track);
        
        bool combine(const Composition & other, bool priority = false);
    };
    
    class Parser{
    protected:
        ofFbo fbo;
    public:
        vector<Composition> compositions;
    public:
        Parser ();
        
        vector<Composition> & getCompositions();
        Composition & getComposition(int index);
        
        bool midi_as_osc;
        
        void debugDraw();
        
        ofFbo redraw(Composition comp);
        
        vector<Composition> load(string path);
    protected:
        static bool compareGroupIndex(Track a, Track b){ return a.group_index < b.group_index; }
        
        void mergeGroupTracks(vector<Track> & tracks);
    };
    
    
}}

namespace ofxVezer = ofx::vezer;
