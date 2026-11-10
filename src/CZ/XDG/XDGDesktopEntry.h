#ifndef XDGDESKTOPENTRY_H
#define XDGDESKTOPENTRY_H

#include <CZ/XDG/XDG.h>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

/**
 * @brief The subset of a desktop entry (freedesktop Desktop Entry Specification) used to present
 *        and launch applications.
 *
 * @see XDGKit::desktopEntry()
 */
class CZ::XDGDesktopEntry
{
public:
    /// An "Additional application action" ([Desktop Action <id>] group listed in Actions=).
    struct Action
    {
        std::string id;
        std::string name;   ///< Localized
        std::string exec;   ///< Raw Exec value (may be empty for D-Bus activatable apps)
    };

    std::filesystem::path path;
    std::string id;         ///< Desktop file id (basename without ".desktop")
    std::string name;       ///< Localized Name
    std::string icon;       ///< Icon (theme icon name or absolute path)
    std::string exec;       ///< Raw Exec value
    bool dbusActivatable { false };
    std::vector<Action> actions;

    /**
     * @brief Loads a desktop entry file.
     *
     * @return The entry, or nullptr if it can't be read, isn't of type Application or is Hidden.
     */
    static std::shared_ptr<XDGDesktopEntry> Load(const std::filesystem::path &path) noexcept;

    /**
     * @brief Launches the application (its Exec key), detached.
     *
     * @return true if the process was spawned.
     */
    bool launch() const noexcept { return launch(exec); }

    /**
     * @brief Launches @p exec (an Exec value, e.g. an action's), detached, expanding or removing its
     *        field codes. No files or URLs are passed.
     *
     * @return true if the process was spawned.
     */
    bool launch(const std::string &exec) const noexcept;
};

#endif // XDGDESKTOPENTRY_H
