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
class XDG::XDGINI : public std::unordered_map<std::string, std::unordered_map<std::string, std::string>>
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

    // Sum of all string sizes
    uint64_t nBytes() const noexcept { return m_nBytes; }
private:
    uint64_t m_nBytes { 0 };
};

class XDG::XDGINIView : public std::unordered_map<std::string_view, std::unordered_map<std::string_view, std::string_view>>
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
    static std::shared_ptr<XDGINIView> Load(const std::filesystem::path &iniFile) noexcept;

    // from serialized data (not regular INI format)
    // the data must remain valid, size is just to validate the data
    static std::shared_ptr<XDGINIView> FromData(char *data, size_t size) noexcept;
    XDGINIView& operator=(XDGINIView &&other) noexcept
    {
        if (this != &other)
        {
            clear();
            std::unordered_map<std::string_view, std::unordered_map<std::string_view, std::string_view>>::operator=(std::move(other));
            m_map = other.m_map;
            m_mapSize = other.m_mapSize;
            m_tmp = other.m_tmp;

            other.m_map = nullptr;
            other.m_mapSize = 0;
            other.m_tmp = nullptr;
        }
        return *this;
    }

    void clear() noexcept;

    const void *map() const noexcept { return m_map; }
    uint64_t mapSize() const noexcept { return m_mapSize; }

    ~XDGINIView();
private:
    void *m_map { nullptr };
    uint64_t m_mapSize { 0 };
    FILE *m_tmp { nullptr };
};

#endif // XDGINI_H
