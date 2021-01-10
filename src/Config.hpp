#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <sndfile.h>
#include <filesystem>
#include <iostream>

#include "Types.hpp"

namespace SP
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
struct Pad
{
    bool available;
    unsigned tone;
    unsigned ctrl;
    std::string sample;
    unsigned sampleIdx;
    double gain;
    double gainLin;
    double pitch;
    Pad() : available(false), tone(255), ctrl(255), sample(""), sampleIdx(0), gain(1.0), pitch(1.0) {};
};
void to_json(nlohmann::json &j, const Pad &p);
void from_json(const nlohmann::json &j, Pad &p);
struct Config
{
    double tempo;
    unsigned numPads;
    unsigned numSamples;
    std::vector<Pad> pads;
    std::vector<Sample> samples;
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
bool VerifyConfiguration(Config &config);
} // namespace SP