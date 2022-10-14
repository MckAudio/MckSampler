#include "SampleExplorer.hpp"
#include <filesystem>
#include <nlohmann/json.hpp>
#include <cstdio>
#include <fstream>
#include <algorithm>
#include <regex>
#include <sndfile.h>
#include <samplerate.h>

namespace fs = std::filesystem;

mck::SampleExplorer::SampleExplorer()
    : m_isInitialized(false),
      m_bufferSize(0),
      m_sampleRate(0),
      m_samplePath(""),
      m_packs(),
      m_packPaths(),
      m_curWave(0)
{
}

mck::SampleExplorer::~SampleExplorer()
{
}

bool mck::SampleExplorer::Init(unsigned bufferSize, unsigned sampleRate, std::string samplePath)
{
    if (m_isInitialized)
    {
        return false;
    }

    m_bufferSize = bufferSize;
    m_sampleRate = sampleRate;

    fs::path sp(samplePath);
    if (sp.is_absolute() == false)
    {
        sp = fs::absolute(sp);
    }

    if (fs::exists(sp) == false)
    {
        if (fs::create_directories(sp) == false)
        {
            return false;
        }
    }
    m_samplePath = sp.string();

    // Malloc buffers etc...

    m_isInitialized = true;
    return true;
}

void mck::SampleExplorer::RefreshSamples(std::vector<SamplePack> &packs)
{
    if (m_isInitialized == false)
    {
        packs.clear();
        return;
    }

    fs::path sp(m_samplePath);
    std::vector<std::pair<SamplePack, std::string>> tmpPacks;

    for (auto &dp : fs::directory_iterator(sp))
    {
        if (dp.is_directory())
        {
            for (auto &cfp : fs::directory_iterator(dp.path()))
            {
                if (cfp.is_regular_file() && cfp.path().has_extension())
                {
                    if (cfp.path().extension() == ".mcksp")
                    {
                        std::pair<SamplePack, std::string> pack;
                        std::ifstream spFile(cfp.path());
                        nlohmann::json j;
                        spFile >> j;
                        spFile.close();
                        try
                        {
                            pack.first = j.get<SamplePack>();
                        }
                        catch (std::exception &e)
                        {
                            std::printf("File %s is malformed: %s\n", cfp.path().c_str(), e.what());
                            continue;
                        }
                        if (dp.path().is_absolute())
                        {
                            pack.second = dp.path();
                        }
                        else
                        {
                            pack.second = fs::absolute(dp.path());
                        }

                        tmpPacks.push_back(pack);
                    }
                }
            }
        }
    }
    // Sort by name
    std::sort(tmpPacks.begin(), tmpPacks.end(),
              [](const std::pair<SamplePack, std::string> &a, const std::pair<SamplePack, std::string> &b) -> bool
              {
                  return a.first.name < b.first.name;
              });

    m_packPaths.resize(tmpPacks.size());
    m_packs.resize(tmpPacks.size());
    for (unsigned i = 0; i < tmpPacks.size(); i++)
    {
        m_packs[i] = tmpPacks[i].first;
        m_packPaths[i] = tmpPacks[i].second;
    }

    packs = m_packs;
    return;
}

mck::WaveInfoDetail mck::SampleExplorer::LoadSample(unsigned packIdx, unsigned sampleIdx)
{
    WaveInfoDetail info;
    info.valid = false;
    info.packIdx = packIdx;
    info.sampleIdx = sampleIdx;

    if (m_isInitialized == false)
    {
        return info;
    }
    if (packIdx >= m_packs.size())
    {
        return info;
    }
    if (sampleIdx >= m_packs[packIdx].samples.size())
    {
        return info;
    }
    fs::path sndPath(m_packPaths[packIdx]);
    sndPath.append(m_packs[packIdx].samples[sampleIdx].path);
    info.path = sndPath.string();
    fs::path relPath(fs::relative(sndPath, fs::path(m_samplePath)));
    info.relPath = relPath.string();
    info.name = m_packs[packIdx].samples[sampleIdx].name;
    if (fs::exists(sndPath) == false)
    {
        return info;
    }

    info = helper::ImportWaveForm(sndPath.string(), m_sampleRate, m_waveBuffer[1 - m_curWave]);
    info.path = sndPath.string();
    info.relPath = relPath.string();
    info.name = m_packs[packIdx].samples[sampleIdx].name;
    info.packIdx = packIdx;
    info.sampleIdx = sampleIdx;

    m_waveInfo[1 - m_curWave] = info;
    return info;
}

