#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <sndfile.h>
#include <filesystem>
#include <iostream>

#include "Types.hpp"

namespace mck
{
    namespace sampler
    {
        namespace fs = std::filesystem;
        struct Sample
        {
            bool available;
            std::string name;
            std::string relativePath;
            std::string fullPath;
            unsigned numChannels;
            unsigned numFrames;
            unsigned sampleRate;
            Sample() : available(false), name(""), relativePath(""), fullPath(""), numChannels(0), numFrames(0), sampleRate(0) {}
        };
        void to_json(nlohmann::json &j, const Sample &s);
        void from_json(const nlohmann::json &j, Sample &s);

        struct Step
        {
            bool active;
            unsigned velocity;
            Step() : active(false), velocity(100) {}
        };
        void to_json(nlohmann::json &j, const Step &s);
        void from_json(const nlohmann::json &j, Step &s);

        struct Pattern
        {
            unsigned nSteps;
            std::vector<Step> steps;
            Pattern() : nSteps(16)
            {
                steps.resize(nSteps);
            }
            Pattern(unsigned stepCount)
                : nSteps(stepCount)
            {
                steps.resize(nSteps);
            }
        };
        void to_json(nlohmann::json &j, const Pattern &p);
        void from_json(const nlohmann::json &j, Pattern &p);

        struct Pad
        {
            bool available;
            bool reverse;
            unsigned tone;
            unsigned ctrl;
            std::string samplePath;
            std::string sampleName;
            double gain;
            double pan; // -100 (L) - 0 (C) - 100 (R)
            double gainLeftLin;
            double gainRightLin;
            double pitch;
            unsigned nPatterns;
            std::vector<Pattern> patterns;
            Pad()
                : available(false),
                  reverse(false),
                  tone(255),
                  ctrl(255),
                  samplePath(""),
                  sampleName(""),
                  gain(0.0),
                  pan(0.0),
                  pitch(1.0),
                  nPatterns(1),
                  patterns()
            {
                patterns.resize(nPatterns);
            };
        };
        void to_json(nlohmann::json &j, const Pad &p);
        void from_json(const nlohmann::json &j, Pad &p);

        struct Config
        {
            double tempo;
            unsigned numPads;
            unsigned numSamples;
            std::vector<Pad> pads;
            unsigned midiChan;
            bool reconnect;
            std::vector<std::string> midiInConnections;
            std::vector<std::string> midiOutConnections;
            std::vector<std::string> audioLeftConnections;
            std::vector<std::string> audioRightConnections;
            Config() : tempo(110.0), numPads(0), midiChan(0), numSamples(0), reconnect(true)
            {
                pads.resize(numPads);
            };
        };
        void to_json(nlohmann::json &j, const Config &c);
        void from_json(const nlohmann::json &j, Config &c);

        bool ScanSampleFolder(std::string path, std::vector<Sample> &sampleList);
        bool VerifyConfiguration(Config &config, std::string samplePackPath);
    } // namespace sampler
} // namespace mck