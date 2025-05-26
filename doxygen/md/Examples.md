# 🕹️ Examples {#examples_page}

### Finding Icons

The following example demonstrates how to load icons from available themes.

XDGKit searches for themes in the standard directories defined by the XDG specification. Additional directories can be specified for icon lookup by setting the `XDG_DATA_DIRS` environment variable **before** creating an XDGKit instance, as explained in @ref XDG::XDGIconThemeManager::searchDirs().

Icon themes are indexed lazily when an icon is first searched within them. This process can be slow depending on the number of subdirectories and icons in the theme.

To improve performance, XDGKit uses cache files. To generate or update the cache, run the `xdgkit-icon-theme-indexer` utility, which scans available themes and stores their information in a compact, serialized format in `~/.cache/xdgkit/icon_themes`. 
When an application loads a theme, it first checks for an existing cache file, if none is found, it loads the theme manually. Using the cache also reduces RAM usage, as all the information is directly mapped from cached files rather than being stored in memory.

Ideally, this utility should be executed at the start of a user session and whenever themes change. If a theme is added or removed while an application is running, the application will not update automatically. In such cases, the user must recreate the XDGKit instance to reflect the changes.

```cpp
#include <XDGKit/XDGKit.h>
#include <iostream>

using namespace XDG;

int main()
{
    XDGKit::Options options;
    options.useIconThemesCache = true; // This is the default value

    // Create an instance of XDGKit.
    auto kit = XDGKit::Make(options);

    // Search for an icon
    const XDGIcon *firefox =
        kit->iconThemeManager().findIcon(
            "firefox",                                  // Icon to search for
            512,                                        // Desired icon size (unscaled)
            2,                                          // Scale factor
            XDGIcon::PNG | XDGIcon::SVG | XDGIcon::XMP, // File extensions to consider
            {""}                                        // Theme names to search in order, "" as wildcard for all themes
        );

    // Check if the icon was found
    if (firefox)
    {
        std::cout
            << "Theme: " << firefox->directory().theme().name() << "\n"
            << "Icon: " << firefox->name() << "\n"
            << "Size: " << firefox->directory().size() << "\n"
            << "Scale: " << firefox->directory().scale() << "\n";

        if (firefox->extensions() & XDGIcon::PNG)
            std::cout << "PNG Path: " << firefox->getPath(XDGIcon::PNG) << "\n";

        if (firefox->extensions() & XDGIcon::SVG)
            std::cout << "SVG Path: " << firefox->getPath(XDGIcon::SVG) << "\n";

        if (firefox->extensions() & XDGIcon::XMP)
            std::cout << "XMP Path: " << firefox->getPath(XDGIcon::XMP) << "\n";

        return 0;
    }
    else
    {
        std::cout << "Could not find an icon named 'firefox'." << "\n";
        return 1;
    }
}
```
