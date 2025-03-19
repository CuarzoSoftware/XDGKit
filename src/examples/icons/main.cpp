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
        std::cout << "   INHERITS: [";
        for (const auto &inh : theme.second.inherits())
            std::cout << inh << ", ";
        std::cout << "]\n";
        std::cout << "   HIDDEN: " << theme.second.hidden() << "\n";
        std::cout << "   EXAMPLE: " << theme.second.example() << "\n";
        std::cout << "   INDEX: " << theme.second.indexFilePath().c_str() << "\n";
        std::cout << "   THEME DIRS: ";
        for (auto &dir : theme.second.dirs())
            std::cout << dir.c_str() << ":";
        std::cout << std::endl;

        std::cout << "   NORMAL ICON DIRS:\n";
        for (auto &dir : theme.second.iconDirectories())
        {
            std::cout << "      Scale:" << dir.scale()
                      << " Size:" << dir.size()
                      << " MinSize:" << dir.minSize()
                      << " MaxSize:" << dir.maxSize()
                      << " Type:" << dir.type()
                      << " SizeType:" << dir.sizeType()
                      << " Context:" << dir.context()
                      << " Threshold:" << dir.threshold()
                      << " Dir:" << dir.dir() << "\n";
        }

        std::cout << "   SCALED ICON DIRS:\n";
        for (auto &dir : theme.second.scaledIconDirectories())
        {
            std::cout << "      Scale:" << dir.scale()
                      << " Size:" << dir.size()
                      << " MinSize:" << dir.minSize()
                      << " MaxSize:" << dir.maxSize()
                      << " Type:" << dir.type()
                      << " SizeType:" << dir.sizeType()
                      << " Context:" << dir.context()
                      << " Threshold:" << dir.threshold()
                      << " Dir:" << dir.dir() << "\n";
        }
    }


    return 0;
}
