#pragma once

#include <string>
#include "Config.hpp"

namespace mck {
    class ConfigFile {
        public:
            ConfigFile();
            ~ConfigFile();

            bool ReadFile(std::string path);
            bool WriteFile(std::string path);

            bool GetConfig(sampler::Config &config);
            void SetConfig(sampler::Config &config);

            static std::string GetHomeDir();
        private:
            bool VerifyPath(std::string path);
            
            bool m_fileLoaded;
            std::string m_filePath;
            sampler::Config m_config;
    };
}
