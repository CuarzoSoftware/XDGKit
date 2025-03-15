#ifndef XDGPARSERS_H
#define XDGPARSERS_H

#include <filesystem>
#include <string>
#include <memory>
#include <unordered_map>

namespace XDG
{
    typedef std::unordered_map<std::string,std::unordered_map<std::string, std::string>> XDGINI;

    namespace XDGParsers
    {
        std::shared_ptr<XDGINI> ParseINI(const std::filesystem::path &iniFile) noexcept;
    }
}

#endif // XDGPARSERS_H
