#include <CZ/XDG/XDGKit.h>
#include <CZ/XDG/XDGIconTheme.h>
#include <CZ/XDG/XDGIconDirectory.h>
#include <list>
#include <vector>
#include <string>
#include <cassert>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <fcntl.h>
#include <sys/stat.h>
#include <pwd.h>

/* THEME CACHE FORMAT (v2, see XDGIconTheme::CacheMagicV2)

u8[8]: "\0XDGKIT2"
str: theme name
u64: serialized index.theme size
str: serialized index.theme data
u64: num directories
FOREACH DIR:
    bol: is scaled dir

    i32: size
    i32: max size
    i32: min size
    i32: scale
    i32: threshold
    u32: type
    u32: size type
    u32: context

    str: theme dir
    str: dir name
    u64: icons num
    u64: records size (bytes)
    u64: offset of the dir's block in the data section
DATA SECTION (after the last dir), FOREACH DIR (block):
    u32[icons num]: offset of each record in the records, in name order (strcmp)
    FOREACH ICON (record):
        str: icon name
        u32: extensions

v1 (no magic): the icons follow "icons num" directly (str name, u32 extensions), unsorted.
*/

/* CACHE DIRS

For system themes : /var/cache/xdgkit/icon_themes/system
For user themes   : /var/cache/xdgkit/icon_themes/users/<username>
*/

using namespace CZ;

static const std::filesystem::path cacheDir { "/var/cache/xdgkit/icon_themes" };
static const std::filesystem::path systemCacheDir { cacheDir / "system" };
static const std::filesystem::path usersCacheDir { cacheDir / "users" };

static void saveCache(const std::string &username)
{
    std::filesystem::path currentCacheDir;
    const bool isSystem { username == "" };

    if (isSystem)
    {
        std::cout << "Generating system cache:\n";
        currentCacheDir = systemCacheDir;
    }
    else
    {
        std::cout << "Generating cache for user " << username << ":\n";
        currentCacheDir = usersCacheDir / username;

        if (!std::filesystem::create_directories(currentCacheDir))
        {
            std::cerr << "Error: Failed to create cache dir: " << currentCacheDir << std::endl;
            return;
        }

        passwd *pw { getpwnam(username.c_str()) };

        if (!pw || seteuid(pw->pw_uid) != 0)
        {
            std::cerr << "Error: Failed to change UID.\n";
            return;
        }
    }

    XDGKit::Options options;
    options.useIconThemesCache = false;
    options.autoReloadCache = false;
    auto kit { XDGKit::Make(options) };

    if (!isSystem && seteuid(0) != 0)
    {
        std::cerr << "Error: Failed to restore UID 0.\n";
        exit(EXIT_FAILURE);
    }

    std::filesystem::path cacheFilePath;
    uint64_t u64;
    uint32_t u32;
    bool boolean;

    for (auto &theme : kit->iconThemeManager().themes())
    {
        const bool inHome { theme.second->indexFilePath().string().starts_with("/home") };

        if (isSystem == inHome)
            continue;

        try {
            std::cout << "    Found theme: " << theme.first << " => " << theme.second->indexFilePath().string() << "\n";
            cacheFilePath = currentCacheDir / theme.first;
            std::ofstream file(cacheFilePath, std::ios::binary);

            if (!file)
            {
                std::cout << "    Failed to create cache file for theme: " << theme.first << "\n";
                continue;
            }

            file.exceptions(std::ofstream::failbit | std::ofstream::badbit);

            // v2 (icons sorted, looked up in the mapped file)
            file.write(XDGIconTheme::CacheMagicV2.data(), XDGIconTheme::CacheMagicV2.size());

            // Theme name
            file.write(theme.first.c_str(), theme.first.size() + 1);

            // Serialized index.theme size
            u64 = theme.second->indexData().mapSize();
            file.write((const char*)&u64, sizeof(u64));

            // Serialized index.theme data
            file.write((const char*)theme.second->indexData().map(), u64);

            // The system cache only covers system dirs: dirs in users' homes (e.g. hicolor in
            // ~/.local/share/icons, found through the environment sudo keeps) are scanned by each
            // app at runtime
            const auto included = [isSystem](const XDGIconDirectory &dir)
            {
                return !isSystem || !dir.themeDir().starts_with("/home/");
            };

            // Offsets tables and records of every dir, written after all the dir headers (so loading
            // the cache only touches the headers' pages)
            std::string data;

            const auto writeDirs = [&](const std::list<XDGIconDirectory> &dirs, bool scaled)
            {
                for (const auto &dir : dirs)
                {
                    if (!included(dir))
                        continue;

                    // Is Scaled
                    boolean = scaled;
                    file.write((const char*)&boolean, sizeof(boolean));

                    // Cache data (size, minSize, etc)
                    file.write((const char*)dir.data(), sizeof(*dir.data()));

                    // Theme dir
                    file.write((const char*)dir.themeDir().data(), dir.themeDir().size() + 1);

                    // Dir name
                    file.write((const char*)dir.dirName().data(), dir.dirName().size() + 1);

                    // Icons, sorted by name (binary searched by XDGIconDirectory::findIcon())
                    std::vector<std::pair<std::string_view, uint32_t>> icons;
                    icons.reserve(dir.icons().size());

                    for (const auto &icon : dir.icons())
                        icons.emplace_back(icon.first, icon.second.extensions());

                    std::sort(icons.begin(), icons.end(), [](const auto &a, const auto &b){ return a.first < b.first; });

                    // Icons num
                    u64 = icons.size();
                    file.write((const char*)&u64, sizeof(u64));

                    // Records size and where its block starts in the data section. The block: the
                    // offset of each record (u32), then the records: name (NUL terminated), extensions (u32)
                    uint64_t recordsSize { 0 };
                    std::vector<uint32_t> offsets;
                    offsets.reserve(icons.size());

                    for (const auto &icon : icons)
                    {
                        offsets.emplace_back(uint32_t(recordsSize));
                        recordsSize += icon.first.size() + 1 + sizeof(uint32_t);
                    }

                    file.write((const char*)&recordsSize, sizeof(recordsSize));
                    u64 = data.size();
                    file.write((const char*)&u64, sizeof(u64));

                    data.append((const char*)offsets.data(), offsets.size() * sizeof(uint32_t));

                    for (const auto &icon : icons)
                    {
                        data.append(icon.first);
                        data.push_back('\0');
                        u32 = icon.second;
                        data.append((const char*)&u32, sizeof(u32));
                    }
                }
            };

            // Directories num
            u64 = std::count_if(theme.second->iconDirectories().begin(), theme.second->iconDirectories().end(), included) +
                  std::count_if(theme.second->scaledIconDirectories().begin(), theme.second->scaledIconDirectories().end(), included);
            file.write((const char*)&u64, sizeof(u64));

            writeDirs(theme.second->iconDirectories(), false);
            writeDirs(theme.second->scaledIconDirectories(), true);

            // Data section
            file.write(data.data(), data.size());

            file.close();

            if (chmod(cacheFilePath.c_str(), S_IRUSR | S_IRGRP | S_IROTH) == 0)
            {
                std::cout << "        Cache permissions set to read-only: " << cacheFilePath.c_str() << "\n";
                std::cout << "        Cache stored succesfully.\n";
            }
            else
                std::cerr << "        Failed to change cache permissions: " << cacheFilePath.c_str() << "\n";
        }
        catch (...)
        {
            std::cout << "        Failed to write into cache file: " << theme.first << "\n";
        }
    }
}

