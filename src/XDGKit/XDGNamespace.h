#ifndef XDGNAMESPACE_H
#define XDGNAMESPACE_H

namespace XDG
{
    class XDGKit;
    class XDGIconThemeManager;
    class XDGIconTheme;
    class XDGIconDirectory;

    enum class IconSizeType
    {
        Fixed,
        Scalable,
        Threshold
    };

    enum class DirectoryType
    {
        Normal,
        Scaled
    };
};

#endif // XDGNAMESPACE_H
