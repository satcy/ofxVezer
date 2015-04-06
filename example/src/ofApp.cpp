#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofBackground(0);
    
    radius = 300;
    
#ifdef USE_VEZER_XML
    vector<ofxVezer::Composition> comps = parser.load("test.xml");
    comp = comps[0];
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
            color.set(m.getArgAsFloat(0), m.getArgAsFloat(1), m.getArgAsFloat(2));
        } else if ( addr == "/rotation" ) {
            rotation = m.getArgAsFloat(0)*180;
        } else if ( addr == "/example" ) {
            ofSetCircleResolution(ofRandom(3,10));
            radius = ofRandom(600);
        }
    }
        
    radius += ( 300 - radius )*0.321;
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofPushStyle();
    ofFill();
    ofPushMatrix();
    ofTranslate(ofGetWidth()/2, ofGetHeight()/2);
    ofSetColor(color);
    ofRotate(rotation);
    ofCircle(0, 0, radius);
    
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
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    
#ifdef USE_VEZER_XML
    parser.redraw(comp);
#endif

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
