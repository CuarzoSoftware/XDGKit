#ifndef XDGMIMEDATABASE_H
#define XDGMIMEDATABASE_H

#include <CZ/XDG/XDG.h>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * @brief File type detection and default applications (shared-mime-info and mimeapps.list).
 *
 * Types are detected from file names (the shared-mime-info `globs2` files), with a few fallbacks
 * for names no glob matches: directories, empty files and plain text. Content sniffing (`magic`)
 * isn't supported.
 *
 * Default applications follow the XDG MIME Applications spec: `mimeapps.list` files (defaults and
 * added/removed associations), then the `mimeinfo.cache` of each applications directory, then the
 * type's parents (e.g. text/x-csrc falls back to text/plain).
 *
 * The databases are loaded on first use. Get it from XDGKit::mimeDatabase().
 */
class CZ::XDGMimeDatabase
{
public:
    /// The type of the file at @p path (follows symlinks). Never empty.
    std::string typeForFile(const std::filesystem::path &path) noexcept;

    /// The type of a file named @p name (globs only). Empty if no glob matches.
    std::string typeForName(const std::string &name) noexcept;

    /// The canonical name of @p mimeType (resolves aliases).
    std::string canonical(const std::string &mimeType) noexcept;

    /// Direct parents of @p mimeType (subclasses), plus the implicit ones (text/plain for text/*,
    /// application/octet-stream for everything but inode/*).
    std::vector<std::string> parents(const std::string &mimeType) noexcept;

    /// Whether @p mimeType is @p base or a subclass of it.
    bool inherits(const std::string &mimeType, const std::string &base) noexcept;

    /// Icon names for @p mimeType, best first, ending with the generic ones (e.g. "text-x-generic").
    std::vector<std::string> iconNames(const std::string &mimeType) noexcept;

    /// Human readable description (the type's comment), e.g. "PNG image". Falls back to the type.
    std::string description(const std::string &mimeType) noexcept;

    /// The default app for @p mimeType (or its parents). nullptr if none.
    std::shared_ptr<const XDGDesktopEntry> defaultApp(const std::string &mimeType) noexcept;

    /// Every app that can open @p mimeType (or its parents), the default first.
    std::vector<std::shared_ptr<const XDGDesktopEntry>> apps(const std::string &mimeType) noexcept;

    /// Drops the loaded databases (reloaded on next use), e.g. after installing apps.
    void reload() noexcept;

private:
    friend class XDGKit;
    XDGMimeDatabase(XDGKit &kit) noexcept : m_kit(kit) {}

    struct Glob
    {
        std::string pattern; // Lowercase unless caseSensitive
        std::string mimeType;
        int weight;
        bool caseSensitive;
    };

    void loadMime() noexcept;
    void loadApps() noexcept;
    std::vector<std::string> associations(const std::string &mimeType, bool defaultsOnly) noexcept;
    std::vector<std::filesystem::path> mimeDirs() const noexcept;
    std::shared_ptr<const XDGDesktopEntry> entry(const std::string &desktopId) noexcept;

    XDGKit &m_kit;
    bool m_mimeLoaded { false }, m_appsLoaded { false };

    std::unordered_map<std::string, Glob> m_literals;    // Whole file names (lowercase)
    std::unordered_map<std::string, Glob> m_extensions;  // "*.ext" globs by ".ext" (lowercase)
    std::vector<Glob> m_complexGlobs;                    // Anything else (fnmatch)
    std::unordered_map<std::string, std::string> m_aliases;
    std::unordered_map<std::string, std::vector<std::string>> m_parents;
    std::unordered_map<std::string, std::string> m_icons, m_genericIcons;
    std::unordered_map<std::string, std::string> m_descriptions;

    // Desktop ids by type: defaults (mimeapps.list) and associations (added + mimeinfo.cache).
    std::unordered_map<std::string, std::vector<std::string>> m_defaults, m_associations, m_removed;
};

#endif // XDGMIMEDATABASE_H
