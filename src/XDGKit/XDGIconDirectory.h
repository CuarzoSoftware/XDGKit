#ifndef XDGICONDIRECTORY_H
#define XDGICONDIRECTORY_H

#include <XDGKit/XDGIcon.h>
#include <cstdint>
#include <filesystem>
#include <map>

/**
 * @brief Group of icons with shared properties.
 *
 * This class encapsulates properties of an icon directory, including its size type,
 * scaling information, context, and the icons it contains.
 */
class XDG::XDGIconDirectory
{
public:

    /**
     * @brief Represents the type of the directory.
     */
    enum Type
    {
        Normal = static_cast<uint32_t>(1) << 0, /**< A normal icon directory. */
        Scaled = static_cast<uint32_t>(1) << 1  /**< A scaled icon directory. */
    };

    /**
     * @brief Describes the algorithm used to determine if an icon matches a given size.
     */
    enum SizeType
    {
        Fixed     = static_cast<uint32_t>(1) << 0, /**< Fixed size icons. */
        Scalable  = static_cast<uint32_t>(1) << 1, /**< Scalable icons. */
        Threshold = static_cast<uint32_t>(1) << 2  /**< Icons with a size threshold. */
    };

    /**
     * @brief Defines the context to which icons belong.
     */
    enum Context
    {
        NoContext     = static_cast<uint32_t>(1) << 0, /**< No specific context. */
        Actions       = static_cast<uint32_t>(1) << 1, /**< Icons related to actions. */
        Devices       = static_cast<uint32_t>(1) << 2, /**< Icons related to devices. */
        FileSystems   = static_cast<uint32_t>(1) << 3, /**< Icons related to file systems. */
        MimeTypes     = static_cast<uint32_t>(1) << 4,  /**< Icons related to MIME types. */
        AnyContext    = NoContext | Actions | Devices | FileSystems | MimeTypes  /**< All flags */
    };

    XDGIconDirectory(XDGIconTheme &theme) noexcept : m_theme(theme) {}

    /**
     * @brief Handle to the parent kit.
     */
    XDGKit &kit() const noexcept;

    /**
     * @brief Retrieves the nominal (unscaled) size of icons in the directory.
     *
     * @return The nominal size.
     */
    int32_t size() const noexcept { return m_size; };

    /**
     * @brief Retrieves the maximum nominal size of scalable icons.
     *
     * @return The maximum nominal size.
     */
    int32_t maxSize() const noexcept { return m_maxSize; };

    /**
     * @brief Retrieves the minimum nominal size of scalable icons.
     *
     * @return The minimum nominal size.
     */
    int32_t minSize() const noexcept { return m_minSize; };

    /**
     * @brief Retrieves the scale factor of the icons.
     *
     * @return The scale factor.
     */
    int32_t scale() const noexcept { return m_scale; };

    /**
     * @brief Retrieves the nominal size threshold for icons in the directory.
     *
     * @return The size threshold.
     */
    int32_t threshold() const noexcept { return m_threshold; };

    /**
     * @brief Retrieves the type of the directory.
     *
     * @return The directory type (e.g., Normal or Scaled).
     */
    Type type() const noexcept { return m_type; };

    /**
     * @brief Retrieves the size type of the icons.
     *
     * @return The size type (e.g., Fixed, Scalable, or Threshold).
     */
    SizeType sizeType() const noexcept { return m_sizeType; };

    /**
     * @brief Retrieves the context associated with the icons in the directory.
     *
     * @return The context (e.g., Actions, Devices).
     */
    Context context() const noexcept { return m_context; };

    /**
     * @brief Retrieves the directory name.
     */
    const char *dirName() const noexcept { return m_dirName; };

    /**
     * @brief Retrieves the absolute path to the icon directory.
     */
    std::filesystem::path dir() const noexcept;

    /**
     * @brief Retrieves the icons located in the directory.
     *
     * @note Use XDKit::getString() to find the string pointer.
     *
     * @return A constant reference to a map of icon names to their corresponding icon objects.
     */
    const std::map<const char *, std::shared_ptr<XDGIcon>> &icons() const noexcept { return m_icons; }

    /**
     * @brief Retrieves the theme to which this directory belongs.
     *
     * @return A reference to the associated XDGIconTheme object.
     */
    XDGIconTheme &theme() const noexcept { return m_theme; };

private:
    friend class XDGIconThemeManager;
    friend class XDGIconTheme;
    void initIcons() noexcept;
    const char *m_themeDir { nullptr };
    const char *m_dirName { nullptr };
    int32_t m_size;
    int32_t m_maxSize;
    int32_t m_minSize;
    int32_t m_scale;
    int32_t m_threshold;
    Type m_type;
    SizeType m_sizeType;
    Context m_context { NoContext };
    std::map<const char *, std::shared_ptr<XDGIcon>> m_icons;
    XDGIconTheme &m_theme;
};

#endif // XDGICONDIRECTORY_H
