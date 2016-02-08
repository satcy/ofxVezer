
#include "Provider.h"

namespace ofx { namespace vezer{
    
    bool Provider::hasWaitingMessages(){
        int queue_length = (int)messages.size();
        return queue_length > 0;
    }
    
    bool Provider::getNextMessage( ofxOscMessage* message ) {

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
    
    void Provider::addMessage(ofxOscMessage * m){
        messages.push_back( m );
    }
    
    void Provider::setCurrentTracks(Composition & comp, int frame){
        for ( int i=0; i<comp.tracks.size(); i++ ) {
            string address = comp.tracks[i].address;
            ofxOscMessage * m;
            ofxVezer::Proc proc;
            if ( comp.tracks[i].getProcess(frame, &proc) ) {
                m = new ofxOscMessage();
                m->copy(proc);
                addMessage(m);
            }
        }
    }
    
    void Provider::sendOscCurrentTracks(Composition & comp, int frame, ofxOscSender & send){
        for ( int i=0; i<comp.tracks.size(); i++ ) {
            string address = comp.tracks[i].address;
            ofxOscMessage m;
            ofxVezer::Proc proc;
            if ( comp.tracks[i].getProcess(frame, &proc) ) {
                send.sendMessage(proc);
            }
        }
    }
    
        
}}

