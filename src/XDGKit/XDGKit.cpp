#include <XDGKit/XDGKit.h>
#include <XDGKit/XDGLog.h>
#include <cstring>
#include <pwd.h>

using namespace XDG;

std::shared_ptr<XDGKit> XDGKit::Make(const Options &options) noexcept
{
    return std::shared_ptr<XDGKit>(new XDGKit(options));
}

void XDGKit::rescanDataDirs() noexcept
{
    m_dataDirs = std::vector<std::filesystem::path>();
    m_dataDirs.reserve(16);

    const char *dataDirs { getenv("XDG_DATA_DIRS") };

    if (!dataDirs || strlen(dataDirs) == 0)
    {
        XDGLog::warning("XDG_DATA_DIRS env is empty. Falling back to /var/lib/flatpak/exports/share:/usr/local/share/:/usr/share/\n");
        setenv("XDG_DATA_DIRS", "/var/lib/flatpak/exports/share:/usr/local/share/:/usr/share/", 0);
    }

    const char *env { getenv("XDG_DATA_DIRS") };
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

XDGKit::XDGKit(const Options &options) noexcept :
    m_options(options),
    m_iconThemeManager(*this)
{
    XDGLog::init();
    initHomeDir();
    rescanDataDirs();
    iconThemeManager().restoreDefaultSearchDirs();
    iconThemeManager().findThemes();
}

void XDGKit::initHomeDir() noexcept
{
    passwd *pw { getpwuid(geteuid()) };

    if (pw)
    {
        if (pw->pw_name)
            m_user = pw->pw_name;

        if (pw->pw_dir)
            m_homeDir = pw->pw_dir;
    }
}
