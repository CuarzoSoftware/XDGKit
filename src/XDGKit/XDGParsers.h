#ifndef XDGPARSERS_H
#define XDGPARSERS_H

#include <filesystem>
#include <string>
#include <unordered_set>
#include <vector>
#include <memory>
#include <unordered_map>

namespace XDG
{
    /**
     * @brief Represents a parsed INI file.
     *
     * The content is organized in nested unordered maps. The top-level map represents
     * the sections, while the nested maps hold the key-value pairs within each section.
     *
     * All values are stored as strings.
     *
     * Example usage:
     * @code
     * const std::string &value = parsedIni["section"]["key"];
     * @endcode
     */
    typedef std::unordered_map<std::string,std::unordered_map<std::string, std::string>> XDGINI;

    namespace XDGParsers
    {
        /**
         * @brief Loads and parses an INI file.
         *
         * @param iniFile The path to the INI file to be loaded and parsed.
         *
         * @return Always returns a valid pointer to an XDGINI object. The returned pointer may reference
         *         an empty object if errors occur.
         */
        std::shared_ptr<XDGINI> ParseINI(const std::filesystem::path &iniFile) noexcept;

        inline std::string trimString(const std::string &str) noexcept
        {
            const size_t start { str.find_first_not_of(" \t") };
            const size_t end { str.find_last_not_of(" \t") };
            return (start != std::string::npos && end != std::string::npos) ? str.substr(start, end - start + 1) : "";
        }

        std::vector<std::string> splitString(const std::string &input, char divider, bool trim = true) noexcept;

        template<class T>
        inline void removeDuplicates(std::vector<T> &vec) noexcept
        {
            std::unordered_set<T> seen;
            auto it = vec.begin();

            while (it != vec.end())
            {
                if (seen.find(*it) != seen.end())
                    it = vec.erase(it);
                else
                {
                    seen.insert(*it);
                    ++it;
                }
            }
        }
    }
}

#endif // XDGPARSERS_H
