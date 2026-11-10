#include <CZ/XDG/XDGMimeDatabase.h>
#include <CZ/XDG/XDGDesktopEntry.h>
#include <CZ/XDG/XDGKit.h>
#include <CZ/XDG/XDGINI.h>
#include <CZ/XDG/XDGLog.h>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <fnmatch.h>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <sys/stat.h>

using namespace CZ;
namespace fs = std::filesystem;

static std::string Lower(std::string s) noexcept
{
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
    return s;
}

static std::vector<std::string> Split(const std::string &s, char sep) noexcept
{
    std::vector<std::string> out;
    std::string cur;

    for (const char c : s)
    {
        if (c == sep)
        {
            if (!cur.empty()) out.emplace_back(std::move(cur));
            cur.clear();
        }
        else
            cur += c;
    }

    if (!cur.empty())
        out.emplace_back(std::move(cur));

    return out;
}

static fs::path EnvDir(const char *var, const fs::path &fallback) noexcept
{
    if (const char *v { getenv(var) }; v && v[0] == '/')
        return v;

    return fallback;
}

void XDGMimeDatabase::reload() noexcept
{
    m_mimeLoaded = m_appsLoaded = false;
    m_literals.clear();
    m_extensions.clear();
    m_complexGlobs.clear();
    m_aliases.clear();
    m_parents.clear();
    m_icons.clear();
    m_genericIcons.clear();
    m_descriptions.clear();
    m_defaults.clear();
    m_associations.clear();
    m_removed.clear();
}

std::vector<fs::path> XDGMimeDatabase::mimeDirs() const noexcept
{
    // Highest priority first.
    std::vector<fs::path> dirs;

    if (!m_kit.homeDir().empty() || getenv("XDG_DATA_HOME"))
        dirs.emplace_back(EnvDir("XDG_DATA_HOME", m_kit.homeDir() / ".local/share") / "mime");

    for (const auto &dir : m_kit.dataDirs())
        dirs.emplace_back(dir / "mime");

    return dirs;
}

void XDGMimeDatabase::loadMime() noexcept
{
    if (m_mimeLoaded)
        return;

    m_mimeLoaded = true;
    auto dirs { mimeDirs() };

    // Lowest priority first, so higher priority directories overwrite.
    std::reverse(dirs.begin(), dirs.end());

    for (const auto &dir : dirs)
    {
        std::ifstream globs { dir / "globs2" };
        std::string line;

        while (std::getline(globs, line))
        {
            if (line.empty() || line[0] == '#')
                continue;

            // weight:type:glob[:flags]
            const auto a { line.find(':') };
            const auto b { a == std::string::npos ? a : line.find(':', a + 1) };

            if (b == std::string::npos)
                continue;

            Glob glob;
            glob.weight = std::atoi(line.substr(0, a).c_str());
            glob.mimeType = line.substr(a + 1, b - a - 1);
            glob.pattern = line.substr(b + 1);
            glob.caseSensitive = false;

            if (const auto c { glob.pattern.find(':') }; c != std::string::npos)
            {
                glob.caseSensitive = glob.pattern.find("cs", c) != std::string::npos;
                glob.pattern.resize(c);
            }

            if (glob.pattern.empty() || glob.mimeType == "__NOGLOBS__")
                continue;

            const bool simpleExt { glob.pattern.size() > 2 && glob.pattern[0] == '*' && glob.pattern[1] == '.' &&
                glob.pattern.find_first_of("*?[", 1) == std::string::npos };
            const bool literal { glob.pattern.find_first_of("*?[") == std::string::npos };

            if (glob.caseSensitive || (!simpleExt && !literal))
            {
                m_complexGlobs.emplace_back(std::move(glob));
                continue;
            }

            glob.pattern = Lower(glob.pattern);
            auto &map { literal ? m_literals : m_extensions };
            const std::string key { literal ? glob.pattern : glob.pattern.substr(1) };

            // Keep the heaviest glob for a pattern (the first listed one on ties, like GIO).
            if (auto it { map.find(key) }; it == map.end() || it->second.weight < glob.weight)
                map[key] = std::move(glob);
        }

        const auto readPairs = [](const fs::path &file, char sep, auto &&fn) {
            std::ifstream in { file };
            std::string line;

            while (std::getline(in, line))
            {
                if (line.empty() || line[0] == '#')
                    continue;

                if (const auto pos { line.find(sep) }; pos != std::string::npos)
                    fn(line.substr(0, pos), line.substr(pos + 1));
            }
        };

        readPairs(dir / "aliases", ' ', [this](std::string alias, std::string type){ m_aliases[alias] = type; });
        readPairs(dir / "subclasses", ' ', [this](std::string type, std::string parent){
            auto &list { m_parents[type] };
            if (std::find(list.begin(), list.end(), parent) == list.end())
                list.emplace_back(parent);
        });
        readPairs(dir / "icons", ':', [this](std::string type, std::string icon){ m_icons[type] = icon; });
        readPairs(dir / "generic-icons", ':', [this](std::string type, std::string icon){ m_genericIcons[type] = icon; });
    }
}

