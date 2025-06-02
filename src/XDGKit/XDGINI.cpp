#include <XDGKit/XDGUtils.h>
#include <XDGKit/XDGLog.h>
#include <XDGKit/XDGINI.h>
#include <cassert>
#include <cstring>
#include <fstream>
#include <sys/mman.h>
#include <unistd.h>

using namespace XDG;

std::shared_ptr<XDGINI> XDGINI::LoadFile(const std::filesystem::path &iniFile) noexcept
{
    std::shared_ptr<XDGINI> data { std::make_shared<XDGINI>() };
    std::ifstream file(iniFile);

    if (!file.is_open())
        return data;

    std::string line;
    std::string currentSection;

    while (std::getline(file, line))
    {
        // Trim leading/trailing whitespace
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        // Skip comments and blank lines
        if (line.empty() || line[0] == '#' || line[0] == ';')
            continue;

        // Handle section headers [SectionName]
        if (line.front() == '[' && line.back() == ']')
        {
            currentSection = line.substr(1, line.size() - 2); // Extract section name
            currentSection.erase(0, currentSection.find_first_not_of(" \t"));
            currentSection.erase(currentSection.find_last_not_of(" \t") + 1);

            if (!(*data).contains(currentSection))
            {
                data->m_nBytes += currentSection.size() + 1;
                (*data)[currentSection] = {};
            }
        }
        // Handle key-value pairs
        else if (!currentSection.empty())
        {
            size_t delimiterPos = line.find('=');
            if (delimiterPos != std::string::npos)
            {
                std::string key = line.substr(0, delimiterPos);
                std::string value = line.substr(delimiterPos + 1);

                // Trim whitespace from key and value
                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);

                if (!(*data)[currentSection].contains(key))
                {
                    // Add the key-value pair to the current section
                    (*data)[currentSection][key] = value;

                    data->m_nBytes += key.size() + value.size() + 2;
                }
            }
        }
    }

    file.close();
    return data;
}

static char *copyAndAdvanceDst(void *dst, const void *src, size_t bytes) noexcept
{
    memcpy(dst, src, bytes);
    return ((char*)dst) + bytes;
}

std::shared_ptr<XDG::XDGINIView> XDGINIView::LoadFile(const std::filesystem::path &iniFile) noexcept
{
    // FORMAT
    //
    // +u64: num of sections
    // FOREACH SECTION:
    //      +u64: num of items
    //      +str: section key
    //      FOREACH KEY PAIR:
    //          +str: item key
    //          +str: item val


    int fd;
    std::string line;
    std::string currentSection;
    uint64_t val;
    char *pos;
    XDGINIView *data { new XDGINIView() };
    auto &ref { *data };
    auto ini { XDGINI::LoadFile(iniFile) };

    if (ini->empty())
        goto failFile;

    // String + sections count + sub sections count
    data->m_mapSize = ini->nBytes() + sizeof(uint64_t) * (ini->size() + 1);

    // Setup tmp file
    data->m_tmp = tmpfile();
    if (!data->m_tmp)
        goto failFile;
    fd = fileno(data->m_tmp);
    if (ftruncate(fd, data->m_mapSize) == -1)
        goto failTmp;

    // Setup map
    data->m_map = mmap(NULL, data->m_mapSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data->m_map == MAP_FAILED)
    {
        data->m_map = nullptr;
        goto failTmp;
    }

    // Num of sections
    pos = (char*)data->m_map;
    val = ini->size();
    pos = copyAndAdvanceDst(pos, &val, sizeof(val));

    for (auto &section : *ini)
    {
        // Num of items
        val = section.second.size();
        memcpy(pos, &val, sizeof(val));
        pos += sizeof(val);

        // Section name
        char *sec { pos };      
        pos = copyAndAdvanceDst(pos, section.first.c_str(), section.first.size() + 1);

        for (auto &item : section.second)
        {
            char *key { pos };
            pos = copyAndAdvanceDst(pos, item.first.c_str(), item.first.size() + 1);

            char *value { pos };
            pos = copyAndAdvanceDst(pos, item.second.c_str(), item.second.size() + 1);

            ref[sec][key] = value;
        }
    }

    assert(pos == (char*)data->m_map + data->m_mapSize);

    return std::shared_ptr<XDGINIView>(data);

failTmp:
    fclose(data->m_tmp);
    data->m_tmp = nullptr;
failFile:
    return std::shared_ptr<XDGINIView>(data);
}

std::shared_ptr<XDGINIView> XDGINIView::FromData(char *pos, size_t size) noexcept
{
    XDGINIView *data { new XDGINIView() };
    auto &ref { *data };
    char *end { pos + size };
    uint64_t nSections;
    uint64_t nItems;
    const char *error { "Unknown error." };

    if (!(pos = XDGUtils::readSafeAndAdvancePos(&nSections, pos, end, sizeof(nSections))))
    {
        error = "Failed to read the number of sections.";
        goto fail;
    }

    ref.reserve(std::min(nSections, 1000UL));

    for (uint64_t i = 0; i < nSections; i++)
    {
        if (!(pos = XDGUtils::readSafeAndAdvancePos(&nItems, pos, end, sizeof(nItems))))
        {
            error = "Failed to read number of items";
            goto fail;
        }

        char *sectionName { pos };
        if (!(pos = XDGUtils::advanceStrPosSafe(pos, end)))
        {
            error = "Failed to read section name.";
            goto fail;
        }

        auto &sec { ref[sectionName] };

        sec.reserve(std::min(nItems, 1000UL));

        for (uint64_t j = 0; j < nItems; j++)
        {
            char *key { pos };
            if (!(pos = XDGUtils::advanceStrPosSafe(pos, end)))
            {
                error = "Failed to read item key.";
                goto fail;
            }

            char *value { pos };
            if (!(pos = XDGUtils::advanceStrPosSafe(pos, end)))
            {
                error = "Failed to read item value.";
                goto fail;
            }

            sec[key] = value;
        }
    }

    return std::shared_ptr<XDGINIView>(data);
fail:
    XDGLog::error("[XDGINIView::FromData] Failed to parse serialized INI file (size = %zu): %s", size, error);
    data->clear();
    return std::shared_ptr<XDGINIView>(data);
}

void XDGINIView::clear() noexcept
{
    std::unordered_map<std::string_view, std::unordered_map<std::string_view, std::string_view>>::clear();

    if (m_map)
    {
        munmap(m_map, m_mapSize);
        m_map = nullptr;
        m_mapSize = 0;
    }

    if (m_tmp)
    {
        fclose(m_tmp);
        m_tmp = nullptr;
    }
}

XDGINIView::~XDGINIView()
{
    clear();
}
