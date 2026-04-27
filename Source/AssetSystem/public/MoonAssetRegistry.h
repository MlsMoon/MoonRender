#pragma once

#include "Source/AssetSystem/public/MoonAssetFormat.h"

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace AssetSystem
{
    struct AssetRegistryEntry
    {
        MoonAssetGUID guid;
        AssetType type = AssetType::Unknown;
        std::string filePath;
        std::string displayName;
        uint64_t fileSize = 0;
        uint64_t lastModified = 0;
        uint32_t assetVersion = 0;
        size_t dependencyCount = 0;
    };

    class AssetRegistry
    {
    public:
        void ScanDirectory(const std::string& rootPath);
        void RegisterAsset(const AssetRegistryEntry& entry);
        void UnregisterAsset(MoonAssetGUID guid);

        std::optional<AssetRegistryEntry> FindByGUID(MoonAssetGUID guid) const;
        std::optional<AssetRegistryEntry> FindByPath(const std::string& path) const;
        std::vector<AssetRegistryEntry> FindByType(AssetType type) const;

        std::string ResolveGUIDToPath(MoonAssetGUID guid) const;
        MoonAssetGUID ResolvePathToGUID(const std::string& path) const;

        std::vector<AssetRegistryEntry> GetAllAssets() const;

    private:
        bool TryReadAssetHeader(const std::string& filePath, AssetRegistryEntry& outEntry);

        std::unordered_map<MoonAssetGUID, AssetRegistryEntry, MoonAssetGUID::Hash> m_guidMap;
        std::unordered_map<std::string, MoonAssetGUID> m_pathMap;
    };
}
