#ifndef XDGKIT_H
#define XDGKIT_H

#include <XDGKit/XDGIconThemeManager.h>
#include <filesystem>
#include <memory>
#include <unordered_set>
#include <vector>

/**
 * @brief Core class.
 *
 * When destroyed, all references to themes and icons are invalidated.
 */
class XDG::XDGKit
{
public:

    /**
     * @brief Creates a new instance of XDGKit.
     */
    XDGKit() noexcept;

    /**
     * @brief Creates a new instance of XDGKit.
     *
     * @return A shared pointer to the newly created XDGKit instance.
     */
    static std::shared_ptr<XDGKit> Make() noexcept;

    /**
     * @brief Provides access to functionality related to icon themes.
     *
     * @return A reference to the icon theme manager.
     */
    XDGIconThemeManager &iconThemeManager() noexcept
    {
        return m_iconThemeManager;
    }

    /**
     * @brief Retrieves the current user's home directory as an absolute path.
     *
     * @return A constant reference to the user's home directory path.
     */
    const std::filesystem::path &homeDir() const noexcept
    {
        return m_homeDir;
    }

    /**
     * @brief Retrieves valid absolute paths found in `XDG_DATA_DIRS`.
     *
     * @return A constant reference to a vector containing the data directory paths.
     */
    const std::vector<std::filesystem::path> &dataDirs() const noexcept
    {
        return m_dataDirs;
    }

    /**
     * @brief Retrieves the pointer of the given string in the string pool.
     *
     * @param string String to query.
     * @return the pointer of the string or `nullptr` if not found.
     */
    const char *getString(const std::string &string) const noexcept
    {
        const auto &it = m_stringPool.find(string);

        if (it == m_stringPool.end())
            return nullptr;

        return it->c_str();
    }

private:
    friend class XDGIconDirectory;
    friend class XDGIconThemeManager;
    friend class XDGIconTheme;
    friend class XDGIcon;
    const char *saveOrGetString(const std::string &string, bool *inserted = nullptr) noexcept
    {
        if (inserted)
        {
            const auto &it = m_stringPool.insert(string);
            *inserted = it.second;
            return it.first->c_str();
        }

        return m_stringPool.insert(string).first->c_str();
    }
    void initHomeDir() noexcept;
    void rescanDataDirs() noexcept;
    XDGIconThemeManager m_iconThemeManager;
    std::filesystem::path m_homeDir;
    std::vector<std::filesystem::path> m_dataDirs;
    std::unordered_set<std::string> m_stringPool;
};

#endif // XDGKIT_H
