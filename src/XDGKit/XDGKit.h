#ifndef XDGKIT_H
#define XDGKIT_H

#include <XDGKit/XDGIconThemeManager.h>
#include <filesystem>
#include <memory>
#include <vector>

class XDG::XDGKit
{
public:
    static std::shared_ptr<XDGKit> Get() noexcept;

    XDGIconThemeManager &iconThemeManager() noexcept
    {
        return m_iconThemeManager;
    }

    const std::filesystem::path &homeDir() const noexcept
    {
        return m_homeDir;
    }

    void rescanDataDirs() noexcept;

    const std::vector<std::filesystem::path> &dataDirs() const noexcept
    {
        return m_dataDirs;
    }

private:
    XDGKit() noexcept;
    void initHomeDir() noexcept;
    XDGIconThemeManager m_iconThemeManager;
    std::filesystem::path m_homeDir;
    std::vector<std::filesystem::path> m_dataDirs;
};

#endif // XDGKIT_H
