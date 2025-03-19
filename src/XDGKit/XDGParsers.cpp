#include <XDGKit/XDGParsers.h>
#include <fstream>

using namespace XDG;

std::shared_ptr<XDGINI> XDGParsers::ParseINI(const std::filesystem::path &iniFile) noexcept
{
    std::shared_ptr<XDGINI> data { std::make_unique<XDGINI>() };
    std::ifstream file(iniFile);

    if (!file.is_open())
        return data;

    std::string line;
    std::string currentSection;

    while (std::getline(file, line))
    {
        // Trim leading/trailing whitespace
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        // Skip comments and blank lines
        if (line.empty() || line[0] == '#' || line[0] == ';')
            continue;

        // Handle section headers [SectionName]
        if (line.front() == '[' && line.back() == ']')
        {
            currentSection = line.substr(1, line.size() - 2); // Extract section name
            currentSection.erase(0, currentSection.find_first_not_of(" \t"));
            currentSection.erase(currentSection.find_last_not_of(" \t") + 1);
            (*data)[currentSection] = {};
        }
        // Handle key-value pairs
        else
        {
            size_t delimiterPos = line.find('=');
            if (delimiterPos != std::string::npos)
            {
                std::string key = line.substr(0, delimiterPos);
                std::string value = line.substr(delimiterPos + 1);

                // Trim whitespace from key and value
                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);

                // Add the key-value pair to the current section
                (*data)[currentSection][key] = value;
            }
        }
    }

    file.close();
    return data;
}

std::vector<std::string> XDGParsers::splitString(const std::string &input, char delimiter, bool trim) noexcept
{
    std::vector<std::string> result;
    result.reserve(32);
    std::stringstream ss(input);
    std::string token;

    while (std::getline(ss, token, delimiter))
    {
        if (trim) result.emplace_back(trimString(token));
        else result.emplace_back(token);
    }

    return result;
}
