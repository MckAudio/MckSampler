#include "SampleExplorer.hpp"
#include <filesystem>
#include <nlohmann/json.hpp>
#include <cstdio>
#include <fstream>

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
  if (sp.is_absolute() == false) {
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
            if (dp.path().is_absolute()) {
              m_packPaths.push_back(dp.path());
            } else {
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
bool mck::SampleExplorer::PlaySample(unsigned packIdx, unsigned sampleIdx)
{
  if (m_isInitialized == false)
  {
    return false;
  }

  return true;
}
void mck::SampleExplorer::ProcessAudio(float **output, unsigned numChannels)
{
}