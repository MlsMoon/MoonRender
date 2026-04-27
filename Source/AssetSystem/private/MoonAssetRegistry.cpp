#include "Source/AssetSystem/public/MoonAssetRegistry.h"

#include "Source/Logging/public/LogSystem.h"

#include <filesystem>
#include <fstream>

namespace AssetSystem
{
    void AssetRegistry::ScanDirectory(const std::string& rootPath)
    {
        if (!std::filesystem::exists(rootPath))
            return;

        for (const auto& entry : std::filesystem::recursive_directory_iterator(rootPath))
        {
            if (!entry.is_regular_file())
                continue;

            if (entry.path().extension() != ".moonasset")
                continue;

            AssetRegistryEntry regEntry;
            std::string filePath = entry.path().string();
            if (TryReadAssetHeader(filePath, regEntry))
            {
                RegisterAsset(regEntry);
            }
        }
    }

    void AssetRegistry::RegisterAsset(const AssetRegistryEntry& entry)
    {
        m_guidMap[entry.guid] = entry;
        m_pathMap[entry.filePath] = entry.guid;
    }

    void AssetRegistry::UnregisterAsset(MoonAssetGUID guid)
    {
        auto it = m_guidMap.find(guid);
        if (it != m_guidMap.end())
        {
            m_pathMap.erase(it->second.filePath);
            m_guidMap.erase(it);
        }
    }

    std::optional<AssetRegistryEntry> AssetRegistry::FindByGUID(MoonAssetGUID guid) const
    {
        auto it = m_guidMap.find(guid);
        if (it != m_guidMap.end())
            return it->second;
        return std::nullopt;
    }

    std::optional<AssetRegistryEntry> AssetRegistry::FindByPath(const std::string& path) const
    {
        auto it = m_pathMap.find(path);
        if (it != m_pathMap.end())
            return FindByGUID(it->second);
        return std::nullopt;
    }

    std::vector<AssetRegistryEntry> AssetRegistry::FindByType(AssetType type) const
    {
        std::vector<AssetRegistryEntry> result;
        for (const auto& [guid, entry] : m_guidMap)
        {
            if (entry.type == type)
                result.push_back(entry);
        }
        return result;
    }

    std::string AssetRegistry::ResolveGUIDToPath(MoonAssetGUID guid) const
    {
        auto entry = FindByGUID(guid);
        return entry ? entry->filePath : "";
    }

    MoonAssetGUID AssetRegistry::ResolvePathToGUID(const std::string& path) const
    {
        auto it = m_pathMap.find(path);
        if (it != m_pathMap.end())
            return it->second;
        return MoonAssetGUID::Invalid();
    }

    std::vector<AssetRegistryEntry> AssetRegistry::GetAllAssets() const
    {
        std::vector<AssetRegistryEntry> result;
        result.reserve(m_guidMap.size());
        for (const auto& [guid, entry] : m_guidMap)
            result.push_back(entry);
        return result;
    }

    bool AssetRegistry::TryReadAssetHeader(const std::string& filePath, AssetRegistryEntry& outEntry)
    {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open())
            return false;

        MoonAssetFileHeader header;
        file.read(reinterpret_cast<char*>(&header), sizeof(header));
        if (!file.good())
            return false;

        if (!header.IsValidMagic())
            return false;

        // Read meta JSON
        std::vector<char> metaJson(header.metaSize + 1, '\0');
        file.seekg(header.metaOffset);
        file.read(metaJson.data(), header.metaSize);

        MoonAssetMeta meta;
        if (!meta.FromJson(std::string(metaJson.data(), header.metaSize)))
            return false;

        outEntry.guid = meta.guid;
        outEntry.type = meta.assetType;
        outEntry.filePath = filePath;
        outEntry.displayName = meta.displayName;
        outEntry.assetVersion = header.assetVersion;
        outEntry.dependencyCount = meta.dependencies.size();

        // Get file size and last modified time
        std::error_code ec;
        auto fsize = std::filesystem::file_size(filePath, ec);
        if (!ec)
            outEntry.fileSize = static_cast<uint64_t>(fsize);

        auto lwt = std::filesystem::last_write_time(filePath, ec);
        if (!ec)
        {
            // Simplified: store 0 for now, cross-platform file_time_type conversion is tricky
            outEntry.lastModified = 0;
        }

        return true;
    }
}
