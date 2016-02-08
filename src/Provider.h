#pragma once

#include "ofxVezer.h"
#include "ofxOsc.h"

namespace ofx { namespace vezer{
    class Provider{
    protected:
        std::deque< ofxOscMessage* > messages;
    public:
        bool hasWaitingMessages();
        bool getNextMessage( ofxOscMessage* message );
        void addMessage(ofxOscMessage * m);
        void setCurrentTracks(Composition & comp, int frame);
        void sendOscCurrentTracks(Composition & comp, int frame, ofxOscSender & send);
    };
        
}}

