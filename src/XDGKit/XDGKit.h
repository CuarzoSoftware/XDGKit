#ifndef XDGKIT_H
#define XDGKIT_H

#include <XDGKit/XDGIconThemeManager.h>
#include <filesystem>
#include <memory>
#include <vector>

/**
 * @brief Core Class.
 *
 * When destroyed, all references to themes and icons are invalidated.
 */
class XDG::XDGKit
{
public:
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

private:
    XDGKit() noexcept;
    void initHomeDir() noexcept;
    void rescanDataDirs() noexcept;
    XDGIconThemeManager m_iconThemeManager;
    std::filesystem::path m_homeDir;
    std::vector<std::filesystem::path> m_dataDirs;
};

#endif // XDGKIT_H
