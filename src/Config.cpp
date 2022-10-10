#include "Config.hpp"
#include <MckHelper/DspHelper.hpp>

void mck::sampler::to_json(nlohmann::json &j, const mck::sampler::Sample &s)
{
    j["available"] = s.available;
    j["name"] = s.name;
    j["relativePath"] = s.relativePath;
    j["fullPath"] = s.fullPath;
    j["numChannels"] = s.numChannels;
    j["numFrames"] = s.numFrames;
    j["sampleRate"] = s.sampleRate;
}

void mck::sampler::from_json(const nlohmann::json &j, mck::sampler::Sample &s)
{
    s.available = j.at("available").get<bool>();
    s.name = j.at("name").get<std::string>();
    s.relativePath = j.at("relativePath").get<std::string>();
    s.fullPath = j.at("fullPath").get<std::string>();
    s.numChannels = j.at("numChannels").get<unsigned>();
    s.numFrames = j.at("numFrames").get<unsigned>();
    s.sampleRate = j.at("sampleRate").get<unsigned>();
}

void mck::sampler::to_json(nlohmann::json &j, const Step &s)
{
    j["active"] = s.active;
    j["velocity"] = s.velocity;
}
void mck::sampler::from_json(const nlohmann::json &j, Step &s)
{
    s.active = j.at("active").get<bool>();
    s.velocity = std::min((unsigned)127, j.at("velocity").get<unsigned>());
}

void mck::sampler::to_json(nlohmann::json &j, const mck::sampler::Pattern &p)
{
    j["nSteps"] = p.nSteps;
    j["steps"] = p.steps;
}
void mck::sampler::from_json(const nlohmann::json &j, mck::sampler::Pattern &p)
{
    p.nSteps = j.at("nSteps").get<unsigned>();
    p.steps = j.at("steps").get<std::vector<Step>>();
}

void mck::sampler::to_json(nlohmann::json &j, const Delay &d)
{
    j["active"] = d.active;
    j["type"] = d.type;
    j["timeMs"] = d.timeMs;
    j["gain"] = d.gain;
    j["feedback"] = d.feedback;
}
void mck::sampler::from_json(const nlohmann::json &j, Delay &d)
{
    d.active = j.at("active").get<bool>();
    d.type = std::min((char)DLY_ANALOG, std::max((char)DLY_DIGITAL, j.at("type").get<char>()));
    d.timeMs = std::max((unsigned)10, std::min((unsigned)1000, j.at("timeMs").get<unsigned>()));
    d.gain = j.at("gain").get<double>();
    d.feedback = std::min(1.0, std::max(0.0, j.at("feedback").get<double>()));
}


void mck::sampler::to_json(nlohmann::json &j, const Compressor &c)
{
    j["active"] = c.active;
    j["attackMs"] = c.attackMs;
    j["releaseMs"] = c.releaseMs;
    j["threshold"] = c.threshold;
    j["ratio"] = c.ratio;
    j["makeup"] = c.makeup;
}
void mck::sampler::from_json(const nlohmann::json &j, Compressor &c)
{
    
    c.active = j.at("active").get<bool>();
    c.attackMs = std::max((unsigned)1, std::min((unsigned)500, j.at("attackMs").get<unsigned>()));
    c.releaseMs = std::max((unsigned)1, std::min((unsigned)1000, j.at("releaseMs").get<unsigned>()));
    c.threshold = std::max(-60.0, std::min(0.0, j.at("threshold").get<double>()));
    c.ratio = std::max(1.0, std::min(10.0, j.at("ratio").get<double>()));
    c.makeup = std::max(0.0, std::min(20.0, j.at("makeup").get<double>()));
}

void mck::sampler::to_json(nlohmann::json &j, const mck::sampler::Pad &p)
{
    j["available"] = p.available;
    j["reverse"] = p.reverse;
    j["lengthMs"] = p.lengthMs;
    j["maxLengthMs"] = p.maxLengthMs;
    j["tone"] = p.tone;
    j["ctrl"] = p.ctrl;
    j["samplePath"] = p.samplePath;
    j["sampleName"] = p.sampleName;
    j["gain"] = p.gain;
    j["pan"] = p.pan;
    j["pitch"] = p.pitch;
    j["delay"] = p.delay;
    j["comp"] = p.comp;
    j["nPatterns"] = p.nPatterns;
    j["patterns"] = p.patterns;
}

