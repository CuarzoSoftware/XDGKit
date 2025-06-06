#include <XDGKit/XDGIcon.h>
#include <XDGKit/XDGIconDirectory.h>

using namespace XDG;

std::filesystem::path XDGIcon::getPath(Extension ext) const noexcept
{
    std::filesystem::path path = m_directory.dir() / m_name;
    switch (ext)
    {
    case PNG:
        path += ".png";
        break;
    case SVG:
        path += ".svg";
        break;
    case XPM:
        path += ".xpm";
        break;
    }

    return path;
}

bool XDGIcon::usingCache() const noexcept
{
    return directory().usingCache();
}

XDGKit &XDGIcon::kit() const noexcept
{
    return m_directory.kit();
}
