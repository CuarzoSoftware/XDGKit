#ifndef XDGICONDIRECTORY_H
#define XDGICONDIRECTORY_H

#include <XDGKit/XDGNamespace.h>
#include <string>

class XDG::XDGIconDirectory
{
public:
    XDGIconDirectory(XDGIconTheme &theme) noexcept;

private:
    friend class XDGIconThemeManager;
    int m_size;
    int m_maxSize;
    int m_minSize;
    int m_scale;
    int m_threshold;
    IconSizeType m_sizeType;
    std::string m_context;
    XDGIconTheme &m_theme;
};

#endif // XDGICONDIRECTORY_H
