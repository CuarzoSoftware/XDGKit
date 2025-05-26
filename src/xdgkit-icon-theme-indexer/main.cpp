#include <XDGKit/XDGKit.h>
#include <cassert>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <sys/stat.h>

using namespace XDG;

int main()
{
    XDGKit::Options options;
    options.useIconThemesCache = false;

    auto kit { XDGKit::Make(options) };

    const std::filesystem::path cacheDir { kit->homeDir() / ".cache/xdgkit/icon_themes" };

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

    std::filesystem::path cacheFilePath;
    uint64_t u64;
    uint32_t u32;
    bool boolean;

    for (auto &theme : kit->iconThemeManager().themes())
    {
        try {
            std::cout << "Found theme: " << theme.first << "\n";
            cacheFilePath = cacheDir / theme.first;
            std::ofstream file(cacheFilePath, std::ios::binary);

            if (!file)
            {
                std::cout << "Failed to create cache file for theme: " << theme.first << "\n";
                continue;
            }

            file.exceptions(std::ofstream::failbit | std::ofstream::badbit);

            // Theme name
            file.write(theme.first.c_str(), theme.first.size() + 1);

            // Serialized index.theme size
            u64 = theme.second->indexData().mapSize();
            file.write((const char*)&u64, sizeof(u64));

            // std::cout << "INDEX DATA SIZE: " << u64 << "\n";

            // Serialized index.theme data
            file.write((const char*)theme.second->indexData().map(), u64);

            // Directories num
            u64 = theme.second->iconDirectories().size() + theme.second->scaledIconDirectories().size();
            file.write((const char*)&u64, sizeof(u64));

            // std::cout << "NUM DIRS: " << u64 << "\n";

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
                std::cout << "\tFile permissions set to read-only: " << cacheFilePath.c_str() << "\n";
                std::cout << "\tCache stored succesfully.\n";
            }
            else
                std::cerr << "\tFailed to change file permissions: " << cacheFilePath.c_str() << "\n";
        }
        catch (...)
        {
            std::cout << "\tFailed to write into cache file: " << theme.first << "\n";
        }
    }

    return 0;
}