mck::WaveInfoDetail mck::SampleExplorer::PlaySample(unsigned packIdx, unsigned sampleIdx, unsigned padIdx, sampler::Pad &padData)
{
    WaveInfoDetail ret;
    if (m_isInitialized == false)
    {
        return ret;
    }

    StopSample();

    char newWave = 1 - m_curWave;

    if (packIdx != m_waveInfo[newWave].packIdx || sampleIdx != m_waveInfo[newWave].sampleIdx || m_waveInfo[newWave].valid == false)
    {
        m_waveInfo[newWave] = LoadSample(packIdx, sampleIdx);
    }

    ret = m_waveInfo[newWave];

    if (ret.valid == false)
    {
        return ret;
    }

    PlayState state;
    state.active = true;
    state.stop = false;
    state.idx = 0;
    state.len = m_waveInfo[newWave].lengthSamps;
    state.pad = padIdx;
    state.gainL = (100.0f/127.0f) * padData.gainLeftLin;
    state.gainR = (100.0f/127.0f) * padData.gainRightLin;

    if (m_isProcessing.load())
    {
        std::mutex tmpMutex;
        std::unique_lock lock(tmpMutex);
        m_processCond.wait(lock);
    }

    m_state = state;
    m_curWave = newWave;

    return ret;
}
void mck::SampleExplorer::StopSample()
{
    if (m_isInitialized == false)
    {
        return;
    }
    if (m_isProcessing.load())
    {
        std::mutex tmpMutex;
        std::unique_lock lock(tmpMutex);
        m_processCond.wait(lock);
    }
    m_state.stop = true;
}

mck::WaveInfoDetail mck::SampleExplorer::GetSample(unsigned packIdx, unsigned sampleIdx, std::vector<std::vector<float>> &buffer)
{
    WaveInfoDetail ret;
    if (m_isInitialized == false)
    {
        return ret;
    }

    char newWave = 1 - m_curWave;

    if (packIdx != m_waveInfo[m_curWave].packIdx || sampleIdx != m_waveInfo[m_curWave].sampleIdx || m_waveInfo[m_curWave].valid == false)
    {
        m_waveInfo[newWave] = LoadSample(packIdx, sampleIdx);
        buffer = m_waveBuffer[newWave];
        return m_waveInfo[newWave];
    }
    else
    {
        buffer = m_waveBuffer[m_curWave];
        return m_waveInfo[m_curWave];
    }
}

std::string mck::SampleExplorer::GetSamplePath(unsigned packIdx, unsigned sampleIdx, bool relativePath)
{
    if (m_isInitialized == false)
    {
        return "";
    }
    if (packIdx >= m_packs.size())
    {
        return "";
    }
    if (sampleIdx >= m_packs[packIdx].samples.size())
    {
        return "";
    }
    fs::path path(m_samplePath);
    path.append(m_packPaths[packIdx]);
    path.append(m_packs[packIdx].samples[sampleIdx].path);
    if (relativePath)
    {
        path = fs::relative(path, fs::path(m_samplePath));
    }
    return path.string();
}

std::string mck::SampleExplorer::GetSampleName(unsigned packIdx, unsigned sampleIdx)
{
    if (m_isInitialized == false)
    {
        return "";
    }
    if (packIdx >= m_packs.size())
    {
        return "";
    }
    if (sampleIdx >= m_packs[packIdx].samples.size())
    {
        return "";
    }
    return m_packs[packIdx].samples[sampleIdx].name;
}

std::string mck::SampleExplorer::GetSampleId(unsigned packIdx, unsigned sampleIdx)
{
    if (m_isInitialized == false)
    {
        return "";
    }
    if (packIdx >= m_packs.size())
    {
        return "";
    }
    if (sampleIdx >= m_packs[packIdx].samples.size())
    {
        return "";
    }
    return m_packs[packIdx].samples[sampleIdx].id;
}

