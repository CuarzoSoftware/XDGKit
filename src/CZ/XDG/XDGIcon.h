#ifndef XDGICON_H
#define XDGICON_H

#include <CZ/XDG/XDG.h>
#include <filesystem>

/**
 * @brief Properties of an icon.
 *
 * This class provides information about an icon, including its name, file extensions,
 * paths, and the directory to which it belongs.
 *
 * @note Most of the icon properties can be found in its respective directory()
 */
class CZ::XDGIcon
{
public:

    /**
     * @brief Enumeration of supported file extensions for icons.
     */
    enum Extension : uint32_t
    {
        PNG = static_cast<uint32_t>(1) << 0, /**< PNG file format. */
        SVG = static_cast<uint32_t>(1) << 1, /**< SVG file format. */
        XPM = static_cast<uint32_t>(1) << 2  /**< XPM file format. */
    };

    XDGIcon(XDGIconDirectory &directory) noexcept : m_directory(directory) {}

    /**
     * @brief Handle to the parent kit.
     */
    XDGKit &kit() const noexcept;

    /**
     * @brief Retrieves the bitset of file extensions found for this icon.
     *
     * Each bit in the bitset corresponds to a supported file extension.
     *
     * @return A bitset of file extensions (e.g., PNG, SVG, XPM).
     */
    uint32_t extensions() const noexcept { return m_extensions; };

    /**
     * @brief Retrieves the name of the icon.
     *
     * @return A constant reference to the icon's name.
     */
    const std::string_view &name() const noexcept { return m_name; };

    /**
     * @brief Retrieves the absolute path of the icon for a specified file extension.
     *
     * The provided extension must exist in the set of extensions returned by `extensions()`.
     *
     * @param ext The file extension for which to retrieve the icon's path.
     * @return The absolute path of the icon file with the specified extension.
     */
    std::filesystem::path getPath(Extension ext) const noexcept;

    /**
     * @brief Retrieves the directory to which this icon belongs.
     *
     * The directory contains information such as size, scale, and context of the icon.
     *
     * @return A reference to the XDGIconDirectory object representing the directory.
     */
    XDGIconDirectory &directory() const noexcept { return m_directory; };

    /**
     * @brief Indicates whether the parent theme was loaded from cache.
     */
    bool usingCache() const noexcept;

private:
    friend class XDGIconDirectory;
    friend class XDGIconTheme;
    uint32_t m_extensions { 0 };
    std::string_view m_name;
    XDGIconDirectory &m_directory;
};

#endif // XDGICON_H
