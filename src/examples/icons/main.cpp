#include <XDGKit/XDGKit.h>
#include <iostream>

using namespace XDG;

int main()
{
    size_t themesN = 0;
    size_t dirsN = 0;
    size_t iconsN = 0;

    auto start = std::chrono::high_resolution_clock::now();
    auto kit = XDGKit::Make();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    themesN = kit->iconThemeManager().themes().size();

    for (auto const &theme : kit->iconThemeManager().themes())
    {
        dirsN += theme.second.iconDirectories().size() + theme.second.scaledIconDirectories().size();

        for (const auto &dir : theme.second.iconDirectories())
            iconsN += dir.icons().size();

        for (const auto &dir : theme.second.scaledIconDirectories())
            iconsN += dir.icons().size();
    }

    std::cout << duration.count() << " ms to index " << themesN << " themes " << dirsN << " icon dirs " << iconsN << " icons." << std::endl;

    start = std::chrono::high_resolution_clock::now();
    const XDGIcon *firefox { kit->iconThemeManager().findIcon("firefox", 512, 2, XDGIcon::PNG | XDGIcon::SVG) };
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << duration.count() << " ms to find icon." << std::endl;

    if (firefox)
    {
        std::cout << firefox->directory().theme().name() << ":firefox " << firefox->directory().size() << "@" << firefox->directory().scale() << std::endl;

        if (firefox->extensions() & XDGIcon::PNG)
            std::cout << "PNG PATH: " << firefox->getPath(XDGIcon::PNG) << std::endl;

        if (firefox->extensions() & XDGIcon::SVG)
            std::cout << "SVG PATH: " << firefox->getPath(XDGIcon::SVG) << std::endl;
    }
    else
        std::cout << "Could not find an icon named 'firefox'." << std::endl;

    return 0;
}
