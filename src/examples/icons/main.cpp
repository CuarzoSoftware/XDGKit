#include <XDGKit/XDGKit.h>
#include <iostream>
#include <fstream>

using namespace XDG;

static const std::vector<int> testSizes { 16, 32, 48, 64, 96, 128, 256, 512 };
static const std::vector<int> testExtensions { XDGIcon::SVG, XDGIcon::PNG  };
static const std::vector<std::string> testIcons
{
    "firefox", "code", "gedit", "foot", "spotify", "google-chrome", "discord",
    "qtcreator", "nemo", "user-info", "weather", "web-browser", "chat", "vscode",
    "date", "zoom-out", "zoom-in", "accessories-calculator", "accessories-clock",
    "application-default", "chat-symbolic", "audio-x-generic", "video-x-generic",
    "x-office-document-template", "zoom", "store"
};

static void printMemoryUsageInMB()
{
    std::ifstream statusFile { "/proc/self/status" };
    if (!statusFile.is_open()) return;

    std::string line;
    long memoryUsageKB { 0 };

    while (std::getline(statusFile, line))
    {
        // Look for the VmRSS line
        if (line.compare(0, 6, "VmRSS:") == 0)
        {
            // Extract the memory usage value (in KB)
            std::istringstream iss(line.substr(6));
            iss >> memoryUsageKB;
            break;
        }
    }

    statusFile.close();
    double memoryUsageMB { static_cast<double>(memoryUsageKB) / 1024.0 };
    std::cout << "Memory usage: " << memoryUsageMB << " MB" << std::endl;
}

int main()
{
    const XDGIcon *icon;
    const char *ext;
    int64_t maxTimeMs { 0 };
    int64_t sumTimeMs { 0 };
    int64_t count { 0 };
    int64_t found { 0 };
    int64_t indexingTimeMs { 0 };

    auto start = std::chrono::high_resolution_clock::now();
    auto kit = XDGKit::Make();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    indexingTimeMs = duration.count();

    for (const auto &iconName : testIcons)
    {
        for (auto extension : testExtensions)
        {
            if (extension == XDGIcon::PNG) ext = "[PNG]";
            else if (extension == XDGIcon::SVG) ext = "[SVG]";
            else ext = "[XPM]";

            for (int scale = 1; scale < 3; scale++)
            {
                for (int size : testSizes)
                {
                    count++;
                    start = std::chrono::high_resolution_clock::now();
                    icon = kit->iconThemeManager().findIcon(iconName, size, scale, extension);
                    end = std::chrono::high_resolution_clock::now();
                    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                    sumTimeMs += duration.count();

                    if (duration.count() > maxTimeMs)
                        maxTimeMs = duration.count();

                    if (icon)
                    {
                        found++;
                        printf("%s", "\033[0;32m");
                    }
                    else
                        printf("%s", "\033[0;31m");

                    printf("[FOUND:%s]\033[0m[%s]%s[%d@x%d]",
                           icon ? "Y" : "N",
                           iconName.c_str(),
                           ext, size, scale);

                    if (duration.count() > 0) printf("%s","\033[0;33m");
                    printf("[%ld ms]\n", duration.count());
                    if (duration.count() > 0) printf("%s","\033[0m");
                }
            }
        }

        printMemoryUsageInMB();
    }

    std::cout << std::endl;
    std::cout << "Themes Found:\t\t" << kit->iconThemeManager().themes().size() << "\n";
    std::cout << "Lookup test results:\t" << found << "/" << count << " icons found\n";
    std::cout << "Indexing Time:\t\t" << indexingTimeMs << " ms\n";
    std::cout << "Max Lookup Time:\t" << maxTimeMs << " ms\n";
    std::cout << "Avg Lookup Time:\t" << sumTimeMs/count << " ms" << std::endl;
    printMemoryUsageInMB();
    return 0;
}
