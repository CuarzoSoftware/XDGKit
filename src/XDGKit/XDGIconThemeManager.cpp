#include <XDGKit/XDGIconThemeManager.h>
#include <XDGKit/XDGKit.h>
#include <XDGKit/XDGUtils.h>

using namespace XDG;

void XDGIconThemeManager::restoreDefaultSearchDirs() noexcept
{
    m_searchDirs = std::vector<std::filesystem::path>();
    m_searchDirs.reserve(32);
    std::filesystem::path path = m_kit.homeDir() / ".icons";

    if (std::filesystem::is_directory(path))
        m_searchDirs.emplace_back(std::move(path));

    path = m_kit.homeDir() / ".local/share/icons";

    if (std::filesystem::is_directory(path))
        m_searchDirs.emplace_back(std::move(path));

    for (auto &dataDir : m_kit.dataDirs())
    {
        path = dataDir / "icons";

        if (std::filesystem::is_directory(path))
            m_searchDirs.emplace_back(std::move(path));
    }

    path = " /usr/share/pixmaps";

    if (std::filesystem::is_directory(path))
        m_searchDirs.emplace_back(std::move(path));
}

void XDGIconThemeManager::findThemes() noexcept
{
    m_themes.clear();

    try
    {
        // Create themes with only name, dirs (all dirs where the theme is found) and index.theme (the first found)
        for (auto &searchDir : searchDirs())
        {
            if (!std::filesystem::is_directory(searchDir))
                continue;

            for (const auto &themeDir : std::filesystem::directory_iterator(searchDir))
            {
                if (!themeDir.is_directory())
                    continue;

                auto foundTheme = m_themes.find(themeDir.path().filename());

                if (foundTheme == m_themes.end())
                {
                    auto [it, inserted] = m_themes.emplace(themeDir.path().filename(), std::shared_ptr<XDGIconTheme>(new XDGIconTheme(m_kit)));
                    it->second->m_name = &it->first;
                    it->second->m_dirs.reserve(16);
                    it->second->m_dirs.emplace_back(themeDir.path());

                    std::filesystem::path indexPath { themeDir.path() / "index.theme" };

                    if (std::filesystem::exists(indexPath) && std::filesystem::is_regular_file(indexPath))
                        it->second->m_indexFilePath = indexPath;
                }
                else
                {
                    foundTheme->second->m_dirs.emplace_back(themeDir.path());

                    if (foundTheme->second->m_indexFilePath.empty())
                    {
                        std::filesystem::path indexPath { themeDir.path() / "index.theme" };

                        if (std::filesystem::exists(indexPath) && std::filesystem::is_regular_file(indexPath))
                            foundTheme->second->m_indexFilePath = indexPath;
                    }
                }
            }
        }

        // Filter themes without index.theme
        for (auto it = m_themes.begin(); it != m_themes.end();)
        {
            if (it->second->m_indexFilePath.empty())
                it = m_themes.erase(it);
            else
            {
                it->second->loadCache();

                if (!it->second->m_usingCache)
                    it->second->m_indexData = std::move(*XDGINIView::LoadFile(it->second->m_indexFilePath).get());

                const auto &mainSection { it->second->m_indexData.find("Icon Theme") };

                if (mainSection == it->second->m_indexData.end())
                {
                    it = m_themes.erase(it);
                    continue;
                }

                // Required fields
                const auto &name { mainSection->second.find("Name") };
                const auto &comment { mainSection->second.find("Comment") };
                const auto &directories { mainSection->second.find("Directories") };

                if (name == mainSection->second.end() || comment == mainSection->second.end() || directories == mainSection->second.end())
                {
                    it = m_themes.erase(it);
                    continue;
                }

                it->second->m_displayName = name->second;
                it->second->m_comment = comment->second;

                it->second->m_iconDirNames = XDGUtils::splitString(directories->second, ',', true);

                const auto &scaledDirectories { mainSection->second.find("ScaledDirectories") };
                const auto &hidden { mainSection->second.find("Hidden") };
                const auto &example { mainSection->second.find("Example") };

                if (scaledDirectories != mainSection->second.end())
                    it->second->m_scaledIconDirNames = XDGUtils::splitString(scaledDirectories->second, ',', true);

                // Inherits is done later

                if (hidden != mainSection->second.end())
                    it->second->m_hidden = hidden->second == "true";
                if (example != mainSection->second.end())
                    it->second->m_example = example->second;

                ++it;
            }
        }

    }
    catch (const std::exception &){}

    sanitizeThemes();
}

void XDGIconThemeManager::sanitizeThemes() noexcept
{
    const auto &hicolor = m_themes.find("hicolor");

    for (auto it = m_themes.begin(); it != m_themes.end(); it++)
    {
        if (hicolor == it)
            continue;

        // Already verified that exists
        const auto &mainSection { it->second->m_indexData.find("Icon Theme") };
        const auto &inherits { mainSection->second.find("Inherits") };

        if (inherits != mainSection->second.end())
        {
            it->second->m_inherits = XDGUtils::splitString(inherits->second, ',', true);
            if (hicolor != m_themes.end())
                it->second->m_inherits.emplace_back("hicolor");
            XDGUtils::removeDuplicates(it->second->m_inherits);

            // Remove self and non existent inherits
            for (auto inh = it->second->m_inherits.begin(); inh != it->second->m_inherits.end();)
            {
                if (*inh == it->first || m_themes.find(*inh) == m_themes.end())
                    inh = it->second->m_inherits.erase(inh);
                else
                    inh++;
            }
        }
        else if (hicolor != m_themes.end())
            it->second->m_inherits.emplace_back("hicolor");
    }
}

