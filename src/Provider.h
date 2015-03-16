#pragma once

#include "ofxVezer.h"
#include "ofxOsc.h"

namespace ofx { namespace vezer{
    
    class Provider{
    protected:
        std::deque< ofxOscMessage* > messages;
    public:
        bool hasWaitingMessages(){
            int queue_length = (int)messages.size();
            return queue_length > 0;
        }
        
        bool getNextMessage( ofxOscMessage* message ) {
            if ( messages.size() == 0 )
            {
                return false;
            }
            
            ofxOscMessage* src_message = messages.front();
            message->copy( *src_message );
            
            delete src_message;
            messages.pop_front();
            
            return true;
        }
        
        void addMessage(ofxOscMessage * m){
            messages.push_back( m );
        }
        
        void setCurrentTracks(Composition & comp, int frame){
            for ( int i=0; i<comp.tracks.size(); i++ ) {
                string address = comp.tracks[i].address;
                ofxOscMessage * m;
                ofxVezer::Proc proc;
                if ( comp.tracks[i].getProcess(frame, &proc) ) {
                    if ( proc.type == ofxVezer::Proc::TYPE_INT ) {
                        m = new ofxOscMessage();
                        m->setAddress(address);
                        m->addIntArg(proc.int_value);
                        addMessage(m);
                    } else if ( proc.type == ofxVezer::Proc::TYPE_FLOAT ) {
                        m = new ofxOscMessage();
                        m->setAddress(address);
                        m->addFloatArg(proc.float_value);
                        addMessage(m);
                    } else if ( proc.type == ofxVezer::Proc::TYPE_FLOAT_COLOR ) {
                        m = new ofxOscMessage();
                        m->setAddress(address);
                        m->addFloatArg(proc.color.r);
                        m->addFloatArg(proc.color.g);
                        m->addFloatArg(proc.color.b);
                        addMessage(m);
                    } else if ( proc.type == ofxVezer::Proc::TYPE_FLAG ) {
                        m = new ofxOscMessage();
                        m->setAddress(proc.flag_value);
                        addMessage(m);
                    }
                }
            }
        }
        
        void sendOscCurrentTracks(Composition & comp, int frame, ofxOscSender & send){
            for ( int i=0; i<comp.tracks.size(); i++ ) {
                string address = comp.tracks[i].address;
                ofxOscMessage m;
                ofxVezer::Proc proc;
                if ( comp.tracks[i].getProcess(frame, &proc) ) {
                    if ( proc.type == ofxVezer::Proc::TYPE_INT ) {
                        m.setAddress(address);
                        m.addIntArg(proc.int_value);
                        send.sendMessage(m);
                    } else if ( proc.type == ofxVezer::Proc::TYPE_FLOAT ) {
                        m.setAddress(address);
                        m.addFloatArg(proc.float_value);
                        send.sendMessage(m);
                    } else if ( proc.type == ofxVezer::Proc::TYPE_FLOAT_COLOR ) {
                        m.setAddress(address);
                        m.addFloatArg(proc.color.r);
                        m.addFloatArg(proc.color.g);
                        m.addFloatArg(proc.color.b);
                        send.sendMessage(m);
                    } else if ( proc.type == ofxVezer::Proc::TYPE_FLAG ) {
                        m.setAddress(proc.flag_value);
                        send.sendMessage(m);
                    }
                }
            }
        }
        
    };
        
}}

