#include <XDGKit/XDGUtils.h>
#include <XDGKit/XDGKit.h>
#include <XDGKit/XDGIconTheme.h>
#include <XDGKit/XDGIconDirectory.h>
#include <fcntl.h>
#include <iostream>
#include <sys/mman.h>

using namespace XDG;

XDGIconTheme::XDGIconTheme(XDGKit &kit) noexcept :
    m_kit(kit)
{
}

XDGIconTheme::~XDGIconTheme()
{
    if (m_cacheMap)
    {
        munmap(m_cacheMap, m_cacheMapSize);
        m_cacheMap = nullptr;
        m_cacheMapSize = 0;
    }

    if (m_cacheFd != -1)
    {
        close(m_cacheFd);
        m_cacheFd = -1;
    }
}

void XDGIconTheme::evictCache() noexcept
{
    if (usingCache())
        madvise(m_cacheMap, m_cacheMapSize, MADV_DONTNEED);
}

void XDGIconTheme::initAllIconsDir() const noexcept
{
    m_initialized = true;
    initIconsDir(m_iconDirNames, XDGIconDirectory::Type::Normal);
    initIconsDir(m_scaledIconDirNames, XDGIconDirectory::Type::Scaled);
    m_iconDirNames.clear();
    m_scaledIconDirNames.clear();
    m_iconDirNames.shrink_to_fit();
    m_scaledIconDirNames.shrink_to_fit();
    m_dirs.shrink_to_fit();
}

void XDGIconTheme::initIconsDir(const std::vector<std::string> &iconDirs, XDGIconDirectory::Type type) const noexcept
{
    if (usingCache())
        return;

    auto *iconsDirVec { type == XDGIconDirectory::Type::Scaled ? &m_scaledIconDirectories : &m_iconDirectories };
    XDGIconDirectory IcD { (*((XDGIconTheme*)this)) };
    IcD.m_ramCache.reset(new XDGIconDirectory::Cache());
    IcD.m_cachePtr = IcD.m_ramCache.get();
    IcD.m_cachePtr->type = type;

    for (const auto &iconDir : iconDirs)
    {
        const auto &iconDirData = indexData().find(iconDir);
        if (iconDirData == indexData().end()) continue;

        const auto &size { iconDirData->second.find("Size") };
        if (size == iconDirData->second.end()) continue;

        // Size
        try { IcD.m_cachePtr->size = std::stoi(size->second.data()); }
        catch (const std::exception &) { IcD.m_cachePtr->size = -1; }
        if (IcD.m_cachePtr->size < 0) continue;

        // Scale
        IcD.m_cachePtr->scale = 1;
        const auto &scale { iconDirData->second.find("Scale") };
        if (scale != iconDirData->second.end())
        {
            try { IcD.m_cachePtr->scale = std::stoi(scale->second.data()); }
            catch (const std::exception &) { IcD.m_cachePtr->scale = -1; }
            if (IcD.m_cachePtr->scale <= 0) IcD.m_cachePtr->scale = 1;
        }

        // Context
        IcD.m_cachePtr->context = XDGIconDirectory::Context::NoContext;
        const auto &context { iconDirData->second.find("Context") };
        if (context != iconDirData->second.end())
        {
            if (context->second == "Actions")
                IcD.m_cachePtr->context = XDGIconDirectory::Context::Actions;
            else if (context->second == "Devices")
                IcD.m_cachePtr->context = XDGIconDirectory::Context::Devices;
            else if (context->second == "FileSystems")
                IcD.m_cachePtr->context = XDGIconDirectory::Context::FileSystems;
            else if (context->second == "MimeTypes")
                IcD.m_cachePtr->context = XDGIconDirectory::Context::MimeTypes;
        }

        // Size Type
        IcD.m_cachePtr->sizeType = XDGIconDirectory::SizeType::Threshold;
        const auto &sizeType { iconDirData->second.find("Type") };
        if (sizeType != iconDirData->second.end())
        {
            if (sizeType->second == "Fixed")
                IcD.m_cachePtr->sizeType = XDGIconDirectory::SizeType::Fixed;
            else if (sizeType->second == "Scalable")
                IcD.m_cachePtr->sizeType = XDGIconDirectory::SizeType::Scalable;
        }

        // Max Size
        IcD.m_cachePtr->maxSize = IcD.m_cachePtr->size;
        const auto &maxSize { iconDirData->second.find("MaxSize") };
        if (maxSize != iconDirData->second.end())
        {
            try { IcD.m_cachePtr->maxSize = std::stoi(maxSize->second.data()); }
            catch (const std::exception &) { IcD.m_cachePtr->maxSize = -1; }
            if (IcD.m_cachePtr->maxSize <= 0 || IcD.m_cachePtr->maxSize < IcD.m_cachePtr->size) IcD.m_cachePtr->maxSize = IcD.m_cachePtr->size;
        }

        // Min Size
        IcD.m_cachePtr->minSize = IcD.m_cachePtr->size;
        const auto &minSize { iconDirData->second.find("MinSize") };
        if (minSize != iconDirData->second.end())
        {
            try { IcD.m_cachePtr->minSize = std::stoi(minSize->second.data()); }
            catch (const std::exception &) { IcD.m_cachePtr->minSize = -1; }
            if (IcD.m_cachePtr->minSize < 0 || IcD.m_cachePtr->minSize > IcD.m_cachePtr->size) IcD.m_cachePtr->minSize = IcD.m_cachePtr->size;
        }

        // Threshold
        IcD.m_cachePtr->threshold = 2;
        const auto &threshold { iconDirData->second.find("Threshold") };
        if (threshold != iconDirData->second.end())
        {
            try { IcD.m_cachePtr->threshold = std::stoi(threshold->second.data()); }
            catch (const std::exception &) { IcD.m_cachePtr->threshold = -1; }
            if (IcD.m_cachePtr->threshold < 0) IcD.m_cachePtr->threshold = 2;
        }

        for (const auto &themeDir : dirs())
        {
            if (std::filesystem::is_directory(themeDir / iconDir))
            {
                auto &newIconDir = iconsDirVec->emplace_back(IcD);
                newIconDir.m_themeDir = kit().saveOrGetString(themeDir.string());
                newIconDir.m_dirName = kit().saveOrGetString(iconDir);
                newIconDir.initIcons();
            }
        }
    }
}

