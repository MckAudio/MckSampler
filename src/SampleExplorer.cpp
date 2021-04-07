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
      m_packPaths()
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

mck::SampleInfo mck::SampleExplorer::LoadSample(unsigned packIdx, unsigned sampleIdx)
{
  SampleInfo info;
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
  if (fs::exists(sndPath) == false)
  {
    return info;
  }

  SNDFILE *snd;
  SF_INFO sndInfo;

  snd = sf_open(sndPath.c_str(), SFM_READ, &sndInfo);
  if (snd == nullptr)
  {
    return info;
  }

  info.numChans = sndInfo.channels;
  info.lengthSamps = sndInfo.frames;
  info.sampleRate = sndInfo.samplerate;
  info.lengthMs = (unsigned)std::floor(1000.0 * (double)info.lengthSamps / (double)info.sampleRate);

  if (m_curSampleBuffer != nullptr)
  {
    free(m_curSampleBuffer);
    m_curSampleBuffer = nullptr;
  }

  m_curSampleBuffer = (float *)malloc(info.lengthSamps * info.numChans * sizeof(float));
  memset(m_curSampleBuffer, 0, info.lengthSamps * info.numChans * sizeof(float));

  // Read Samples from File
  unsigned numSrcFrames = sf_readf_float(snd, m_curSampleBuffer, sndInfo.frames);

  // Samplerate conversion
  if (info.sampleRate != m_sampleRate)
  {
  }

  // 500us
  unsigned sampsPerMs = (unsigned)std::ceil((double)info.sampleRate / 2000.0);
  unsigned lengthMs = (unsigned)std::floor(2000.0 * (double)info.lengthSamps / (double)info.sampleRate);

  // Read Waveform
  info.waveForm.resize(info.numChans);
  for (unsigned c = 0; c < info.numChans; c++)
  {
    info.waveForm[c].resize(lengthMs, 0.0);
  }
  for (unsigned i = 0; i < lengthMs; i++)
  {
    for (unsigned c = 0; c < info.numChans; c++)
    {
      float wMax = 0.0f;
      float wMin = 0.0f;
      for (unsigned s = i * sampsPerMs; s < std::min((i + 1) * sampsPerMs, info.lengthSamps); s++)
      {
        float sample = m_curSampleBuffer[s * info.numChans + c];
        if (sample > wMax)
        {
          wMax = sample;
        }
        else if (sample < wMin)
        {
          wMin = sample;
        }
      }
      info.waveForm[c][i] = std::abs(wMin) > wMax ? wMin : wMax;
    }
  }

  sf_close(snd);

  info.valid = true;
  m_curInfo = info;
  return info;
}

mck::SampleInfo mck::SampleExplorer::PlaySample(unsigned packIdx, unsigned sampleIdx)
{
  SampleInfo ret;
  if (m_isInitialized == false)
  {
    return ret;
  }

  StopSample();

  if (packIdx != m_curInfo.packIdx || sampleIdx != m_curInfo.sampleIdx || m_curInfo.valid == false)
  {
    ret = LoadSample(packIdx, sampleIdx);
  } else {
    ret = m_curInfo;
  }

  if (m_curInfo.valid == false)
  {
    return ret;
  }

  m_waveBuffer.resize(m_curInfo.numChans);
  for (unsigned c = 0; c < m_curInfo.numChans; c++)
  {
    m_waveBuffer[c].resize(m_curInfo.lengthSamps, 0.0);
  }

  for (unsigned s = 0; s < m_curInfo.lengthSamps; s++)
  {
    for (unsigned c = 0; c < m_curInfo.numChans; c++)
    {
      m_waveBuffer[c][s] = m_curSampleBuffer[s * m_curInfo.numChans + c];
    }
  }

  PlayState state;
  state.active = true;
  state.stop = false;
  state.idx = 0;
  state.len = m_curInfo.lengthSamps;

  if (m_isProcessing.load())
  {
    std::mutex tmpMutex;
    std::unique_lock lock(tmpMutex);
    m_processCond.wait(lock);
  }

  m_state = state;

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
    if (m_waveBuffer.size() == 1)
    {
      for (unsigned s = 0; s < std::min(samplesLeft, nframes); s++)
      {
        outLeft[s] += m_waveBuffer[0][s + m_state.idx] * 0.707;
        outRight[s] += m_waveBuffer[0][s + m_state.idx] * 0.707;
      }
    }
    else
    {
      for (unsigned s = 0; s < std::min(samplesLeft, nframes); s++)
      {
        outLeft[s] += m_waveBuffer[0][s + m_state.idx];
      }
      for (unsigned s = 0; s < std::min(samplesLeft, nframes); s++)
      {
        outRight[s] += m_waveBuffer[1][s + m_state.idx];
      }
    }
    m_state.idx += nframes;
  }

  m_isProcessing = false;
  m_processCond.notify_all();
}