std::string XDGMimeDatabase::typeForName(const std::string &name) noexcept
{
    loadMime();

    const std::string lower { Lower(name) };
    const Glob *best { nullptr };

    const auto consider = [&best](const Glob &glob) {
        if (!best || glob.weight > best->weight || (glob.weight == best->weight && glob.pattern.size() > best->pattern.size()))
            best = &glob;
    };

    if (auto it { m_literals.find(lower) }; it != m_literals.end())
        consider(it->second);

    // Every suffix starting at a dot, e.g. "a.tar.gz" → ".tar.gz", ".gz".
    for (size_t dot { lower.find('.') }; dot != std::string::npos; dot = lower.find('.', dot + 1))
        if (auto it { m_extensions.find(lower.substr(dot)) }; it != m_extensions.end())
            consider(it->second);

    for (const auto &glob : m_complexGlobs)
        if (fnmatch(glob.pattern.c_str(), glob.caseSensitive ? name.c_str() : lower.c_str(), 0) == 0)
            consider(glob);

    return best ? best->mimeType : std::string {};
}

std::string XDGMimeDatabase::typeForFile(const fs::path &path) noexcept
{
    struct stat st;

    if (stat(path.c_str(), &st) != 0)
        return lstat(path.c_str(), &st) == 0 && S_ISLNK(st.st_mode) ? "inode/symlink" : "application/octet-stream";

    if (S_ISDIR(st.st_mode))  return "inode/directory";
    if (S_ISCHR(st.st_mode))  return "inode/chardevice";
    if (S_ISBLK(st.st_mode))  return "inode/blockdevice";
    if (S_ISFIFO(st.st_mode)) return "inode/fifo";
    if (S_ISSOCK(st.st_mode)) return "inode/socket";

    if (auto type { typeForName(path.filename().string()) }; !type.empty())
        return canonical(type);

    if (st.st_size == 0)
        return "application/x-zerosize";

    // A tiny bit of sniffing for names without a known extension.
    char buf[512];
    std::ifstream in { path, std::ios::binary };
    in.read(buf, sizeof(buf));
    const std::streamsize n { in.gcount() };

    if (n >= 4 && std::memcmp(buf, "\x7f" "ELF", 4) == 0)
        return "application/x-executable";

    if (n >= 2 && buf[0] == '#' && buf[1] == '!')
        return "application/x-shellscript";

    for (std::streamsize i = 0; i < n; i++)
    {
        const unsigned char c ( buf[i] );

        if (c == 0 || (c < 0x20 && c != '\n' && c != '\r' && c != '\t' && c != '\f' && c != 0x1b))
            return "application/octet-stream";
    }

    return "text/plain";
}

std::string XDGMimeDatabase::canonical(const std::string &mimeType) noexcept
{
    loadMime();
    auto it { m_aliases.find(mimeType) };
    return it != m_aliases.end() ? it->second : mimeType;
}

std::vector<std::string> XDGMimeDatabase::parents(const std::string &mimeType) noexcept
{
    loadMime();
    const std::string type { canonical(mimeType) };
    std::vector<std::string> out;

    if (auto it { m_parents.find(type) }; it != m_parents.end())
        out = it->second;

    const auto add = [&out](const std::string &t) {
        if (std::find(out.begin(), out.end(), t) == out.end())
            out.emplace_back(t);
    };

    if (type.starts_with("text/") && type != "text/plain")
        add("text/plain");

    if (!type.starts_with("inode/") && type != "application/octet-stream")
        add("application/octet-stream");

    return out;
}

bool XDGMimeDatabase::inherits(const std::string &mimeType, const std::string &base) noexcept
{
    const std::string target { canonical(base) };
    std::vector<std::string> queue { canonical(mimeType) };
    std::unordered_set<std::string> seen;

    while (!queue.empty())
    {
        const std::string type { queue.back() };
        queue.pop_back();

        if (type == target)
            return true;

        if (!seen.insert(type).second)
            continue;

        for (const auto &parent : parents(type))
            queue.emplace_back(parent);
    }

    return false;
}

