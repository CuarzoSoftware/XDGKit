#ifndef XDGKIT_H
#define XDGKIT_H

#include <CZ/XDG/XDGIconThemeManager.h>
#include <CZ/XDG/XDGMimeDatabase.h>
#include <filesystem>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <vector>

/**
 * @brief Core class.
 *
 * When destroyed, all references to themes and icons are invalidated.
 */
class CZ::XDGKit
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
         * stored in `/var/cache/xdgkit/icon_themes`, if available. This can
         * significantly improve performance.
         *
         * If `false`, the cache files will be ignored, and themes will be loaded directly.
         */
        bool useIconThemesCache { true };

        /**
         * @brief Automatically reload icon themes.
         *
         * Icon themes are reloaded when the cache has changed.
         *
         * @note Cache changes are only checked when `CZ::XDGIconThemeManager::findIcon()` is called.
         *       To manually trigger a check, use `CZ::XDGIconThemeManager::reloadThemes(true)`.
         */
        bool autoReloadCache { true };
    };

    /**
     * @brief Creates a new instance of XDGKit.
     */
    XDGKit(const Options &options = Options()) noexcept;

    /**
     * @brief Creates a new instance of XDGKit.
     *
     * @return A shared pointer to the newly created XDGKit instance. Never `nullptr`.
     */
    static std::shared_ptr<XDGKit> Make(const Options &options = Options()) noexcept;

    /**
     * @brief Provides access to functionality related to icon themes.
     */
    XDGIconThemeManager &iconThemeManager() noexcept
    {
        return m_iconThemeManager;
    }

    /**
     * @brief Retrieves the current user's username.
     */
    const std::string &username() const noexcept
    {
        return m_user;
    }

    /**
     * @brief Retrieves the current user's home directory as an absolute path.
     */
    const std::filesystem::path &homeDir() const noexcept
    {
        return m_homeDir;
    }

    /**
     * @brief Retrieves valid absolute paths found in `XDG_DATA_DIRS`.
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

    /**
     * @brief Finds the desktop entry (.desktop file) of an application.
     *
     * Looks for `<appId>.desktop` (and its lowercase variant, which covers most X11 WM_CLASS based
     * ids) in `$XDG_DATA_HOME/applications` and every `$XDG_DATA_DIRS/applications`. Results,
     * including misses, are cached.
     *
     * @return The entry, or nullptr if none was found.
     */
    std::shared_ptr<const XDGDesktopEntry> desktopEntry(const std::string &appId) noexcept;

    /**
     * @brief File types and their default applications (loaded on first use).
     */
    XDGMimeDatabase &mimeDatabase() noexcept { return m_mimeDatabase; }

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
    std::string m_user;
    std::filesystem::path m_homeDir;
    std::vector<std::filesystem::path> m_dataDirs;
    std::unordered_set<std::string> m_stringPool;
    std::unordered_map<std::string, std::shared_ptr<const XDGDesktopEntry>> m_desktopEntries;
    XDGMimeDatabase m_mimeDatabase { *this };
};

#endif // XDGKIT_H
