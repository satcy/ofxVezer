#pragma once
#include <string.h>
#include "ofxVezer.h"
#include "ofxOsc.h"

namespace ofx { namespace vezer{
    using namespace std;
    class Provider{
    private:
        int pre_frame;
    protected:
        deque< ofxOscMessage* > messages;
    public:
        Provider() : pre_frame(-1) {}
        bool hasWaitingMessages();
        bool getNextMessage( ofxOscMessage* message );
        void addMessage(ofxOscMessage * m);
        void setCurrentTracks(Composition & comp, int frame, bool catch_behind = false);
        void sendOscCurrentTracks(Composition & comp, int frame, ofxOscSender & send, bool catch_behind = false);
    };
        
}}