void mck::sampler::from_json(const nlohmann::json &j, mck::sampler::Pad &p)
{
    p.available = j.at("available").get<bool>();
    p.reverse = j.at("reverse").get<bool>();
    p.lengthMs = j.at("lengthMs").get<unsigned>();
    p.tone = j.at("tone").get<unsigned>();
    p.ctrl = j.at("ctrl").get<unsigned>();
    p.samplePath = j.at("samplePath").get<std::string>();
    p.sampleName = j.at("sampleName").get<std::string>();
    p.gain = j.at("gain").get<double>();
    p.pan = j.at("pan").get<double>();
    p.pitch = j.at("pitch").get<double>();
    try
    {
        p.delay = j.at("delay").get<Delay>();
    }
    catch (std::exception &e)
    {
        p.delay = Delay();
    }
    try
    {
        p.comp = j.at("comp").get<Compressor>();
    }
    catch (std::exception &e)
    {
        p.comp = Compressor();
    }
    try
    {
        p.nPatterns = j.at("nPatterns").get<unsigned>();
        p.patterns = j.at("patterns").get<std::vector<Pattern>>();
    }
    catch (std::exception &e)
    {
        p.nPatterns = 1;
        p.patterns.resize(p.nPatterns);
    }
}

void mck::sampler::to_json(nlohmann::json &j, const mck::sampler::Config &c)
{
    j["tempo"] = c.tempo;
    j["activePad"] = c.activePad;
    j["numPads"] = c.numPads;
    j["numSamples"] = c.numSamples;
    j["pads"] = c.pads;
    j["midiChan"] = c.midiChan;
    j["reconnect"] = c.reconnect;
    j["midiInConnections"] = c.midiInConnections;
    j["midiOutConnections"] = c.midiOutConnections;
    j["audioLeftConnections"] = c.audioLeftConnections;
    j["audioRightConnections"] = c.audioRightConnections;
}

void mck::sampler::from_json(const nlohmann::json &j, mck::sampler::Config &c)
{
    c.tempo = j.at("tempo").get<double>();
    try {
        c.activePad = j.at("activePad").get<unsigned>();
    } catch (std::exception &e) {
        c.activePad = 0;
    }
    c.numPads = j.at("numPads").get<unsigned>();
    c.numSamples = j.at("numSamples").get<unsigned>();
    c.pads = j.at("pads").get<std::vector<mck::sampler::Pad>>();
    c.midiChan = j.at("midiChan").get<unsigned>();
    c.reconnect = j.at("reconnect").get<bool>();
    c.midiInConnections = j.at("midiInConnections").get<std::vector<std::string>>();
    c.midiOutConnections = j.at("midiOutConnections").get<std::vector<std::string>>();
    c.audioLeftConnections = j.at("audioLeftConnections").get<std::vector<std::string>>();
    c.audioRightConnections = j.at("audioRightConnections").get<std::vector<std::string>>();
}

bool mck::sampler::ScanSampleFolder(std::string path, std::vector<Sample> &sampleList)
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

bool mck::sampler::VerifyConfiguration(Config &config, std::string samplePackPath, unsigned sampleRate)
{
    config.numPads = config.pads.size();
    for (unsigned i = 0; i < config.numPads; i++)
    {
        config.pads[i].available = false;
        config.pads[i].gain = std::min(6.0, std::max(-200.0, config.pads[i].gain));
        config.pads[i].pan = std::min(100.0, std::max(-100.0, config.pads[i].pan));
        double gainLin = mck::DbToLin(config.pads[i].gain);
        config.pads[i].gainLeftLin = gainLin * std::sqrt((double)(100 - config.pads[i].pan) / 200.0);
        config.pads[i].gainRightLin = gainLin * std::sqrt((double)(100 + config.pads[i].pan) / 200.0);
        config.pads[i].lengthSamps = (unsigned)std::floor((double)config.pads[i].lengthMs * (double)sampleRate / 1000.0);

        fs::path samplePath(samplePackPath);
        samplePath.append(config.pads[i].samplePath);
        if (fs::exists(samplePath))
        {
            config.pads[i].available = true;
        }
    }
    return true;
}