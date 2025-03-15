#ifndef XDGICONTHEMEMANAGER_H
#define XDGICONTHEMEMANAGER_H

#include <XDGKit/XDGIconTheme.h>
#include <filesystem>
#include <unordered_map>
#include <vector>

class XDG::XDGIconThemeManager
{
public:
    // By default
    // $HOME/.icons
    // $XDG_DATA_DIRS/icons
    // /usr/share/pixmaps
    const std::vector<std::filesystem::path> &searchDirs() const noexcept
    {
        return m_searchDirs;
    }

    const std::unordered_map<std::string, XDGIconTheme> &themes() const noexcept
    {
        return m_themes;
    }

private:
    friend class XDGKit;
    XDGIconThemeManager(XDGKit &kit) noexcept;
    void restoreDefaultSearchDirs() noexcept;
    void findThemes() noexcept;
    XDGKit &m_kit;
    std::vector<std::filesystem::path> m_searchDirs;
    std::unordered_map<std::string, XDGIconTheme> m_themes;
};

#endif // XDGICONTHEMEMANAGER_H
