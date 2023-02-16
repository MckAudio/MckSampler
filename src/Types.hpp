#pragma once
#include <string>
#include <vector>
#include <atomic>
#include <nlohmann/json.hpp>
#include <MckHelper/WaveHelper.hpp>
#include <juce_dsp/juce_dsp.h>

#include "dsp/DelayModule.hpp"
#include "dsp/ReverbModule.hpp"
namespace mck
{
    struct AudioSample
    {
        // Sample
        bool update;
        size_t curSample;
        size_t curDelay;
        size_t newDelay;
        WaveInfo info[2];
        std::vector<std::vector<float>> buffer[2];
        mck::dsp::DelayModule delay[2];
        juce::dsp::Compressor<float> compressor;
        //mck::dsp::ReverbModule reverb;
        // Buffer
        double *dsp[2];
        /*
        float **pitchBuffer;
        float **outBuffer;
        RubberBand::RubberBandStretcher *pitcher;
        */
        AudioSample()
            : update(false),
              curSample(0),
              curDelay(0),
              newDelay(1)
        {
        }
        ~AudioSample()
        {
            /*
            if (delay[0] != nullptr)
            {
                delete delay[0];
                delay[0] = nullptr;
            }
            if (delay[1] != nullptr)
            {
                delete delay[1];
                delay[1] = nullptr;
            }*/
            
            if (dsp[0] != nullptr)
            {
                delete dsp[0];
                dsp[0] = nullptr;
            }
            if (dsp[1] != nullptr)
            {
                delete dsp[1];
                dsp[1] = nullptr;
            }
        }
    };
    struct AudioVoice
    {
        bool playSample;
        unsigned padIdx;
        unsigned startIdx;
        unsigned bufferIdx;
        unsigned bufferLen;
        float gainL;
        float gainR;
        float pitch;
        AudioVoice() : playSample(false), padIdx(0), startIdx(0), bufferIdx(0), bufferLen(0), gainL(0.0), gainR(0.0), pitch(1.0) {}
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
        std::string id;
        unsigned type;
        unsigned index;
        SamplePackSample()
            : path(""),
              name(""),
              id(""),
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
        unsigned padIdx;
        SampleCommand()
            : type("show"),
              packIdx(0),
              sampleIdx(0),
              padIdx(0) {}
    };
    void to_json(nlohmann::json &j, const SampleCommand &s);
    void from_json(const nlohmann::json &j, SampleCommand &s);

    enum SampleEditCmd
    {
        SCMD_CREATE = 0,
        SCMD_DELETE,
        SCMD_CHANGE,
        SCMD_IMPORT,
        SCMD_EXPORT,
        SCMD_LENGTH
    };

    enum SampleEditClass
    {
        SEC_PACK = 0,
        SEC_CATEGORY,
        SEC_SAMPLE,
        SEC_LENGTH
    };
    enum SampleEditType
    {
        SET_NAME = 0,
        SET_INDEX,
        SET_CATEGORY,
        SET_LENGTH
    };

    struct SampleEdit
    {
        unsigned cmd;       // create, delete, change
        unsigned classType; // pack, category, sample
        unsigned editType;
        unsigned numberValue;
        std::string stringValue;
        unsigned packIdx;
        unsigned categoryIdx;
        unsigned sampleIdx;
        SampleEdit()
            : cmd(SCMD_CHANGE),
              classType(SEC_SAMPLE),
              editType(SET_NAME),
              numberValue(0),
              stringValue(""),
              packIdx(0),
              categoryIdx(0),
              sampleIdx(0)
        {
        }
    };
    void to_json(nlohmann::json &j, const SampleEdit &s);
    void from_json(const nlohmann::json &j, SampleEdit &s);

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
