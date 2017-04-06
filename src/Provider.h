#pragma once

#include "ofxVezer.h"
#include "ofxOsc.h"

namespace ofx { namespace vezer{
    class Provider{
    private:
        int pre_frame;
    protected:
        std::deque< ofxOscMessage* > messages;
    public:
        Provider() : pre_frame(-1) {}
        bool hasWaitingMessages();
        bool getNextMessage( ofxOscMessage* message );
        void addMessage(ofxOscMessage * m);
        void setCurrentTracks(Composition & comp, int frame);
        void sendOscCurrentTracks(Composition & comp, int frame, ofxOscSender & send);
    };
        
}}

