# 🕹️ Examples {#examples_page}

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
