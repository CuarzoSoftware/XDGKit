#include <XDGKit/XDGKit.h>

using namespace XDG;

std::shared_ptr<XDGKit> XDGKit::Make() noexcept
{
    return std::shared_ptr<XDGKit>(new XDGKit());
}

void XDGKit::rescanDataDirs() noexcept
{
    m_dataDirs = std::vector<std::filesystem::path>();
    m_dataDirs.reserve(16);

    const char *env { std::getenv("XDG_DATA_DIRS") };
    if (!env) return;

    const std::string pathsString { env };
    std::istringstream stream { pathsString };
    std::string found;

    while (std::getline(stream, found, ':'))
    {
        const std::filesystem::path path { std::move(found) };
        if (std::filesystem::is_directory(path) && path.is_absolute())
            m_dataDirs.emplace_back(std::move(path));
        found.clear();
    }
}

XDGKit::XDGKit() noexcept :
    m_iconThemeManager(*this)
{
    initHomeDir();
    rescanDataDirs();
    iconThemeManager().restoreDefaultSearchDirs();
    iconThemeManager().findThemes();
}

void XDGKit::initHomeDir() noexcept
{
    const char *env { std::getenv("HOME") };
    if (!env) return;
    m_homeDir = env;
}
