#pragma once
#include <string>
#include <vector>
#include "nlohmann/json.hpp"
#include "helper/WaveHelper.hpp"
//#include <rubberband/RubberBandStretcher.h>

namespace mck
{
    struct AudioSample
    {
        bool update;
        char curSample;
        WaveInfo info[2];
        std::vector<std::vector<float>> buffer[2];
        /*
        float **pitchBuffer;
        float **outBuffer;
        RubberBand::RubberBandStretcher *pitcher;
        */
        AudioSample() : update(false), curSample(0) {}
    };
    struct AudioVoice
    {
        bool playSample;
        unsigned sampleIdx;
        unsigned startIdx;
        unsigned bufferIdx;
        float gain;
        float pitch;
        AudioVoice() : playSample(false), sampleIdx(0), startIdx(0), bufferIdx(0), gain(0.0), pitch(1.0) {}
    };
    struct Connection
    {
        std::string name;
        Connection() : name("") {}
    };
    void to_json(nlohmann::json &j, const Connection &c);
    void from_json(const nlohmann::json &j, Connection &c);

    struct Port
    {
        bool isInput;
        std::string name;
        std::string fullName;
        std::vector<Connection> connections;
        Port() : isInput(false), name(""), fullName(""), connections() {}
    };
    void to_json(nlohmann::json &j, const Port &p);
    void from_json(const nlohmann::json &j, Port &p);

    struct Message
    {
        std::string section;
        std::string msgType;
        std::string data;
        Message() : section(""), msgType(""), data("") {}
    };
    void to_json(nlohmann::json &j, const Message &m);
    void from_json(const nlohmann::json &j, Message &m);

    struct TriggerData
    {
        int index;
        double strength;
        TriggerData() : index(-1), strength(0.0) {}
    };
    void to_json(nlohmann::json &j, const TriggerData &t);
    void from_json(const nlohmann::json &j, TriggerData &t);

    struct PadData
    {
        std::string type;
        int index;
        double value;
        PadData() : type(""), index(-1), value(0.0) {}
    };
    void to_json(nlohmann::json &j, const PadData &p);
    void from_json(const nlohmann::json &j, PadData &p);

    struct SamplePackSample
    {
        std::string path;
        std::string name;
        unsigned type;
        unsigned index;
        SamplePackSample()
            : path(""),
              name(""),
              type(0),
              index(0) {}
    };
    void to_json(nlohmann::json &j, const SamplePackSample &s);
    void from_json(const nlohmann::json &j, SamplePackSample &s);

    struct SamplePack
    {
        std::string name;
        std::string picture;
        unsigned sampleRate;
        unsigned numBits;
        unsigned numChannels;
        std::vector<std::string> categories;
        std::vector<SamplePackSample> samples;
        SamplePack()
            : name(""),
              picture(""),
              sampleRate(0),
              numBits(0),
              numChannels(0),
              categories(),
              samples() {}
    };
    void to_json(nlohmann::json &j, const SamplePack &s);
    void from_json(const nlohmann::json &j, SamplePack &s);

    struct SampleCommand
    {
        std::string type;
        unsigned packIdx;
        unsigned sampleIdx;
        SampleCommand()
            : type("show"),
              packIdx(0),
              sampleIdx(0) {}
    };
    void to_json(nlohmann::json &j, const SampleCommand &s);
    void from_json(const nlohmann::json &j, SampleCommand &s);

    struct SampleInfo
    {
        bool valid;
        unsigned packIdx;
        unsigned sampleIdx;
        unsigned sampleRate;
        unsigned numChans;
        unsigned lengthMs;
        unsigned lengthSamps;
        std::string path;
        std::vector<std::vector<double>> waveForm;
        SampleInfo()
            : valid(false),
              packIdx(0),
              sampleIdx(0),
              sampleRate(0),
              numChans(1),
              lengthMs(0),
              lengthSamps(0),
              path(""),
              waveForm() {}
    };
    void to_json(nlohmann::json &j, const SampleInfo &s);
    void from_json(const nlohmann::json &j, SampleInfo &s);
}