void XDGIconTheme::loadCache() noexcept
{
    if (!kit().options().useIconThemesCache)
        return;

    std::filesystem::path cacheFilePath { std::filesystem::path("/var/cache/xdgkit/icon_themes/users") / kit().username() / name() };
    off_t off;
    uint64_t u64, numDirs, numIcons;
    uint32_t u32;
    char *pos, *end, *str, *themeDir, *dirName, *iconName;
    const char *error {"Unknown error."};
    bool boolean;
    std::list<XDGIconDirectory> *dirList;
    XDGIconDirectory::Cache *cache;
    bool inHome { true };
retry:
    m_usingCache = true;
    m_initialized = true;
    m_cacheFd = open(cacheFilePath.c_str(), O_RDONLY);

    if (m_cacheFd == -1)
    {
        if (inHome)
        {
            inHome = false;
            cacheFilePath = std::filesystem::path("/var/cache/xdgkit/icon_themes/system") / name();
            goto retry;
        }

        error = "Failed to open cache file.";
        goto fail;
    }

    // Get file size
    off = lseek(m_cacheFd, 0, SEEK_END);
    if (off == -1)
    {
        error = "Failed to get cache file size.";
        goto failFd;
    }
    m_cacheMapSize = off;

    // Map file
    m_cacheMap = mmap(nullptr, m_cacheMapSize, PROT_READ, MAP_SHARED, m_cacheFd, 0);
    if (m_cacheMap == MAP_FAILED)
    {
        error = "Failed to map cache file.";
        m_cacheMap = nullptr;
        goto failFd;
    }

    pos = (char*)m_cacheMap;
    end = pos + m_cacheMapSize;

    // Validate the name match
    str = pos;
    if (!(pos = XDGUtils::advanceStrPosSafe(pos, end)))
    {
        error = "Failed to get theme name.";
        goto failParse;
    }
    if (name() != std::string_view(str))
    {
        error = "Cache file is corrupted.";
        goto failParse;
    }

    // Index size
    if (!(pos = XDGUtils::readSafeAndAdvancePos(&u64, pos, end, sizeof(u64))))
    {
        error = "Failed to get the theme index size.";
        goto failParse;
    }

    if ((int64_t)end - (int64_t)pos < (int64_t)u64)
    {
        error = "The theme index size is greater than the file size.";
        goto failParse;
    }

    // Parse index
    m_indexData = std::move(*XDGINIView::FromData(pos, u64).get());
    if (m_indexData.empty())
    {
        error = "The index map is empty.";
        goto failParse;
    }

    {
        const auto &mainSection { m_indexData.find("Icon Theme") };

        if (mainSection == m_indexData.end())
        {
            error = "Missing required section Icon Theme.";
            goto failParse;
        }

        if (!mainSection->second.contains("Name"))
        {
            error = "Missing required field Name.";
            goto failParse;
        }

        // Required fields
        if (!mainSection->second.contains("Name"))
        {
            error = "Missing required field Name.";
            goto failParse;
        }

        if (!mainSection->second.contains("Comment"))
        {
            error = "Missing required field Comment.";
            goto failParse;
        }

        if (!mainSection->second.contains("Directories"))
        {
            error = "Missing required field Directories.";
            goto failParse;
        }
    }

    pos += u64;

    // Num of dirs
    if (!(pos = XDGUtils::readSafeAndAdvancePos(&numDirs, pos, end, sizeof(numDirs))))
    {
        error = "Failed to get the number of direcories.";
        goto failParse;
    }

    for (uint64_t i = 0; i < numDirs; i++)
    {
        // Is scaled
        if (!(pos = XDGUtils::readSafeAndAdvancePos(&boolean, pos, end, sizeof(boolean))))
        {
            error = "Failed to get directory type.";
            goto failParse;
        }

        // Dir cache
        cache = (XDGIconDirectory::Cache*)pos;
        if (!(pos = XDGUtils::advanceSafe(pos, end, sizeof(*cache))))
        {
            error = "Failed to get directory data.";
            goto failParse;
        }

        // Theme dir
        themeDir = pos;
        if (!(pos = XDGUtils::advanceStrPosSafe(pos, end)))
        {
            error = "Failed to get theme dir.";
            goto failParse;
        }

        // Dir name
        dirName = pos;
        if (!(pos = XDGUtils::advanceStrPosSafe(pos, end)))
        {
            error = "Failed to get dir name";
            goto failParse;
        }

        // Store pointers
        if (boolean)
            dirList = &m_iconDirectories;
        else
            dirList = &m_scaledIconDirectories;

        auto &dir { dirList->emplace_back(*this) };
        dir.m_cachePtr = cache;
        dir.m_dirName = dirName;
        dir.m_themeDir = themeDir;

        // Icons num
        if (!(pos = XDGUtils::readSafeAndAdvancePos(&numIcons, pos, end, sizeof(numIcons))))
        {
            error = "Failed to get numer of icons.";
            goto failParse;
        }

        for (uint64_t j = 0; j < numIcons; j++)
        {
            // Icon name
            iconName = pos;
            if (!(pos = XDGUtils::advanceStrPosSafe(pos, end)))
            {
                error = "Failed to get icon name.";
                goto failParse;
            }

            // Extensions
            if (!(pos = XDGUtils::readSafeAndAdvancePos(&u32, pos, end, sizeof(u32))))
            {
                error = "Failed to get icon extensions.";
                goto failParse;
            }

            // Assign pointers
            const auto &icon { dir.m_icons.emplace(iconName, dir) };
            icon.first->second.m_name = iconName;
            icon.first->second.m_extensions = u32;
        }
    }

    return;
failParse:
    m_iconDirectories.clear();
    m_indexData = {};
    munmap(m_cacheMap, m_cacheMapSize);
    m_cacheMap = nullptr;
    m_cacheMapSize = 0;
failFd:
    close(m_cacheFd);
    m_cacheFd = -1;
fail:
    m_initialized = false;
    m_usingCache = false;
    if (name() != "default")
        std::cerr << "[XDGKit] Failed to load cache for icon theme " << cacheFilePath << ": " << error << "\n";
}