const XDGIcon *XDGIconThemeManager::findIconHelper(Search &search, std::shared_ptr<XDGIconTheme> theme) const noexcept
{
    if (search.visitedThemes.contains(theme))
        return nullptr;

    search.visitedThemes.emplace(theme);

    int32_t distance;

    for (const auto &dir : theme->scaledIconDirectories())
    {
        if ((dir.context() & search.contexts) == 0)
            continue;

        const auto &icon = dir.icons().find(search.icon);

        if (icon == dir.icons().end() || (icon->second.extensions() & search.extensions) == 0)
            continue;

        if ((icon->second.extensions() & search.extensions & XDGIcon::SVG) != 0)
            return &icon->second;

        distance = directorySizeDistance(search, dir);

        if (distance < search.bestDistance)
        {
            search.bestDistance = distance;
            search.bestIcon = &icon->second;
        }

        if (!directoryMatchesSize(search, dir))
            continue;

        return &icon->second;
    }

    for (const auto &dir : theme->iconDirectories())
    {
        if ((dir.context() & search.contexts) == 0)
            continue;

        const auto &icon = dir.icons().find(search.icon);

        if (icon == dir.icons().end() || (icon->second.extensions() & search.extensions) == 0)
            continue;

        if ((icon->second.extensions() & search.extensions & XDGIcon::SVG) != 0)
            return &icon->second;

        distance = directorySizeDistance(search, dir);

        if (distance < search.bestDistance)
        {
            search.bestDistance = distance;
            search.bestIcon = &icon->second;
        }

        if (!directoryMatchesSize(search, dir))
            continue;

        return &icon->second;
    }


    const XDGIcon *found { nullptr };
    for (const auto &parentIt : theme->inherits())
    {
        const auto &parent = m_themes.find(parentIt);
        found = findIconHelper(search, parent->second);
        if (found) return found;
    }

    return nullptr;
}

bool XDGIconThemeManager::directoryMatchesSize(Search &search, const XDGIconDirectory &dir) const noexcept
{
    if (search.scale != dir.scale())
        return false;

    if (dir.sizeType() == XDGIconDirectory::SizeType::Fixed)
        return search.size == dir.size();
    else if (dir.sizeType() == XDGIconDirectory::SizeType::Scalable)
        return dir.minSize() <= search.size &&  search.size <= dir.maxSize();
    else if (dir.sizeType() == XDGIconDirectory::SizeType::Threshold)
        return dir.size() - dir.threshold() <= search.size && search.size <= dir.size() + dir.threshold();

    return false;
}

int32_t XDGIconThemeManager::directorySizeDistance(Search &search, const XDGIconDirectory &dir) const noexcept
{
    if (dir.sizeType() == XDGIconDirectory::SizeType::Fixed)
        return std::abs(search.bufferSize - dir.size() * dir.scale());
    else if (dir.sizeType() == XDGIconDirectory::SizeType::Scalable)
    {
        if (search.bufferSize < dir.minSize() * dir.scale())
            return dir.minSize() * dir.scale() - search.bufferSize;
        else if (search.bufferSize > dir.maxSize() * dir.scale())
            return search.bufferSize - dir.maxSize() * dir.scale();
    }
    else if (dir.sizeType() == XDGIconDirectory::SizeType::Threshold)
    {
        if (search.bufferSize < (dir.size() - dir.threshold()) * dir.scale())
            return (dir.size() - dir.threshold()) * dir.scale() - search.bufferSize;
        else if (search.bufferSize > (dir.maxSize() + dir.threshold()) * dir.scale())
            return search.bufferSize - (dir.maxSize() + dir.threshold()) * dir.scale();
    }

    return std::numeric_limits<int32_t>::max() - 1;
}

void XDGIconThemeManager::reloadThemes() noexcept
{
    m_themes.clear();
    m_searchDirs.clear();
    kit().m_stringPool.clear();

    kit().rescanDataDirs();
    restoreDefaultSearchDirs();
    findThemes();
}

const XDGIcon *XDGIconThemeManager::findIcon(const std::string &icon, int32_t size, int32_t scale, uint32_t extensions, const std::vector<std::string> &themes, uint32_t contexts) const noexcept
{
    Search search
    {
        .icon = icon,
        .size = size,
        .scale = scale,
        .bufferSize = size * scale,
        .extensions = extensions,
        .contexts = contexts,
        .themes = {},
        .visitedThemes = {}
    };

    if ((extensions & (1 | 2 | 4)) == 0 || scale <= 0 || themes.empty() || (contexts & XDGIconDirectory::AnyContext) == 0)
        return nullptr;

    search.themes.reserve(m_themes.size());

    for (auto &theme : themes)
    {
        if (theme == "")
        {
            for (auto &T : m_themes)
                search.themes.emplace_back(T.second);

            continue;
        }

        const auto &it = m_themes.find(theme);

        if (it == m_themes.end())
            continue;

        search.themes.emplace_back(it->second);
    }

    XDGUtils::removeDuplicates(search.themes);

    if (search.themes.empty())
        return nullptr;

    search.visitedThemes.reserve(m_themes.size());

    const XDGIcon *found { nullptr };

    for (const auto &theme : search.themes)
    {
        found = findIconHelper(search, theme);
        if (found) return found;
    }

    if (!search.bestIcon && (search.contexts & XDGIconDirectory::AnyContext) == XDGIconDirectory::AnyContext)
        kit().m_stringPool.erase(icon);

    return search.bestIcon;
}

void XDGIconThemeManager::evictCache() noexcept
{
    for (const auto &theme : themes())
        theme.second->evictCache();
}