std::vector<std::string> XDGMimeDatabase::iconNames(const std::string &mimeType) noexcept
{
    loadMime();
    const std::string type { canonical(mimeType) };
    std::vector<std::string> names;

    const auto add = [&names](const std::string &name) {
        if (!name.empty() && std::find(names.begin(), names.end(), name) == names.end())
            names.emplace_back(name);
    };

    if (type == "inode/directory")
        add("folder");

    if (auto it { m_icons.find(type) }; it != m_icons.end())
        add(it->second);

    std::string dashed { type };
    std::replace(dashed.begin(), dashed.end(), '/', '-');
    add(dashed);

    if (auto it { m_genericIcons.find(type) }; it != m_genericIcons.end())
        add(it->second);

    // The first parent with an icon of its own (e.g. a script → text-x-script).
    for (const auto &parent : parents(type))
    {
        if (parent == "application/octet-stream" || parent == "text/plain")
            continue;

        std::string p { parent };
        std::replace(p.begin(), p.end(), '/', '-');
        add(p);
    }

    const std::string media { type.substr(0, type.find('/')) };

    if (media == "text")
        add("text-x-generic");
    else if (media == "image" || media == "audio" || media == "video" || media == "font")
        add(media + "-x-generic");
    else if (media == "application")
        add("application-x-generic");

    add("unknown");
    add("text-x-generic");
    return names;
}

std::string XDGMimeDatabase::description(const std::string &mimeType) noexcept
{
    loadMime();
    const std::string type { canonical(mimeType) };

    if (auto it { m_descriptions.find(type) }; it != m_descriptions.end())
        return it->second;

    // <comment xml:lang="es">…</comment> in <dir>/<media>/<subtype>.xml
    std::string lang;

    for (const char *var : { "LC_ALL", "LC_MESSAGES", "LANG" })
        if (const char *v { getenv(var) }; v && v[0])
        {
            lang = v;
            break;
        }

    lang = lang.substr(0, lang.find_first_of("_.@"));

    std::string generic, localized;

    for (const auto &dir : mimeDirs())
    {
        std::ifstream in { dir / (type + ".xml") };

        if (!in)
            continue;

        std::stringstream ss;
        ss << in.rdbuf();
        const std::string xml { ss.str() };

        for (size_t pos { xml.find("<comment") }; pos != std::string::npos; pos = xml.find("<comment", pos + 1))
        {
            const size_t close { xml.find('>', pos) };
            const size_t end { xml.find("</comment>", pos) };

            if (close == std::string::npos || end == std::string::npos || end < close)
                break;

            const std::string attrs { xml.substr(pos, close - pos) };
            const std::string text { xml.substr(close + 1, end - close - 1) };

            if (attrs.find("xml:lang") == std::string::npos)
            {
                if (generic.empty()) generic = text;
            }
            else if (!lang.empty() && attrs.find("\"" + lang + "\"") != std::string::npos && localized.empty())
                localized = text;
        }

        break;
    }

    std::string result { !localized.empty() ? localized : !generic.empty() ? generic : type };
    m_descriptions[type] = result;
    return result;
}

