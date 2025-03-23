#ifndef XDGINI_H
#define XDGINI_H

#include <XDGKit/XDGNamespace.h>
#include <filesystem>
#include <string>
#include <unordered_map>

/**
 * @brief A parsed INI file.
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
class XDG::XDGINI : public std::unordered_map<std::string,std::unordered_map<std::string, std::string>>
{
public:
    /**
     * @brief Loads and parses an INI file.
     *
     * @param iniFile The path to the INI file to be loaded and parsed.
     *
     * @return Always returns a valid pointer to an XDGINI object. The returned pointer may reference
     *         an empty object if an error occurs.
     */
    static std::shared_ptr<XDGINI> Load(const std::filesystem::path &iniFile) noexcept;
};

#endif // XDGINI_H
