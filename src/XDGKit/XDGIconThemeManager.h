#ifndef XDGICONTHEMEMANAGER_H
#define XDGICONTHEMEMANAGER_H

#include <XDGKit/XDGIconTheme.h>
#include <unordered_set>
#include <filesystem>
#include <vector>

/**
 * @brief Utility for finding icons.
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
     *
     * - ~/.icons
     * - ~/.local/share/icons
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
     * @return A constant reference to a map where the key is the theme's
     *         directory basename (e.g. "Adwaita"), and the value is the corresponding XDGIconTheme object.
     */
    const std::unordered_map<std::string, std::shared_ptr<XDGIconTheme>> &themes() const noexcept
    {
        return m_themes;
    }

    /**
     * @brief Reloads all available themes.
     *
     * Rescans the system and reloads all detected themes, replacing any previously loaded data.
     *
     * Use this function when the set of themes in the system has changed (e.g., after installation or removal).
     *
     * @warning All existing references to themes, theme directories, and icons will be invalidated after this call.
     */
    void reloadThemes() noexcept;

    /**
     * @brief Searches for an icon within the specified themes.
     *
     * This function attempts to locate an icon that matches the provided criteria
     * (name, size, scale, and extensions) within the given list of themes.
     *
     * @warning It is not recommended to keep a reference to the returned icon, as it will be invalidated
     *          when `reloadThemes()` is called or when the `XDGKit` instance is removed.
     *
     * @param icon The name of the icon to search for.
     * @param size The desired nominal size of the icon.
     * @param scale The scale factor of the icon. Defaults to 1.
     * @param extensions Flags indicating the acceptable image file extensions.
     * @param themes A list of theme names to search, in the specified order.
     *               An empty string ("") serves as a placeholder to search in all themes available.
     * @param contexts Flags to limit the search to the given XDGIconDirectory::Context (s).
     * @return A pointer to the closest matching icon, or `nullptr` if no match is found.
     */
    const XDGIcon *findIcon(
        const std::string &icon,
        int32_t size, int32_t scale = 1,
        uint32_t extensions = XDGIcon::PNG | XDGIcon::SVG,
        const std::vector<std::string> &themes = { "" },
        uint32_t contexts = XDGIconDirectory::AnyContext) const noexcept;

    /**
     * @brief Suggests to the OS to evict all mapped cache files from memory.
     *
     * Use this function when themes are not expected to be used in the near future,
     * helping to free up memory by prompting the OS to release the data.
     */
    void evictCache() noexcept;
private:
    struct Search
    {
        std::string icon;
        bool inserted { false };
        int32_t size;
        int32_t scale;
        int32_t bufferSize;
        uint32_t extensions;
        uint32_t contexts;
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
    std::unordered_map<std::string, std::shared_ptr<XDGIconTheme>> m_themes;
    XDGKit &m_kit;
};

#endif // XDGICONTHEMEMANAGER_H
