#include "SampleExplorer.hpp"
#include <filesystem>
#include <nlohmann/json.hpp>
#include <cstdio>
#include <fstream>
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

  m_packPaths.clear();
  m_packs.clear();

  fs::path sp(m_samplePath);

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
            std::ifstream spFile(cfp.path());
            nlohmann::json j;
            spFile >> j;
            spFile.close();
            try
            {
              SamplePack sp = j;
              m_packs.push_back(sp);
            }
            catch (std::exception &e)
            {
              std::printf("File %s is malformed: %s\n", cfp.path().c_str(), e.what());
              continue;
            }
            if (dp.path().is_absolute())
            {
              m_packPaths.push_back(dp.path());
            }
            else
            {
              m_packPaths.push_back(fs::absolute(dp.path()));
            }
          }
        }
      }
    }
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

mck::WaveInfoDetail mck::SampleExplorer::PlaySample(unsigned packIdx, unsigned sampleIdx)
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
      for (unsigned s = 0; s < std::min(samplesLeft, nframes); s++)
      {
        outLeft[s] += m_waveBuffer[m_curWave][0][s + m_state.idx] * 0.707;
        outRight[s] += m_waveBuffer[m_curWave][0][s + m_state.idx] * 0.707;
      }
    }
    else
    {
      for (unsigned s = 0; s < std::min(samplesLeft, nframes); s++)
      {
        outLeft[s] += m_waveBuffer[m_curWave][0][s + m_state.idx];
      }
      for (unsigned s = 0; s < std::min(samplesLeft, nframes); s++)
      {
        outRight[s] += m_waveBuffer[m_curWave][1][s + m_state.idx];
      }
    }
    m_state.idx += nframes;
  }

  m_isProcessing = false;
  m_processCond.notify_all();
}