void XDGMimeDatabase::loadApps() noexcept
{
    if (m_appsLoaded)
        return;

    m_appsLoaded = true;

    const fs::path home { m_kit.homeDir() };
    const fs::path configHome { EnvDir("XDG_CONFIG_HOME", home / ".config") };
    const fs::path dataHome { EnvDir("XDG_DATA_HOME", home / ".local/share") };

    std::vector<fs::path> configDirs;
    {
        const char *env { getenv("XDG_CONFIG_DIRS") };
        for (const auto &d : Split(env && env[0] ? env : "/etc/xdg", ':'))
            configDirs.emplace_back(d);
    }

    std::vector<std::string> desktops;
    if (const char *env { getenv("XDG_CURRENT_DESKTOP") })
        for (const auto &d : Split(env, ':'))
            desktops.emplace_back(Lower(d));

    // mimeapps.list files, highest priority first (desktop specific ones before the generic one).
    std::vector<fs::path> lists;
    const auto addLists = [&](const fs::path &dir) {
        for (const auto &d : desktops)
            lists.emplace_back(dir / (d + "-mimeapps.list"));
        lists.emplace_back(dir / "mimeapps.list");
    };

    addLists(configHome);
    for (const auto &d : configDirs) addLists(d);
    addLists(dataHome / "applications");
    for (const auto &d : m_kit.dataDirs()) addLists(d / "applications");

    const auto append = [](std::vector<std::string> &list, const std::string &id) {
        if (std::find(list.begin(), list.end(), id) == list.end())
            list.emplace_back(id);
    };

    for (const auto &file : lists)
    {
        std::error_code ec;

        if (!fs::is_regular_file(file, ec))
            continue;

        auto ini { XDGINI::LoadFile(file) };

        if (!ini)
            continue;

        if (auto group { ini->find("Default Applications") }; group != ini->end())
            for (const auto &[type, ids] : group->second)
                for (const auto &id : Split(ids, ';'))
                    append(m_defaults[type], id);

        if (auto group { ini->find("Added Associations") }; group != ini->end())
            for (const auto &[type, ids] : group->second)
                for (const auto &id : Split(ids, ';'))
                    append(m_associations[type], id);

        if (auto group { ini->find("Removed Associations") }; group != ini->end())
            for (const auto &[type, ids] : group->second)
                for (const auto &id : Split(ids, ';'))
                    append(m_removed[type], id);
    }

    // Then what the installed apps declare (cached by update-desktop-database).
    std::vector<fs::path> appDirs { dataHome / "applications" };
    for (const auto &d : m_kit.dataDirs()) appDirs.emplace_back(d / "applications");

    for (const auto &dir : appDirs)
    {
        std::error_code ec;
        const fs::path cache { dir / "mimeinfo.cache" };

        if (!fs::is_regular_file(cache, ec))
            continue;

        auto ini { XDGINI::LoadFile(cache) };

        if (!ini)
            continue;

        if (auto group { ini->find("MIME Cache") }; group != ini->end())
            for (const auto &[type, ids] : group->second)
                for (const auto &id : Split(ids, ';'))
                    append(m_associations[type], id);
    }
}

std::shared_ptr<const XDGDesktopEntry> XDGMimeDatabase::entry(const std::string &desktopId) noexcept
{
    std::string id { desktopId };

    if (id.ends_with(".desktop"))
        id.resize(id.size() - 8);

    return m_kit.desktopEntry(id);
}

std::vector<std::string> XDGMimeDatabase::associations(const std::string &mimeType, bool defaultsOnly) noexcept
{
    loadApps();
    std::vector<std::string> out;
    const auto &removed { m_removed[mimeType] };

    const auto add = [&](const std::vector<std::string> &ids) {
        for (const auto &id : ids)
            if (std::find(removed.begin(), removed.end(), id) == removed.end() &&
                std::find(out.begin(), out.end(), id) == out.end())
                out.emplace_back(id);
    };

    if (auto it { m_defaults.find(mimeType) }; it != m_defaults.end())
        add(it->second);

    if (!defaultsOnly)
        if (auto it { m_associations.find(mimeType) }; it != m_associations.end())
            add(it->second);

    return out;
}

std::vector<std::shared_ptr<const XDGDesktopEntry>> XDGMimeDatabase::apps(const std::string &mimeType) noexcept
{
    std::vector<std::shared_ptr<const XDGDesktopEntry>> out;
    std::unordered_set<std::string> seen, seenTypes;

    // Breadth-first over the type and its parents. The generic application/octet-stream only counts
    // when asked for directly (anything "opens" bytes).
    const std::string start { canonical(mimeType) };
    std::vector<std::string> level { start };

    for (int pass = 0; pass < 2; pass++) // Defaults first (on every level), then the rest
    {
        level = { start };
        seenTypes.clear();

        while (!level.empty())
        {
            std::vector<std::string> next;

            for (const auto &type : level)
            {
                if (!seenTypes.insert(type).second)
                    continue;

                if (type == "application/octet-stream" && start != type)
                    continue;

                for (const auto &id : associations(type, pass == 0))
                {
                    if (!seen.insert(id).second)
                        continue;

                    if (auto e { entry(id) })
                        out.emplace_back(std::move(e));
                }

                for (const auto &parent : parents(type))
                    next.emplace_back(parent);
            }

            level = std::move(next);
        }
    }

    return out;
}

std::shared_ptr<const XDGDesktopEntry> XDGMimeDatabase::defaultApp(const std::string &mimeType) noexcept
{
    auto list { apps(mimeType) };
    return list.empty() ? nullptr : list.front();
}
