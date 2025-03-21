#include <XDGKit/XDGIconDirectory.h>
#include <XDGKit/XDGIcon.h>
#include <XDGKit/XDGKit.h>
#include <unordered_set>

using namespace XDG;

std::filesystem::path XDGIconDirectory::dir() const noexcept
{
    return std::filesystem::path(m_themeDir) / m_dirName;
}

XDGKit &XDGIconDirectory::kit() const noexcept
{
    return m_theme.kit();
}

void XDGIconDirectory::initIcons() noexcept
{
    try
    {
        static const std::unordered_set<std::string> extensions( {".png", ".svg", ".xpm"} );

        for (const auto &entry : std::filesystem::directory_iterator(dir()))
        {
            if (!std::filesystem::is_regular_file(entry.path()) || !extensions.contains(entry.path().extension()))
                continue;

            const char *name { kit().saveOrGetString(entry.path().stem().string()) };

            auto it = m_icons.find(name);

            if (it == m_icons.end())
            {
                it = m_icons.emplace(name, std::shared_ptr<XDGIcon>(new XDGIcon(*this))).first;
                it->second->m_name = it->first;
            }

            if (entry.path().extension() == ".png")
                it->second->m_extensions |= XDGIcon::PNG;
            else if (entry.path().extension() == ".svg")
                it->second->m_extensions |= XDGIcon::SVG;
            else
                it->second->m_extensions |= XDGIcon::XPM;

            // TODO: Load .icon info
        }
    }
    catch (const std::exception &) {}
}
