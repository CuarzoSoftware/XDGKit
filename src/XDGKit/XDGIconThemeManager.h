#ifndef XDGICONTHEMEMANAGER_H
#define XDGICONTHEMEMANAGER_H

#include <XDGKit/XDGIconTheme.h>
#include <filesystem>
#include <unordered_map>
#include <vector>

class XDG::XDGIconThemeManager
{
public:
    /**
     * @brief Retrieves the directories to search for icon themes, in order of precedence.
     *
     * The default search directories are:
     * - $HOME/.icons
     * - $XDG_DATA_DIRS/icons
     * - /usr/share/pixmaps
     *
     * @return A constant reference to a vector containing the search directories.
     */
    const std::vector<std::filesystem::path> &searchDirs() const noexcept
    {
        return m_searchDirs;
    }

    /**
     * @brief Retrieves all discovered icon themes.
     *
     * @return A constant reference to an unordered map where the key is the theme's
     *         directory name, and the value is the corresponding XDGIconTheme object.
     */
    const std::unordered_map<std::string, XDGIconTheme> &themes() const noexcept
    {
        return m_themes;
    }

private:
    friend class XDGKit;
    XDGIconThemeManager(XDGKit &kit) noexcept;
    void restoreDefaultSearchDirs() noexcept;
    void findThemes() noexcept;
    void sanitizeThemes() noexcept;
    XDGKit &m_kit;
    std::vector<std::filesystem::path> m_searchDirs;
    std::unordered_map<std::string, XDGIconTheme> m_themes;
};

#endif // XDGICONTHEMEMANAGER_H
