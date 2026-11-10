#include <CZ/XDG/XDGTrash.h>
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <pwd.h>
#include <sstream>
#include <unistd.h>
#include <sys/stat.h>

using namespace CZ;
namespace fs = std::filesystem;

fs::path XDGTrash::HomeDir() noexcept
{
    if (const char *env { getenv("XDG_DATA_HOME") }; env && env[0] == '/')
        return fs::path(env) / "Trash";

    if (const char *home { getenv("HOME") }; home && home[0] == '/')
        return fs::path(home) / ".local/share/Trash";

    if (const passwd *pw { getpwuid(getuid()) }; pw && pw->pw_dir)
        return fs::path(pw->pw_dir) / ".local/share/Trash";

    return {};
}

std::string XDGTrash::EncodePath(const std::string &path) noexcept
{
    static constexpr char hex[] { "0123456789ABCDEF" };
    std::string out;

    for (const unsigned char c : path)
    {
        if (std::isalnum(c) || std::strchr("/-._~", c))
            out += char(c);
        else
        {
            out += '%';
            out += hex[c >> 4];
            out += hex[c & 0xF];
        }
    }

    return out;
}

XDGTrash::Result XDGTrash::MoveToTrash(const fs::path &path, std::string *error) noexcept
{
    const auto fail = [error](const std::string &msg) {
        if (error) *error = msg;
        return Result::Failed;
    };

    const fs::path trash { HomeDir() };

    if (trash.empty())
        return fail("no home directory");

    const fs::path files { trash / "files" }, info { trash / "info" };
    std::error_code ec;
    fs::create_directories(files, ec);
    fs::create_directories(info, ec);

    // The trash isn't trashed.
    if (fs::equivalent(path, trash, ec) || fs::equivalent(path, files, ec))
        return fail("it's the trash itself");

    // Absolute path for the .trashinfo (without resolving symlinks: the link itself is trashed).
    fs::path absolute { fs::absolute(path, ec).lexically_normal() };

    if (absolute.has_filename() == false && absolute.has_parent_path() && absolute != absolute.root_path())
        absolute = absolute.parent_path(); // Trailing slash

    // Never the root, the home folder or a mount point (e.g. a disk dragged from the desktop).
    struct stat st, parentSt;

    const char *home { getenv("HOME") };

    if (absolute == absolute.root_path() || fs::equivalent(absolute, trash.parent_path().parent_path().parent_path(), ec) ||
        (home && home[0] == '/' && fs::equivalent(absolute, home, ec)))
        return fail("refusing to trash the root or home folder");

    if (lstat(absolute.c_str(), &st) != 0)
        return fail(strerror(errno));

    if (S_ISDIR(st.st_mode) && stat(absolute.parent_path().c_str(), &parentSt) == 0 && parentSt.st_dev != st.st_dev)
        return fail("it's a mount point");

    // Reserve a unique name through its .trashinfo (O_EXCL), as the spec requires.
    const std::string stem { absolute.filename().string() };
    std::string name { stem };
    int fd { -1 };

    for (int i = 2; i < 100000; i++)
    {
        fd = ::open((info / (name + ".trashinfo")).c_str(), O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0600);

        if (fd >= 0 || errno != EEXIST)
            break;

        name = stem + "." + std::to_string(i);
    }

    if (fd < 0)
        return fail(std::string("can't create the .trashinfo file: ") + strerror(errno));

    char date[32];
    const time_t now { time(nullptr) };
    tm local;
    localtime_r(&now, &local);
    strftime(date, sizeof(date), "%Y-%m-%dT%H:%M:%S", &local);

    const std::string data { "[Trash Info]\nPath=" + EncodePath(absolute.string()) + "\nDeletionDate=" + date + "\n" };
    const bool written { write(fd, data.data(), data.size()) == ssize_t(data.size()) };
    close(fd);

    if (written && rename(absolute.c_str(), (files / name).c_str()) == 0)
        return Result::Trashed;

    const int err { written ? errno : EIO };
    fs::remove(info / (name + ".trashinfo"), ec);

    if (err == EXDEV)
        return Result::OtherDevice;

    return fail(strerror(err));
}

bool XDGTrash::IsEmpty() noexcept
{
    std::error_code ec;
    fs::directory_iterator it { HomeDir() / "files", ec };
    return ec || it == fs::directory_iterator();
}

std::vector<fs::path> XDGTrash::ParseUriList(const std::string &uriList) noexcept
{
    std::vector<fs::path> paths;
    std::istringstream lines { uriList };
    std::string line;

    while (std::getline(lines, line))
    {
        while (!line.empty() && (line.back() == '\r' || line.back() == ' '))
            line.pop_back();

        if (line.empty() || line.front() == '#' || !line.starts_with("file://"))
            continue;

        // file://host/path → /path
        const auto slash { line.find('/', 7) };

        if (slash == std::string::npos)
            continue;

        std::string decoded;

        for (size_t i = slash; i < line.size(); i++)
        {
            if (line[i] == '%' && i + 2 < line.size() && std::isxdigit((unsigned char)line[i + 1]) && std::isxdigit((unsigned char)line[i + 2]))
            {
                decoded += char(std::strtol(line.substr(i + 1, 2).c_str(), nullptr, 16));
                i += 2;
            }
            else
                decoded += line[i];
        }

        paths.emplace_back(decoded);
    }

    return paths;
}
