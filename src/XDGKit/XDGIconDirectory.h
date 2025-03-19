#ifndef XDGICONDIRECTORY_H
#define XDGICONDIRECTORY_H

#include <XDGKit/XDGNamespace.h>
#include <cstdint>
#include <filesystem>

class XDG::XDGIconDirectory
{
public:

    enum Type
    {
        Normal = static_cast<uint32_t>(1) << 0,
        Scaled = static_cast<uint32_t>(1) << 1,
    };

    enum SizeType
    {
        Fixed     = static_cast<uint32_t>(1) << 0,
        Scalable  = static_cast<uint32_t>(1) << 1,
        Threshold = static_cast<uint32_t>(1) << 2,
    };

    enum Context
    {
        NoContext     = 0,
        Actions       = static_cast<uint32_t>(1) << 0,
        Devices       = static_cast<uint32_t>(1) << 1,
        FileSystems   = static_cast<uint32_t>(1) << 2,
        MimeTypes     = static_cast<uint32_t>(1) << 3
    };

    XDGIconDirectory(XDGIconTheme &theme) noexcept;

    int size() const noexcept { return m_size; };
    int maxSize() const noexcept { return m_maxSize; };
    int minSize() const noexcept { return m_minSize; };
    int scale() const noexcept { return m_scale; };
    int threshold() const noexcept { return m_threshold; };
    Type type() const noexcept { return m_type; };
    SizeType sizeType() const noexcept { return m_sizeType; };
    Context context() const noexcept { return m_context; };
    const std::filesystem::path &dir() const noexcept { return m_dir; };
    XDGIconTheme &theme() const noexcept { return m_theme; };
private:
    friend class XDGIconThemeManager;
    friend class XDGIconTheme;
    int m_size;
    int m_maxSize;
    int m_minSize;
    int m_scale;
    int m_threshold;
    Type m_type;
    SizeType m_sizeType;
    Context m_context;
    std::filesystem::path m_dir;
    XDGIconTheme &m_theme;
};

#endif // XDGICONDIRECTORY_H
