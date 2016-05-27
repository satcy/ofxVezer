
#define USE_VEZER_XML

#include "ofMain.h"
#ifdef USE_VEZER_XML
#define VEZER_USE_CEREAL
    #include "ofxVezer.h"
    #include "ParserCereal.h"
#else
    #include "ofxOsc.h"
#endif


class ofApp : public ofBaseApp{
#ifdef USE_VEZER_XML
    ofxVezer::Parser parser;
    ofxVezer::Composition comp;
    ofxVezer::Provider provider;
#else
    ofxOscReceiver recv;
#endif
    float radius;
    vector<ofFloatColor> colors;
    vector<float> rotations;
    ofVec3f rot;
public:
    
    void setup(){
        ofBackground(0);
        
        radius = 300;
        
#ifdef USE_VEZER_XML
        //        vector<ofxVezer::Composition> comps = parser.load("test2.xml");
        vector<ofxVezer::Composition> comps = ofxVezer::ParserCereal::load("test2.xml", "test2.bin");
        comp = comps[0];
        parser.redraw(comp);
#else
        recv.setup(8000);
#endif

    }
    
    void update(){
        
        
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
        
    void draw(){
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

    void windowResized(int w, int h){
#ifdef USE_VEZER_XML
        parser.redraw(comp);
#endif
    }

};

//========================================================================
int main( ){
	ofSetupOpenGL(1024,768,OF_WINDOW);			// <-------- setup the GL context

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());

}