std::string mck::SampleExplorer::GetSampleType(unsigned packIdx, unsigned sampleIdx)
{
    if (m_isInitialized == false)
    {
        return "";
    }
    if (packIdx >= m_packs.size())
    {
        return "";
    }
    if (sampleIdx >= m_packs[packIdx].samples.size())
    {
        return "";
    }
    return m_packs[packIdx].categories[m_packs[packIdx].samples[sampleIdx].type];
}

mck::SamplePackSample mck::SampleExplorer::GetSampleMeta(unsigned packIdx, unsigned sampleIdx)
{
    if (m_isInitialized == false)
    {
        return mck::SamplePackSample();
    }
    if (packIdx >= m_packs.size())
    {
        return mck::SamplePackSample();
    }
    if (sampleIdx >= m_packs[packIdx].samples.size())
    {
        return mck::SamplePackSample();
    }
    return m_packs[packIdx].samples[sampleIdx];
}
mck::SamplePackSample mck::SampleExplorer::GetSampleMeta(const std::string &id)
{
    if (id != "")
    {
        for (auto &sp : m_packs)
        {
            for (auto &s : sp.samples)
            {
                if (s.id == id)
                {
                    return s;
                }
            }
        }
    }
    return SamplePackSample();
}

bool mck::SampleExplorer::ApplyEditCommand(SampleEdit &cmd, GuiWindow *gui)
{
    if (m_isInitialized == false)
    {
        return false;
    }
    if (cmd.cmd >= SCMD_LENGTH)
    {
        return false;
    }

    switch (cmd.cmd)
    {
    case SCMD_CREATE:
        switch (cmd.classType)
        {
        case SEC_PACK:
            CreatePack(cmd.stringValue);
            break;
        case SEC_CATEGORY:
            CreateCategory(cmd.stringValue, cmd.packIdx);
            break;
        case SEC_SAMPLE:
            ImportSample(cmd.stringValue, cmd.packIdx, cmd.categoryIdx, gui);
            break;
        default:
            return false;
        };
        break;
    default:
        break;
    };

    return true;
}

void mck::SampleExplorer::ProcessAudio(float *outLeft, float *outRight, unsigned nframes)
{
    if (m_isInitialized == false)
    {
        m_isProcessing = false;
        m_processCond.notify_all();
        return;
    }
    m_isProcessing = true;

    if (m_state.stop)
    {
        m_state.active = false;
        m_state.stop = false;
        return;
    }

    if (m_state.active)
    {
        unsigned samplesLeft = m_state.len - m_state.idx;
        if (samplesLeft <= nframes)
        {
            m_state.active = false;
        }
        if (m_waveBuffer[m_curWave].size() == 1)
        {
            // Compensate Mono Panning Law
            float gainL = std::min(1.0f, m_state.gainL * std::sqrt(2.0f));
            float gainR = std::min(1.0f, m_state.gainR * std::sqrt(2.0f));

            for (unsigned s = 0; s < std::min(samplesLeft, nframes); s++)
            {
                outLeft[s] += m_waveBuffer[m_curWave][0][s + m_state.idx] * gainL;
                outRight[s] += m_waveBuffer[m_curWave][0][s + m_state.idx] * gainR;
            }
        }
        else
        {
            for (unsigned s = 0; s < std::min(samplesLeft, nframes); s++)
            {
                outLeft[s] += m_waveBuffer[m_curWave][0][s + m_state.idx] * m_state.gainL;
                outRight[s] += m_waveBuffer[m_curWave][1][s + m_state.idx] * m_state.gainR;
            }
        }
        m_state.idx += nframes;
    }

    m_isProcessing = false;
    m_processCond.notify_all();
}

