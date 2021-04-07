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

  m_curSampleBuffer = (float *)malloc(info.lengthSamps * info.numChans * sizeof(float));
  memset(m_curSampleBuffer, 0, info.lengthSamps * info.numChans * sizeof(float));

  // Read Samples from File
  unsigned numSrcFrames = sf_readf_float(snd, m_curSampleBuffer, sndInfo.frames);

  // Samplerate conversion
  if (info.sampleRate != m_sampleRate)
  {
  }

  // 1/5 ms
  unsigned sampsPerMs = (unsigned)std::ceil((double)info.sampleRate / 5000.0);
  unsigned lengthMs = (unsigned)std::floor(5000.0 * (double)info.lengthSamps / (double)info.sampleRate);

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
      for (unsigned s = i * sampsPerMs; s < std::min((i + 1) * sampsPerMs, info.lengthSamps - i * sampsPerMs); s++)
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

  free(m_curSampleBuffer);

  info.valid = true;
  m_curInfo = info;
  return info;
}

bool mck::SampleExplorer::PlaySample(unsigned packIdx, unsigned sampleIdx)
{
  if (m_isInitialized == false)
  {
    return false;
  }

  return true;
}
void mck::SampleExplorer::StopSample()
{
}
void mck::SampleExplorer::ProcessAudio(float **output, unsigned numChannels)
{
}