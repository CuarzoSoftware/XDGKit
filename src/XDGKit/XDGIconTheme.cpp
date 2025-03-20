#include <XDGKit/XDGIconTheme.h>
#include <XDGKit/XDGIconDirectory.h>

using namespace XDG;

XDGIconTheme::XDGIconTheme(XDGKit &kit) noexcept :
    m_kit(kit)
{
    m_iconDirectories.reserve(64);
    m_scaledIconDirectories.reserve(64);
}

void XDGIconTheme::initAllIconsDir() const noexcept
{
    m_initialized = true;
    initIconsDir(m_iconDirNames, XDGIconDirectory::Type::Normal);
    initIconsDir(m_scaledIconDirNames, XDGIconDirectory::Type::Scaled);
}

void XDGIconTheme::initIconsDir(const std::vector<std::string> &iconDirs, XDGIconDirectory::Type type) const noexcept
{
    auto *iconsDirVec { type == XDGIconDirectory::Type::Scaled ? &m_scaledIconDirectories : &m_iconDirectories };
    XDGIconDirectory IcD { (*((XDGIconTheme*)this)) };
    IcD.m_type = type;

    for (const auto &iconDir : iconDirs)
    {
        const auto &iconDirData = indexData().find(iconDir);
        if (iconDirData == indexData().end()) continue;

        const auto &size { iconDirData->second.find("Size") };
        if (size == iconDirData->second.end()) continue;

        // Size
        try { IcD.m_size = std::stoi(size->second); }
        catch (const std::exception &) { IcD.m_size = -1; }
        if (IcD.m_size < 0) continue;

        // Scale
        IcD.m_scale = 1;
        const auto &scale { iconDirData->second.find("Scale") };
        if (scale != iconDirData->second.end())
        {
            try { IcD.m_scale = std::stoi(scale->second); }
            catch (const std::exception &) { IcD.m_scale = -1; }
            if (IcD.m_scale <= 0) IcD.m_scale = 1;
        }

        // Context
        IcD.m_context = XDGIconDirectory::Context::NoContext;
        const auto &context { iconDirData->second.find("Context") };
        if (context != iconDirData->second.end())
        {
            if (context->second == "Actions")
                IcD.m_context = XDGIconDirectory::Context::Actions;
            else if (context->second == "Devices")
                IcD.m_context = XDGIconDirectory::Context::Devices;
            else if (context->second == "FileSystems")
                IcD.m_context = XDGIconDirectory::Context::FileSystems;
            else if (context->second == "MimeTypes")
                IcD.m_context = XDGIconDirectory::Context::MimeTypes;
        }

        // Size Type
        IcD.m_sizeType = XDGIconDirectory::SizeType::Threshold;
        const auto &sizeType { iconDirData->second.find("Type") };
        if (sizeType != iconDirData->second.end())
        {
            if (sizeType->second == "Fixed")
                IcD.m_sizeType = XDGIconDirectory::SizeType::Fixed;
            else if (sizeType->second == "Scalable")
                IcD.m_sizeType = XDGIconDirectory::SizeType::Scalable;
        }

        // Max Size
        IcD.m_maxSize = IcD.m_size;
        const auto &maxSize { iconDirData->second.find("MaxSize") };
        if (maxSize != iconDirData->second.end())
        {
            try { IcD.m_maxSize = std::stoi(maxSize->second); }
            catch (const std::exception &) { IcD.m_maxSize = -1; }
            if (IcD.m_maxSize <= 0 || IcD.m_maxSize < IcD.m_size) IcD.m_maxSize = IcD.m_size;
        }

        // Min Size
        IcD.m_minSize = IcD.m_size;
        const auto &minSize { iconDirData->second.find("MinSize") };
        if (minSize != iconDirData->second.end())
        {
            try { IcD.m_minSize = std::stoi(minSize->second); }
            catch (const std::exception &) { IcD.m_minSize = -1; }
            if (IcD.m_minSize < 0 || IcD.m_minSize > IcD.m_size) IcD.m_minSize = IcD.m_size;
        }

        // Threshold
        IcD.m_threshold = 2;
        const auto &threshold { iconDirData->second.find("Threshold") };
        if (threshold != iconDirData->second.end())
        {
            try { IcD.m_threshold = std::stoi(threshold->second); }
            catch (const std::exception &) { IcD.m_threshold = -1; }
            if (IcD.m_threshold < 0) IcD.m_threshold = 2;
        }

        for (const auto &themeDir : dirs())
        {
            IcD.m_dir = themeDir / iconDir;

            if (std::filesystem::is_directory(IcD.m_dir))
            {
                auto &newIconDir = iconsDirVec->emplace_back(std::shared_ptr<XDGIconDirectory>(new XDGIconDirectory(IcD)));
                newIconDir->initIcons();
            }
        }
    }
}
