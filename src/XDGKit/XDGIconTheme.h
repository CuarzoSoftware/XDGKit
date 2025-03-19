#ifndef XDGICONTHEME_H
#define XDGICONTHEME_H

#include <XDGKit/XDGNamespace.h>
#include <XDGKit/XDGParsers.h>
#include <XDGKit/XDGIconDirectory.h>
#include <filesystem>
#include <string>
#include <vector>

class XDG::XDGIconTheme
{
public:
    XDGIconTheme(XDGKit &kit) noexcept;

    // The theme directory basename
    const std::string &name() const noexcept
    {
        return m_name;
    }

    // Display name
    const std::string &visibleName() const noexcept
    {
        return m_visibleName;
    }

    // Long name
    const std::string &comment() const noexcept
    {
        return m_comment;
    }

    const std::vector<std::string> &inherits() const noexcept
    {
        return m_inherits;
    }

    // Example icon name
    const std::string &example() const noexcept
    {
        return m_example;
    }

    bool hidden() const noexcept
    {
        return m_hidden;
    }

    // Path to the first index.theme found
    const std::filesystem::path &indexFilePath() const noexcept
    {
        return m_indexFilePath;
    }

    // All dirs where this theme was found
    const std::vector<std::filesystem::path> &dirs() const noexcept
    {
        return m_dirs;
    }

    // Sections and values from index.theme
    const XDGINI &indexData() const noexcept
    {
        return m_indexData;
    }

    // Normal icon dirs
    const std::vector<XDGIconDirectory> &iconDirectories() const noexcept
    {
        return m_iconDirectories;
    }

    // Scaled icon dirs
    const std::vector<XDGIconDirectory> &scaledIconDirectories() const noexcept
    {
        return m_scaledIconDirectories;
    }

private:
    friend class XDGIconThemeManager;
    void initIconsDir(const std::vector<std::string> &iconDirs, XDGIconDirectory::Type type) noexcept;
    XDGKit &m_kit;
    std::string m_name;
    std::string m_visibleName;
    std::string m_comment;
    std::string m_example;
    std::vector<std::string> m_inherits;
    std::vector<XDGIconDirectory> m_iconDirectories;
    std::vector<XDGIconDirectory> m_scaledIconDirectories;
    std::vector<std::filesystem::path> m_dirs;
    std::filesystem::path m_indexFilePath;
    XDGINI m_indexData;
    bool m_hidden { false };
};

#endif // XDGICONTHEME_H
