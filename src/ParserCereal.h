#pragma once
#include <string.h>
//#define NOT_USE_CEREAL
#ifndef NOT_USE_CEREAL
#include "ofMain.h"

#include "Parser.h"
#include "ofxCereal.h"

namespace cereal
{
    template<class Archive>
    void save(Archive & archive,
              ofxVezer::Proc const & m)
    {
        using namespace ofxVezer;
        archive << m.getAddress();
        archive << m.frame;
        archive << m.type;
        archive << m.getNumArgs();
        for (int i=0; i<m.getNumArgs(); ++i) {
            unsigned char type = m.getArgType(i);
            archive << type;
            if (type == OFXOSC_TYPE_FLOAT) {
                archive << m.getArgAsFloat(i);
            } else if (type == OFXOSC_TYPE_STRING) {
                archive << m.getArgAsString(i);
            } else {
                archive << m.getArgAsInt(i);
            }
        }
    }
    
    template<class Archive>
    void load(Archive & archive,
              ofxVezer::Proc & m)
    {
        using namespace ofxVezer;
        Proc in;
        std::string address;
        archive >> address;
        in.setAddress(address);
        archive >> in.frame;
        archive >> in.type;
        int numargs;
        archive >> numargs;
        for (int i=0; i<numargs; ++i) {
            unsigned char type = 0;
            archive >> type;
            if (type == OFXOSC_TYPE_FLOAT) {
                float f;
                archive >> f;
                in.addFloatArg(f);
            } else if (type == OFXOSC_TYPE_STRING) {
                std::string s;
                archive >> s;
                in.addStringArg(s);
            } else {
                int i32;
                archive >> i32;
                in.addIntArg(i32);
            }
        }
        m = in;
    }
    
    template<class Archive>
    void serialize(Archive & archive,
                   ofxVezer::Track & m)
    {
        archive(m.group_index,
                m.state,
                m.address,
                m.name,
                m.type,
                m.min,
                m.max,
                m.current,
                m.process);
    }
    
    template<class Archive>
    void serialize(Archive & archive, ofxVezer::Composition & m)
    {
        archive(m.state,
                m.name,
                m.fps,
                m.length,
                m.bpm,
                m.loop,
                m.start,
                m.end,
                m.tracks,
                m.tmp_tracks);
    }
}


namespace ofx { namespace vezer{
    class ParserCereal{
    public:
        static std::vector<Composition> load(std::string xml_path, std::string bin_path){
            ofFile xml(xml_path);
            ofFile bin(bin_path);
            
            std::vector<Composition> comps;
            if (!bin.exists() || filesystem::last_write_time(xml) > filesystem::last_write_time(bin)) {
                Parser parser;
                comps = parser.load(xml.getAbsolutePath());
                ofFile file(bin_path, ofFile::WriteOnly);
                ofxCereal::binout(file) << comps;
            } else {
                ofxCereal::binin(bin) >> comps;
            }
            return comps;
            
        }
    };
}}
#endif
