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
     * @brief Configuration options.
     */
    struct Options
    {
        Options() {}

        /**
         * @brief Enables icon theme caching.
         *
         * When set to `true`, icon themes will be loaded from cached files
         * stored in `~/.cache/xdgkit/icon_themes`, if available. This can
         * significantly improve performance.
         *
         * If `false`, the cache will be ignored, and themes will be loaded directly.
         */
        bool useIconThemesCache { true };
    };

    /**
     * @brief Creates a new instance of XDGKit.
     */
    XDGKit(const Options &options = Options()) noexcept;

    /**
     * @brief Creates a new instance of XDGKit.
     *
     * @return A shared pointer to the newly created XDGKit instance.
     */
    static std::shared_ptr<XDGKit> Make(const Options &options = Options()) noexcept;

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
     * @brief Retrieves the configuration options used to create this instance.
     */
    const Options &options() const noexcept
    {
        return m_options;
    }

private:
    friend class XDGIconDirectory;
    friend class XDGIconThemeManager;
    friend class XDGIconTheme;
    friend class XDGIcon;
    std::string_view saveOrGetString(const std::string &string, bool *inserted = nullptr) noexcept
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
    Options m_options;
    XDGIconThemeManager m_iconThemeManager;
    std::filesystem::path m_homeDir;
    std::vector<std::filesystem::path> m_dataDirs;
    std::unordered_set<std::string> m_stringPool;
};

#endif // XDGKIT_H
