#include "ConfigFile.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

mck::ConfigFile::ConfigFile()
    : m_fileLoaded(false),
      m_filePath(""),
      m_config()
{
}

mck::ConfigFile::~ConfigFile()
{
}

bool mck::ConfigFile::ReadFile(std::string path)
{
    if (path == "")
    {
        return false;
    }
    if (VerifyPath(path) == false)
    {
        return false;
    }

    std::ifstream confFile(path);
    nlohmann::json j;
    confFile >> j;
    confFile.close();
    try
    {
        m_config = j.get<mck::sampler::Config>();
    }
    catch (std::exception &e)
    {
        std::printf("Failed to convert the config file: %s", e.what());
        return false;
    }

    m_filePath = path;
    return true;
}

bool mck::ConfigFile::WriteFile(std::string path)
{
    if (path == "")
    {
        return false;
    }
    VerifyPath(path);

    std::ofstream confFile(path);
    try
    {
        nlohmann::json j = m_config;
        confFile << j.dump(4) << std::endl;
        confFile.close();
    }
    catch (std::exception &e)
    {
        std::printf("Failed to write config file: %s", e.what());
        return false;
    }
    return true;
}

bool mck::ConfigFile::GetConfig(sampler::Config &config)
{
    config = m_config;
    return m_fileLoaded;
}

void mck::ConfigFile::SetConfig(sampler::Config &config)
{
    m_config = config;
}

std::string mck::ConfigFile::GetHomeDir()
{
    struct passwd *pw = getpwuid(getuid());
    std::string path = std::string(pw->pw_dir);
    return path;
}

bool mck::ConfigFile::VerifyPath(std::string path)
{
    std::filesystem::path fp(path);
    if (std::filesystem::exists(fp.parent_path()) == false)
    {
        std::filesystem::create_directories(fp.parent_path());
    }

    return std::filesystem::exists(path);
}