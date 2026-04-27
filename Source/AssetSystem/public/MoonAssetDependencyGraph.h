#pragma once

#include "Source/AssetSystem/public/MoonAssetFormat.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace AssetSystem
{
    class AssetDependencyGraph
    {
    public:
        void AddDependency(MoonAssetGUID asset, MoonAssetGUID dependency);
        void RemoveDependency(MoonAssetGUID asset, MoonAssetGUID dependency);

        std::vector<MoonAssetGUID> GetDirectDependencies(MoonAssetGUID asset) const;
        std::vector<MoonAssetGUID> GetAllDependencies(MoonAssetGUID asset) const;
        std::vector<MoonAssetGUID> GetReferencers(MoonAssetGUID asset) const;

        bool HasCircularDependency(MoonAssetGUID startAsset) const;
        std::vector<MoonAssetGUID> GetLoadOrder(const std::vector<MoonAssetGUID>& assets) const;

    private:
        std::unordered_map<MoonAssetGUID, std::unordered_set<MoonAssetGUID, MoonAssetGUID::Hash>, MoonAssetGUID::Hash> m_dependencies;
        std::unordered_map<MoonAssetGUID, std::unordered_set<MoonAssetGUID, MoonAssetGUID::Hash>, MoonAssetGUID::Hash> m_referencers;
    };
}
