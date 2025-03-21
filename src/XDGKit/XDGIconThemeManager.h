#ifndef XDGICONTHEMEMANAGER_H
#define XDGICONTHEMEMANAGER_H

#include <XDGKit/XDGIconTheme.h>
#include <filesystem>
#include <map>
#include <vector>

/**
 * @brief Utility for Finding Icons.
 */
class XDG::XDGIconThemeManager
{
public:
    /**
     * @brief Handle to the parent kit.
     */
    XDGKit &kit() const noexcept
    {
        return m_kit;
    }

    /**
     * @brief Retrieves the directories to search for icon themes, in order of precedence.
     *
     * The default search directories in order are:
     * - $HOME/.icons
     * - $HOME/.local/share/icons
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
     *         directory basename, and the value is the corresponding XDGIconTheme object.
     */
    const std::map<std::string, std::shared_ptr<XDGIconTheme>> &themes() const noexcept
    {
        return m_themes;
    }

    /**
     * @brief Searches for an icon within the specified themes.
     *
     * This function attempts to locate an icon that matches the provided criteria
     * (name, size, scale, and extensions) within the given list of themes.
     *
     * @param icon The name of the icon to search for.
     * @param size The desired nominal size of the icon.
     * @param scale The scale factor of the icon. Defaults to 1.
     * @param extensions Flags indicating the acceptable image file extensions.
     * @param themes A list of theme names to search, in the specified order.
     *               An empty string ("") serves as a placeholder to search in all themes available.
     * @return A pointer to the closest matching icon, or `nullptr` if no match is found.
     */
    const XDGIcon *findIcon(const std::string &icon, int32_t size, int32_t scale = 1, uint32_t extensions = XDGIcon::PNG | XDGIcon::SVG, const std::vector<std::string> &themes = { "" }) const noexcept;

    /**
     * @brief Clears the `indexData()` of all themes stored in `XDGIconTheme`.
     *
     * This function is useful for freeing up memory when the additional parameters
     * defined in `index.theme` files are no longer needed. It ensures that the
     * theme data is released, reducing memory usage.
     *
     * @note This operation is non-recoverable. Once cleared, the `indexData()`
     *       for all themes will need to be reloaded if accessed again.
     */
    void clearThemesIndexData() noexcept;

private:
    struct Search
    {
        const char *icon;
        int32_t size;
        int32_t scale;
        int32_t bufferSize;
        uint32_t extensions;
        std::vector<std::shared_ptr<XDGIconTheme>> themes;
        std::unordered_set<std::shared_ptr<XDGIconTheme>> visitedThemes;
        int32_t bestDistance { std::numeric_limits<int32_t>::max() };
        const XDGIcon *bestIcon { nullptr };
    };
    friend class XDGKit;
    XDGIconThemeManager(XDGKit &kit) noexcept : m_kit(kit) {}
    void restoreDefaultSearchDirs() noexcept;
    void findThemes() noexcept;
    void sanitizeThemes() noexcept;
    const XDGIcon *findIconHelper(Search &search, std::shared_ptr<XDGIconTheme> theme) const noexcept;
    bool directoryMatchesSize(Search &search, const XDGIconDirectory &dir) const noexcept;
    int32_t directorySizeDistance(Search &search, const XDGIconDirectory &dir) const noexcept;
    std::vector<std::filesystem::path> m_searchDirs;
    std::map<std::string, std::shared_ptr<XDGIconTheme>> m_themes;
    XDGKit &m_kit;
};

#endif // XDGICONTHEMEMANAGER_H