void showHelp()
{
    std::cout << "\nXDGKit Icon Theme Indexer\n\n"
              << "This program generates cache files for icon themes, improving indexing performance in XDGKit.\n"
              << "It should be used each time a theme is added or removed from the system. Running it while apps\n"
              << "are mapping current cache files is safe.\n\n"
              << "Cache files are stored in:\n"
              << "  - System themes: /var/cache/xdgkit/icon_themes/system\n"
              << "  - User themes: /var/cache/xdgkit/icon_themes/<user>\n\n"
              << "Additional search paths can be specified using the XDG_DATA_DIRS environment variable.\n";
}

int main(int argc, char* argv[])
{
    for (int i = 1; i < argc; i++)
    {
        std::string_view arg = argv[i];
        if (arg == "-h" || arg == "--help")
        {
            showHelp();
            return 0;
        }
    }

    setenv("XDGKIT_DEBUG", "3", 0);

    if (getuid() != 0)
    {
        std::cerr << "Error: Elevated permissions are required to modify " << cacheDir.string() << ". Try running the command as the root user.\n";
        return EXIT_FAILURE;
    }

    if (std::filesystem::is_directory(cacheDir))
    {
        if (!std::filesystem::remove_all(cacheDir))
        {
            std::cerr << "Error: Failed to clear cache dir: " << cacheDir << std::endl;
            return EXIT_FAILURE;
        }
    }

    if (!std::filesystem::create_directories(cacheDir))
    {
        std::cerr << "Error: Failed to create cache dir: " << cacheDir << std::endl;
        return EXIT_FAILURE;
    }

    if (!std::filesystem::create_directories(systemCacheDir))
    {
        std::cerr << "Error: Failed to create cache dir: " << systemCacheDir << std::endl;
        return EXIT_FAILURE;
    }

    if (!std::filesystem::create_directories(usersCacheDir))
    {
        std::cerr << "Error: Failed to create cache dir: " << usersCacheDir << std::endl;
        return EXIT_FAILURE;
    }

    for (const auto &entry : std::filesystem::directory_iterator("/home"))
        if (entry.is_directory())
            saveCache(entry.path().filename());

    saveCache(""); // System

    return 0;
}
