#include <XDGKit/XDGIconDirectory.h>
#include <XDGKit/XDGIcon.h>
#include <iostream>
#include <unordered_set>
#include <unordered_map>

using namespace XDG;

XDGIconDirectory::XDGIconDirectory(XDGIconTheme &theme) noexcept : m_theme(theme)
{
    m_icons.reserve(512);
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


            auto it = m_icons.find(entry.path().stem().string());

            if (it == m_icons.end())
            {
                it = m_icons.emplace(entry.path().stem().string(), *this).first;
                it->second.m_name = it->first;
                it->second.m_path = entry.path().parent_path() / entry.path().stem();
            }

            if (entry.path().extension() == ".png")
                it->second.m_extensions |= XDGIcon::PNG;
            else if (entry.path().extension() == ".svg")
                it->second.m_extensions |= XDGIcon::SVG;
            else
                it->second.m_extensions |= XDGIcon::XPM;

            // TODO: Load .icon info
        }
    }
    catch (const std::exception &) {}
}
