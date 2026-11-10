#ifndef XDGTRASH_H
#define XDGTRASH_H

#include <CZ/XDG/XDG.h>
#include <filesystem>
#include <string>
#include <vector>

/**
 * @brief The user's home trash (FreeDesktop.org Trash specification).
 *
 * Files are moved to `$XDG_DATA_HOME/Trash/files` (`~/.local/share/Trash` by default) with a
 * matching `.trashinfo` in `Trash/info`, so file managers can list and restore them.
 */
class CZ::XDGTrash
{
public:
    enum class Result
    {
        Trashed,       ///< Moved to the home trash
        OtherDevice,   ///< On another file system: it belongs in that device's trash (e.g. use `gio trash`)
        Failed         ///< See the error message
    };

    /// The home trash directory (`$XDG_DATA_HOME/Trash`).
    static std::filesystem::path HomeDir() noexcept;

    /// Moves @p path to the home trash.
    static Result MoveToTrash(const std::filesystem::path &path, std::string *error = nullptr) noexcept;

    /// Whether the home trash has no files.
    static bool IsEmpty() noexcept;

    /// Local paths in a `text/uri-list` (file:// URIs, percent-decoded). Other schemes are skipped.
    static std::vector<std::filesystem::path> ParseUriList(const std::string &uriList) noexcept;

    /// Percent-encodes a path (for .trashinfo files and file:// URIs).
    static std::string EncodePath(const std::string &path) noexcept;
};

#endif // XDGTRASH_H
