# 🕹️ Examples {#examples_page}

### Finding Icons

The following example demonstrates how to load icons from available themes.

XDGKit searches for themes in the standard directories defined by the XDG specification. Additional directories can be specified for icon lookup by setting the `XDG_DATA_DIRS` environment variable **before** creating an XDGKit instance, as explained in CZ::XDGIconThemeManager::searchDirs().

Icon themes are indexed lazily when an icon is first searched within them. This process can be slow depending on the number of subdirectories and icons in the theme.

To improve performance, XDGKit uses cache files. To generate or update the cache, run the `xdgkit-icon-theme-indexer` utility, which scans available themes and stores their information in a compact, serialized format under `/var/cache/xdgkit/icon_themes`.
When XDGKkit searches for an icon within a theme for the first time, it first checks for an existing cache file. If no cache file is found, the application loads and indexes the information manually (slow). Using the cache also reduces RAM usage, as all the information is directly mapped from cached files rather than being stored in memory.

```cpp
#include <CZ/XDG/XDGKit.h>
#include <iostream>

using namespace CZ;

int main()
{
    XDGKit::Options options;

    // Load themes from cache when possible
    options.useIconThemesCache = true;

    // Automatically reload themes if a cache update is detected
    options.autoReloadCache = true;

    // Create an instance of XDGKit.
    auto kit = XDGKit::Make(options);

    // Search for an icon
    const XDGIcon *firefox =
        kit->iconThemeManager().findIcon(
            "firefox",                                  // Icon to search for
            512,                                        // Desired icon size (unscaled)
            2,                                          // Scale factor
            XDGIcon::PNG | XDGIcon::SVG | XDGIcon::XMP, // File extensions to consider
            { "Adwaita", "" }                           // Theme names to search in order, "" as wildcard for all themes
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
