#include <XDGKit/XDGKit.h>
#include <cassert>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <sys/stat.h>
#include <pwd.h>

/* THEME CACHE FORMAT

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
    FOREACH ICON:
        str: icon name
        u32: extensions
*/

/* CACHE DIRS

For system themes : /var/cache/xdgkit/icon_themes/system
For user themes   : /var/cache/xdgkit/icon_themes/users/<username>
*/

using namespace XDG;

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

            // Theme name
            file.write(theme.first.c_str(), theme.first.size() + 1);

            // Serialized index.theme size
            u64 = theme.second->indexData().mapSize();
            file.write((const char*)&u64, sizeof(u64));

            // Serialized index.theme data
            file.write((const char*)theme.second->indexData().map(), u64);

            // Directories num
            u64 = theme.second->iconDirectories().size() + theme.second->scaledIconDirectories().size();
            file.write((const char*)&u64, sizeof(u64));

            // Unscaled dirs
            boolean = false;
            for (const auto &dir : theme.second->iconDirectories())
            {
                // Is Scaled
                file.write((const char*)&boolean, sizeof(boolean));

                // Cache data (size, minSize, etc)
                file.write((const char*)dir.data(), sizeof(*dir.data()));

                // Theme dir
                file.write((const char*)dir.themeDir().data(), dir.themeDir().size() + 1);

                // Dir name
                file.write((const char*)dir.dirName().data(), dir.dirName().size() + 1);

                // Icons num
                u64 = dir.icons().size();
                file.write((const char*)&u64, sizeof(u64));

                for (const auto &icon : dir.icons())
                {
                    // Icon name
                    file.write((const char*)icon.first.data(), icon.first.size() + 1);

                    // Extensions
                    u32 = icon.second.extensions();
                    file.write((const char*)&u32, sizeof(u32));
                }
            }

            // Scaled dirs
            boolean = true;
            for (const auto &dir : theme.second->iconDirectories())
            {
                // Is Scaled
                file.write((const char*)&boolean, sizeof(boolean));

                // Cache data (size, minSize, etc)
                file.write((const char*)dir.data(), sizeof(*dir.data()));

                // Theme dir
                file.write((const char*)dir.themeDir().data(), dir.themeDir().size() + 1);

                // Dir name
                file.write((const char*)dir.dirName().data(), dir.dirName().size() + 1);

                // Icons num
                u64 = dir.icons().size();
                file.write((const char*)&u64, sizeof(u64));

                for (const auto &icon : dir.icons())
                {
                    // Icon name
                    file.write((const char*)icon.first.data(), icon.first.size() + 1);

                    // Extensions
                    u32 = icon.second.extensions();
                    file.write((const char*)&u32, sizeof(u32));
                }
            }

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

int main()
{
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
