#pragma once

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

    void setup();

    void update();

    void draw();

    void windowResized(int w, int h);
		
};
