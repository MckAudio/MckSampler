#include "Config.hpp"
#include "helper/DspHelper.hpp"

void SP::to_json(nlohmann::json &j, const SP::Sample &s)
{
    j["available"] = s.available;
    j["name"] = s.name;
    j["relativePath"] = s.relativePath;
    j["fullPath"] = s.fullPath;
    j["numChannels"] = s.numChannels;
    j["numFrames"] = s.numFrames;
    j["sampleRate"] = s.sampleRate;
}

void SP::from_json(const nlohmann::json &j, SP::Sample &s)
{
    s.available = j.at("available").get<bool>();
    s.name = j.at("name").get<std::string>();
    s.relativePath = j.at("relativePath").get<std::string>();
    s.fullPath = j.at("fullPath").get<std::string>();
    s.numChannels = j.at("numChannels").get<unsigned>();
    s.numFrames = j.at("numFrames").get<unsigned>();
    s.sampleRate = j.at("sampleRate").get<unsigned>();
}

void SP::to_json(nlohmann::json &j, const SP::Pad &p)
{
    j["available"] = p.available;
    j["tone"] = p.tone;
    j["ctrl"] = p.ctrl;
    j["sample"] = p.sample;
    j["sampleIdx"] = p.sampleIdx;
    j["gain"] = p.gain;
    j["pitch"] = p.pitch;
}

void SP::from_json(const nlohmann::json &j, SP::Pad &p)
{
    p.available = j.at("available").get<bool>();
    p.tone = j.at("tone").get<unsigned>();
    p.ctrl = j.at("ctrl").get<unsigned>();
    p.sample = j.at("sample").get<std::string>();
    p.sampleIdx = j.at("sampleIdx").get<unsigned>();
    p.gain = j.at("gain").get<double>();
    p.pitch = j.at("pitch").get<double>();
}

void SP::to_json(nlohmann::json &j, const SP::Config &c)
{
    j["tempo"] = c.tempo;
    j["numPads"] = c.numPads;
    j["numSamples"] = c.numSamples;
    j["pads"] = c.pads;
    j["samples"] = c.samples;
    j["midiChan"] = c.midiChan;
    j["reconnect"] = c.reconnect;
    j["midiInConnections"] = c.midiInConnections;
    j["midiOutConnections"] = c.midiOutConnections;
    j["audioLeftConnections"] = c.audioLeftConnections;
    j["audioRightConnections"] = c.audioRightConnections;
}

void SP::from_json(const nlohmann::json &j, SP::Config &c)
{
    c.tempo = j.at("tempo").get<double>();
    c.numPads = j.at("numPads").get<unsigned>();
    c.numSamples = j.at("numSamples").get<unsigned>();
    c.pads = j.at("pads").get<std::vector<SP::Pad>>();
    c.samples = j.at("samples").get<std::vector<SP::Sample>>();
    c.midiChan = j.at("midiChan").get<unsigned>();
    c.reconnect = j.at("reconnect").get<bool>();
    c.midiInConnections = j.at("midiInConnections").get<std::vector<std::string>>();
    c.midiOutConnections = j.at("midiOutConnections").get<std::vector<std::string>>();
    c.audioLeftConnections = j.at("audioLeftConnections").get<std::vector<std::string>>();
    c.audioRightConnections = j.at("audioRightConnections").get<std::vector<std::string>>();
}

bool SP::ScanSampleFolder(std::string path, std::vector<Sample> &sampleList)
{
    sampleList.clear();
    Sample tmp;
    fs::path tmpPath;
    SNDFILE *tmpSnd;
    SF_INFO tmpInfo;

    for (auto &p : std::filesystem::recursive_directory_iterator(path))
    {
        if (p.path().extension() == ".wav")
        {
            tmpPath = fs::relative(p.path(), fs::path(path));
            tmp.relativePath = tmpPath.string();
            tmp.name = tmpPath.stem().string();
            tmp.fullPath = p.path().string();

            tmpSnd = sf_open(tmp.fullPath.c_str(), SFM_READ, &tmpInfo);

            tmp.numChannels = tmpInfo.channels;
            tmp.numFrames = tmpInfo.frames;
            tmp.sampleRate = tmpInfo.samplerate;

            sf_close(tmpSnd);

            sampleList.push_back(tmp);
        }
    }

    // Sort List
    std::sort(sampleList.begin(), sampleList.end(), [](Sample &a, Sample &b) {
        return a.relativePath < b.relativePath;
    });
    return true;
}

bool SP::VerifyConfiguration(Config &config)
{
    config.numPads = config.pads.size();
    config.numSamples = config.samples.size();
    for (unsigned i = 0; i < config.numPads; i++)
    {
        config.pads[i].available = false;

        for (unsigned j = 0; j < config.numSamples; j++)
        {
            if (config.samples[j].relativePath == config.pads[i].sample)
            {
                config.pads[i].sampleIdx = j;
                config.pads[i].available = true;
                config.pads[i].gain = std::min(6.0, std::max(-200.0, config.pads[i].gain));
                config.pads[i].gainLin = mck::DbToLin(config.pads[i].gain);
                break;
            }
        }
    }
    return true;
}