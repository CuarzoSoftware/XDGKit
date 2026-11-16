#include <CZ/XDG/XDGIconDirectory.h>
#include <CZ/XDG/XDGIcon.h>
#include <CZ/XDG/XDGKit.h>
#include <unordered_set>
#include <cstring>

using namespace CZ;

std::filesystem::path XDGIconDirectory::dir() const noexcept
{
    return std::filesystem::path(m_themeDir) / m_dirName;
}

bool XDGIconDirectory::usingCache() const noexcept
{
    // Dirs scanned at runtime (e.g. those a theme's cache doesn't cover) have their own RAM copy
    return !m_ramCache && theme().usingCache();
}

const XDGIcon *XDGIconDirectory::cachedIconAt(uint64_t index) const noexcept
{
    uint32_t offset;
    memcpy(&offset, m_cacheOffsets + index * sizeof(uint32_t), sizeof(offset));

    // Validated lazily: the name must be NUL terminated, followed by the extensions, in the block
    if (offset >= m_cacheRecordsSize)
        return nullptr;

    const char *name { m_cacheRecords + offset };
    const char *nul { static_cast<const char*>(memchr(name, '\0', m_cacheRecordsSize - offset)) };

    if (!nul || uint64_t(nul + 1 + sizeof(uint32_t) - m_cacheRecords) > m_cacheRecordsSize)
        return nullptr;

    const std::string_view key { name, size_t(nul - name) };

    if (auto it { m_icons.find(key) }; it != m_icons.end())
        return &it->second;

    auto &icon { m_icons.emplace(key, *const_cast<XDGIconDirectory*>(this)).first->second };
    icon.m_name = key;
    memcpy(&icon.m_extensions, nul + 1, sizeof(uint32_t));
    return &icon;
}

const XDGIcon *XDGIconDirectory::findIcon(std::string_view name) const noexcept
{
    if (auto it { m_icons.find(name) }; it != m_icons.end())
        return &it->second;

    if (m_allIcons)
        return nullptr;

    // Binary search in the mapped cache (records sorted by name, see the indexer)
    uint64_t lo { 0 }, hi { m_cacheIconCount };

    while (lo < hi)
    {
        const uint64_t mid { lo + (hi - lo) / 2 };
        uint32_t offset;
        memcpy(&offset, m_cacheOffsets + mid * sizeof(uint32_t), sizeof(offset));

        if (offset >= m_cacheRecordsSize)
            return nullptr;

        const char *candidate { m_cacheRecords + offset };
        const size_t len { strnlen(candidate, m_cacheRecordsSize - offset) };
        const int cmp { name.compare(std::string_view(candidate, len)) };

        if (cmp == 0)
            return cachedIconAt(mid);

        if (cmp < 0)
            hi = mid;
        else
            lo = mid + 1;
    }

    return nullptr;
}

const XDGMap<std::string_view, XDGIcon> &XDGIconDirectory::icons() const noexcept
{
    if (!m_allIcons)
    {
        for (uint64_t i = 0; i < m_cacheIconCount; i++)
            cachedIconAt(i);

        m_allIcons = true;
    }

    return m_icons;
}

XDGKit &XDGIconDirectory::kit() const noexcept
{
    return m_theme.kit();
}

void XDGIconDirectory::initIcons() noexcept
{
    if (usingCache())
        return;

    try
    {
        static const std::unordered_set<std::string> extensions( {".png", ".svg", ".xpm"} );

        for (const auto &entry : std::filesystem::directory_iterator(dir()))
        {
            if (!std::filesystem::is_regular_file(entry.path()) || !extensions.contains(entry.path().extension()))
                continue;

            std::string_view name { kit().saveOrGetString(entry.path().stem().string()) };

            auto it = m_icons.find(name);

            if (it == m_icons.end())
            {
                it = m_icons.emplace(name, *this).first;
                it->second.m_name = it->first;
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
