#pragma once
#include <string>
#include <vector>
#include "nlohmann/json.hpp"
//#include <rubberband/RubberBandStretcher.h>

namespace MCK {
    struct AudioSample {
        unsigned numChans;
        unsigned numFrames;
        float *buffer;
        /*
        float **pitchBuffer;
        float **outBuffer;
        RubberBand::RubberBandStretcher *pitcher;
        */
        AudioSample() : numChans(0), numFrames(0), buffer(nullptr)/*, pitchBuffer(nullptr), outBuffer(nullptr), pitcher(nullptr)*/ {}
    };
    struct AudioVoice {
        bool playSample;
        unsigned sampleIdx;
        unsigned startIdx;
        unsigned bufferIdx;
        float gain;
        float pitch;
        AudioVoice() : playSample(false), sampleIdx(0), startIdx(0), bufferIdx(0), gain(1.0f), pitch(1.0) {}
    };
    struct Connection {
        std::string name;
        Connection() : name("") {}
    };
    void to_json(nlohmann::json &j, const Connection &c);
    void from_json(const nlohmann::json &j, Connection &c);

    struct Port {
        bool isInput;
        std::string name;
        std::string fullName;
        std::vector<Connection> connections;
        Port() : isInput(false), name(""), fullName(""), connections() {}
    };
    void to_json(nlohmann::json &j, const Port &p);
    void from_json(const nlohmann::json &j, Port &p);

}
