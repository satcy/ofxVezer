#include "ofApp.h"
#define USE_VEZER_XML

//--------------------------------------------------------------
void ofApp::setup(){
    ofBackground(0);

    radius = 300;

#ifdef USE_VEZER_XML
    vector<ofxVezer::Composition> comps = ofxVezer::ParserCereal::load("test.xml", "test.bin");

    comp = comps[0];
    comp.combine(comps[1]);
    parser.redraw(comp);
#else
    recv.setup(8000);
#endif

}

//--------------------------------------------------------------
void ofApp::update(){

#ifdef USE_VEZER_XML
    int frame = int(ofGetElapsedTimef() * comp.fps)  % comp.length;
    provider.setCurrentTracks(comp, frame);
    while ( provider.hasWaitingMessages() ) {
        ofxOscMessage m;
        provider.getNextMessage(&m);
        string addr = m.getAddress();
#else
        while ( recv.hasWaitingMessages() ) {
            ofxOscMessage m;
            recv.getNextMessage(&m);
            string addr = m.getAddress();
#endif

            if ( addr == "/color" ) {
                ofFloatColor color;
                color.set(m.getArgAsFloat(0), m.getArgAsFloat(1), m.getArgAsFloat(2));
                colors.push_back(color);
                if ( colors.size() > 100 ) colors.erase(colors.begin(), colors.begin()+1);
            } else if ( addr == "/rotation" ) {
                float rotation = m.getArgAsFloat(0)*180;
                rotations.push_back(rotation);
                if ( rotations.size() > 100 ) rotations.erase(rotations.begin(), rotations.begin()+1);
            } else if ( addr == "/example" ) {
                ofSetCircleResolution(ofRandom(3,10));
                radius = ofRandom(300);
            } else if ( addr == "/group" ) {
                rot.x = m.getArgAsFloat(0);
                rot.y = m.getArgAsFloat(1);
                rot.z = m.getArgAsFloat(2);
            }
        }

        radius += ( 100 - radius )*0.2321;
    }

//--------------------------------------------------------------
void ofApp::draw(){
    ofPushStyle();
    ofPushMatrix();
    ofTranslate(ofGetWidth()/2, ofGetHeight()/2);
    ofFill();
    for ( int i=0; i<rotations.size(); i++ ) {
        if ( i < colors.size() ) ofSetColor(colors[i]);
        ofRotate(rotations[i]);
        if ( i == 0 ) ofFill();
        else ofNoFill();
        ofCircle(0, 0, radius+i*6);
    }
    ofPopMatrix();

    ofPushMatrix();
    ofNoFill();
    ofSetColor(255);
    ofTranslate(ofGetWidth()/2, ofGetHeight()/2);
    ofRotateX(rot.x);
    ofRotateY(rot.y);
    ofRotateZ(rot.z);
    ofDrawBox(0, 0, 0, 200);
    ofPopMatrix();

    ofPopStyle();

#ifdef USE_VEZER_XML
    ofSetColor(255);
    parser.debugDraw();
    int frame = int(ofGetElapsedTimef() * comp.fps)  % comp.length;
    float x = ( frame/float(comp.length) ) * ofGetWidth();
    ofRect(x, 0, 1, ofGetHeight());
#endif

}


//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
#ifdef USE_VEZER_XML
    parser.redraw(comp);
#endif
}


