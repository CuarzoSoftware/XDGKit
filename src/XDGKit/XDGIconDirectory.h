#ifndef XDGICONDIRECTORY_H
#define XDGICONDIRECTORY_H

#include <XDGKit/XDGIcon.h>
#include <XDGKit/XDGMap.h>
#include <cstdint>
#include <filesystem>

/**
 * @brief Group of icons with shared properties.
 *
 * This class encapsulates properties of an icon directory, including its size type,
 * scaling information, context, and the icons it contains.
 *
 * @note All icons within the directory share these properties.
 */
class XDG::XDGIconDirectory
{
public:

    /**
     * @brief Represents the type of the directory.
     */
    enum Type : uint32_t
    {
        Normal = static_cast<uint32_t>(1) << 0, /**< A normal icon directory. */
        Scaled = static_cast<uint32_t>(1) << 1  /**< A scaled icon directory. */
    };

    /**
     * @brief Describes the algorithm used to determine if an icon matches a given size.
     */
    enum SizeType : uint32_t
    {
        Fixed     = static_cast<uint32_t>(1) << 0, /**< Fixed size icons. */
        Scalable  = static_cast<uint32_t>(1) << 1, /**< Scalable icons. */
        Threshold = static_cast<uint32_t>(1) << 2  /**< Icons with a size threshold. */
    };

    /**
     * @brief Defines the context to which icons belong.
     */
    enum Context : uint32_t
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
    int32_t size() const noexcept { return m_cachePtr->size; };

    /**
     * @brief Retrieves the maximum nominal size of scalable icons.
     *
     * @return The maximum nominal size.
     */
    int32_t maxSize() const noexcept { return m_cachePtr->maxSize; };

    /**
     * @brief Retrieves the minimum nominal size of scalable icons.
     *
     * @return The minimum nominal size.
     */
    int32_t minSize() const noexcept { return m_cachePtr->minSize; };

    /**
     * @brief Retrieves the scale factor of the icons.
     *
     * @return The scale factor.
     */
    int32_t scale() const noexcept { return m_cachePtr->scale; };

    /**
     * @brief Retrieves the nominal size threshold for icons in the directory.
     *
     * @return The size threshold.
     */
    int32_t threshold() const noexcept { return m_cachePtr->threshold; };

    /**
     * @brief Retrieves the type of the directory.
     *
     * @return The directory type (e.g., Normal or Scaled).
     */
    Type type() const noexcept { return m_cachePtr->type; };

    /**
     * @brief Retrieves the size type of the icons.
     *
     * @return The size type (e.g., Fixed, Scalable, or Threshold).
     */
    SizeType sizeType() const noexcept { return m_cachePtr->sizeType; };

    /**
     * @brief Retrieves the context associated with the icons in the directory.
     *
     * @return The context (e.g., Actions, Devices).
     */
    Context context() const noexcept { return m_cachePtr->context; };

    /**
     * @brief Retrieves the directory name.
     */
    const std::string_view &dirName() const noexcept { return m_dirName; };

    /**
     * @brief Retrieves the absolute path to the icon directory.
     */
    std::filesystem::path dir() const noexcept;

    /**
     * @brief Retrieves the icons located in the directory.
     *
     * @return A constant reference to a map of icon names to their corresponding icon objects.
     */
    const XDGMap<std::string_view, XDGIcon> &icons() const noexcept { return m_icons; }

    /**
     * @brief Retrieves the theme to which this directory belongs.
     *
     * @return A reference to the associated XDGIconTheme object.
     */
    XDGIconTheme &theme() const noexcept { return m_theme; };

    /**
     * @brief Indicates whether the parent theme was loaded from cache.
     */
    bool usingCache() const noexcept;

#pragma pack(push, 1)
    struct Cache
    {
        int32_t size;
        int32_t maxSize;
        int32_t minSize;
        int32_t scale;
        int32_t threshold;
        Type type;
        SizeType sizeType;
        Context context { NoContext };
    };
#pragma pack(pop)

    const Cache *data() const noexcept { return m_cachePtr; };
    const std::string_view &themeDir() const noexcept { return m_themeDir; };
private:
    friend class XDGIconThemeManager;
    friend class XDGIconTheme;
    void initIcons() noexcept;
    XDGMap<std::string_view, XDGIcon> m_icons;
    std::string_view m_themeDir;
    std::string_view m_dirName;

    // Equal to m_notCache or to the mapped cache
    Cache *m_cachePtr;
    std::shared_ptr<Cache> m_ramCache;
    XDGIconTheme &m_theme;
};

#endif // XDGICONDIRECTORY_H