bool mck::SampleExplorer::WritePack(std::string path, SamplePack &pack)
{
    if (m_isInitialized == false)
    {
        return false;
    }

    fs::path packPath(path);
    fs::create_directories(packPath);

    if (fs::exists(packPath) == false)
    {
        return false;
    }

    packPath.append("config.mcksp");
    std::ofstream confFile(packPath);
    try
    {
        nlohmann::json j = pack;
        confFile << j.dump(4) << std::endl;
        confFile.close();
    }
    catch (std::exception &e)
    {
        std::printf("Failed to write sample pack file: %s", e.what());
        return false;
    }

    return true;
}

bool mck::SampleExplorer::UpdatePack(unsigned packIdx)
{
    if (m_isInitialized == false)
    {
        return false;
    }
    if (packIdx >= m_packs.size())
    {
        return false;
    }

    return WritePack(m_packPaths[packIdx], m_packs[packIdx]);
}

bool mck::SampleExplorer::CreatePack(std::string name)
{
    if (m_isInitialized == false)
    {
        return false;
    }

    SamplePack pack;
    name = name != "" ? name : "Sample Pack " + std::to_string(m_packs.size() + 1);
    fs::path packPath(m_samplePath);
    packPath.append(name);
    unsigned idx;
    pack.name = name;
    while (fs::exists(packPath))
    {
        packPath = fs::path(m_samplePath);
        pack.name = name + "_" + std::to_string(idx + 1);
        packPath.append(pack.name);
        idx++;
    }

    return WritePack(packPath.string(), pack);
}
bool mck::SampleExplorer::CreateCategory(std::string name, unsigned packIdx)
{
    if (m_isInitialized == false)
    {
        return false;
    }
    if (packIdx >= m_packs.size())
    {
        return false;
    }

    fs::path packPath(m_samplePath);
    packPath.append(m_packPaths[packIdx]);

    std::string catName = name != "" ? name : "CAT " + std::to_string(m_packs[packIdx].categories.size() + 1);
    name = catName;

    unsigned idx = 0;
    fs::path catPath(packPath);
    catPath.append(catName);

    while (fs::exists(catPath))
    {
        catPath = fs::path(packPath);
        catName = name + "_" + std::to_string(idx + 1);
        catPath.append(catName);
        idx++;
    }
    fs::create_directories(catPath);

    m_packs[packIdx].categories.push_back(catName);

    UpdatePack(packIdx);

    return true;
}
bool mck::SampleExplorer::ImportSample(std::string path, unsigned packIdx, unsigned categoryIdx, GuiWindow *gui)
{
    if (m_isInitialized == false)
    {
        return false;
    }
    if (packIdx >= m_packs.size())
    {
        return false;
    }
    if (categoryIdx >= m_packs[packIdx].categories.size())
    {
        return false;
    }

    std::vector<std::string> files;
    // gui->ShowOpenFileDialog("Import one or more sample files", "audio/wav", files, true);

    fs::path catPath(m_samplePath);
    catPath.append(m_packPaths[packIdx]).append(m_packs[packIdx].categories[categoryIdx]);

    unsigned sampleIndex = 1 + std::count_if(m_packs[packIdx].samples.begin(),
                                             m_packs[packIdx].samples.end(),
                                             [&categoryIdx](const SamplePackSample &s)
                                             { return s.type == categoryIdx; });

    for (auto &f : files)
    {
        // Check file
        fs::path oldPath(f);
        if (fs::exists(oldPath) == false)
        {
            continue;
        }

        // Copy file into category folder
        fs::path newPath(catPath);
        std::string idxStr = std::to_string(sampleIndex);
        idxStr.insert(idxStr.begin(), 3 - idxStr.size(), '0');
        newPath.append(idxStr + "_" + oldPath.filename().string());
        if (fs::copy_file(oldPath, newPath) == false)
        {
            continue;
        }

        // Update m_packs
        fs::path relPath = fs::relative(newPath, fs::path(m_samplePath).append(m_packPaths[packIdx]));
        SamplePackSample sample;
        sample.path = relPath.string();
        sample.name = oldPath.stem().string();
        sample.name = std::regex_replace(sample.name, std::regex("_"), " ");
        sample.type = categoryIdx;
        sample.index = sampleIndex++;
        m_packs[packIdx].samples.push_back(sample);
    }

    return UpdatePack(packIdx);
}
