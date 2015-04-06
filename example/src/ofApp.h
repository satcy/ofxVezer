#pragma once

#define USE_VEZER_XML

#include "ofMain.h"
#ifdef USE_VEZER_XML
    #include "ofxVezer.h"
#else
    #include "ofxOsc.h"
#endif

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
#ifdef USE_VEZER_XML    
    ofxVezer::Parser parser;
    ofxVezer::Composition comp;
    ofxVezer::Provider provider;
#else
    ofxOscReceiver recv;
#endif
    ofFloatColor color;
    float rotation;
    float radius;
    
};
