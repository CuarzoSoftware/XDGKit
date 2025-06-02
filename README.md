# XDGKit

<p align="left">
  <a href="https://github.com/CuarzoSoftware/XDGKit/blob/main/LICENSE">
    <img src="https://img.shields.io/badge/license-LGPLv2.1-blue.svg" alt="XDGKit is released under the LGPLv2.1 license." />
  </a>
  <a href="https://github.com/CuarzoSoftware/XDGKit">
    <img src="https://img.shields.io/badge/version-1.0.0-brightgreen" alt="Current XDGKit version." />
  </a>
</p>

XDGKit is a C++ library providing utilities for working with XDG standards.

### Features

- **Icon Themes**: Tools for searching icons, adhering to the [XDG Icon Theme Specification v0.13](https://specifications.freedesktop.org/icon-theme-spec/latest/#overview).

> This library does not load icons into memory. Instead, it provides fast lookups for icon metadata, including paths, extensions, sizes, and other relevant details.

## Links

* [📖 C++ API Documentation](https://cuarzosoftware.github.io/XDGKit/annotated.html)
* [🕹️ Examples](https://cuarzosoftware.github.io/XDGKit/examples_page.html)
* [📦 Downloads](https://cuarzosoftware.github.io/XDGKit/downloads_page.html)
* [⚙️ Environment](https://cuarzosoftware.github.io/XDGKit/environment_page.html)
* [💬 Contact](https://cuarzosoftware.github.io/XDGKit/contact_page.html)

### Example

```cpp
#include <XDGKit/XDGKit.h>
#include <iostream>

using namespace XDG;

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
