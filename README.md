# XDGKit

<p align="left">
  <a href="https://github.com/CuarzoSoftware/XDGKit/blob/main/LICENSE">
    <img src="https://img.shields.io/badge/license-LGPLv2.1-blue.svg" alt="XDGKit is released under the LGPLv2.1 license." />
  </a>
  <a href="https://github.com/CuarzoSoftware/XDGKit">
    <img src="https://img.shields.io/badge/version-0.1.0-brightgreen" alt="Current XDGKit version." />
  </a>
</p>

XDGKit is a C++ library providing utilities for working with XDG standards.

### Features

- **Icon Themes**: Tools for searching icons, adhering to the [XDG Icon Theme Specification v0.13](https://specifications.freedesktop.org/icon-theme-spec/latest/#overview).

## Examples

### Finding Icons

```cpp
#include <XDGKit/XDGKit.h>
#include <iostream>

using namespace XDG;

int main()
{
    // Create an instance of XDGKit.
    auto kit = XDGKit::Make();

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

## Build

Install a C++ compiler, Meson and then:

```bash
$ git clone https://github.com/CuarzoSoftware/XDGKit.git
$ cd XDGKit
$ meson setup builddir
$ cd builddir
$ meson install
```
