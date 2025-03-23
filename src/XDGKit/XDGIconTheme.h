#ifndef XDGICONTHEME_H
#define XDGICONTHEME_H

#include <XDGKit/XDGIconDirectory.h>
#include <XDGKit/XDGINI.h>
#include <filesystem>
#include <string>
#include <vector>

/**
 * @brief An icon theme.
 *
 * This class encapsulates the properties and data of an icon theme, including
 * its name, directories, fallback themes, and associated metadata.
 *
 * Icon themes are loaded lazily. The loading typically occurs whenever `XDGIconThemeManager::findIcon()`
 * attempts to access `iconDirectories()` or `scaledIconDirectories()` for the first time.
 */
class XDG::XDGIconTheme
{
public:
    XDGIconTheme(XDGKit &kit) noexcept;

    /**
     * @brief Handle to the parent kit.
     */
    XDGKit &kit() const noexcept
    {
        return m_kit;
    }

    /**
     * @brief Verifies whether the theme has been loaded.
     *
     * The theme is loaded lazily when either `iconDirectories()` or `scaledIconDirectories()`
     * is accessed for the first time.
     */
    bool initialized() const noexcept
    {
        return m_initialized;
    }

    /**
     * @brief Retrieves the theme's directory base name.
     *
     * @return A constant reference to the theme's base name.
     */
    const std::string &name() const noexcept
    {
        return *m_name;
    }

    /**
     * @brief Retrieves the theme's display name.
     *
     * @return A constant reference to the theme's display name.
     */
    const std::string &displayName() const noexcept
    {
        return m_displayName;
    }

    /**
     * @brief Retrieves the theme's long name or comment.
     *
     * @return A constant reference to the theme's comment.
     */
    const std::string &comment() const noexcept
    {
        return m_comment;
    }

    /**
     * @brief Retrieves themes inherited by this theme.
     *
     * Inherited themes are used as fallback if an icon is not found in this theme.
     *
     * @return A constant reference to a vector of inherited theme names.
     */
    const std::vector<std::string> &inherits() const noexcept
    {
        return m_inherits;
    }

    /**
     * @brief Retrieves an example icon name associated with this theme.
     *
     * Used as a preview of the theme for icon theme selection panels.
     *
     * @return A constant reference to the example icon name.
     */
    const std::string &example() const noexcept
    {
        return m_example;
    }

    /**
     * @brief Checks whether the theme is hidden.
     *
     * Hidden themes should not be displayed as options in icon theme selection panels.
     *
     * @return `true` if the theme is hidden, otherwise `false`.
     */
    bool hidden() const noexcept
    {
        return m_hidden;
    }

    /**
     * @brief Retrieves the path to the first `index.theme` file found in the theme's directories.
     *
     * @return A constant reference to the path of the `index.theme` file.
     */
    const std::filesystem::path &indexFilePath() const noexcept
    {
        return m_indexFilePath;
    }

    /**
     * @brief Retrieves all directories where this theme was found.
     *
     * @return A constant reference to a vector of directories associated with this theme.
     */
    const std::vector<std::filesystem::path> &dirs() const noexcept
    {
        return m_dirs;
    }

    /**
     * @brief Retrieves the parsed `index.theme` data.
     *
     * @return A constant reference to the parsed data stored in an XDGINI object.
     */
    const XDGINI &indexData() const noexcept
    {
        return m_indexData;
    }

    /**
     * @brief Clears the contents of indexData().
     *
     * This method releases the information of the loaded `index.theme` when it is no longer needed.
     * This doesn't affect properties such as name(), displayName(), comment(), ext.
     *
     * @note Calling this method on an uninitialized theme has no effect.
     */
    void freeIndexData() noexcept
    {
        if (m_initialized)
            m_indexData.clear();
    }

    /**
     * @brief Retrieves the directories containing normal icons.
     *
     * @return A constant reference to a vector of icon directories.
     */
    const std::vector<std::shared_ptr<XDGIconDirectory>> &iconDirectories() const noexcept
    {
        if (!m_initialized)
            initAllIconsDir();

        return m_iconDirectories;
    }

    /**
     * @brief Retrieves the directories containing scaled icons.
     *
     * @return A constant reference to a vector of scaled icon directories.
     */
    const std::vector<std::shared_ptr<XDGIconDirectory>> &scaledIconDirectories() const noexcept
    {
        if (!m_initialized)
            initAllIconsDir();

        return m_scaledIconDirectories;
    }

private:
    friend class XDGIconThemeManager;
    void initAllIconsDir() const noexcept;
    void initIconsDir(const std::vector<std::string> &iconDirs, XDGIconDirectory::Type type) const noexcept;
    mutable std::vector<std::shared_ptr<XDGIconDirectory>> m_iconDirectories;
    mutable std::vector<std::shared_ptr<XDGIconDirectory>> m_scaledIconDirectories;
    const std::string *m_name;
    std::string m_displayName;
    std::string m_comment;
    std::string m_example;
    mutable std::vector<std::string> m_inherits;
    mutable std::vector<std::filesystem::path> m_dirs;
    mutable std::filesystem::path m_indexFilePath;
    mutable XDGINI m_indexData;
    mutable std::vector<std::string> m_iconDirNames, m_scaledIconDirNames;
    XDGKit &m_kit;
    mutable bool m_initialized { false };
    bool m_hidden { false };
};

#endif // XDGICONTHEME_H
