#include <XDGKit/XDGKit.h>
#include <iostream>

using namespace XDG;

int main()
{
    auto kit = XDGKit::Get();

    std::cout << "HOME: " << kit->homeDir().c_str() << "\n";

    std::cout << "XDG_DATA_DIRS:\n";
    for (auto &dir : kit->dataDirs())
        std::cout << " - " << dir.c_str() << "\n";

    std::cout << "ICON THEMES SEARCH DIRS:\n";
    for (auto &dir : kit->iconThemeManager().searchDirs())
        std::cout << " - " << dir.c_str() << "\n";

    std::cout << "FOUND THEMES:\n";
    for (auto &theme : kit->iconThemeManager().themes())
    {
        std::cout << " - NAME: " << theme.first.c_str() << "\n";
        std::cout << "   VISIBLE NAME: " << theme.second.visibleName() << "\n";
        std::cout << "   COMMENT: " << theme.second.comment() << "\n";
        std::cout << "   INHERITS: " << theme.second.inherits() << "\n";
        std::cout << "   HIDDEN: " << theme.second.hidden() << "\n";
        std::cout << "   EXAMPLE: " << theme.second.example() << "\n";
        std::cout << "   INDEX: " << theme.second.indexFilePath().c_str() << "\n";
        std::cout << "   DIRS: ";
        for (auto &dir : theme.second.dirs())
            std::cout << dir.c_str() << ":";
        std::cout << std::endl;
    }


    return 0;
}
