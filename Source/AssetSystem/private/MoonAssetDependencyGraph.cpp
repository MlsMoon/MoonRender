#include "Source/AssetSystem/public/MoonAssetDependencyGraph.h"

namespace AssetSystem
{
    void AssetDependencyGraph::AddDependency(MoonAssetGUID asset, MoonAssetGUID dependency)
    {
        m_dependencies[asset].insert(dependency);
        m_referencers[dependency].insert(asset);
    }

    void AssetDependencyGraph::RemoveDependency(MoonAssetGUID asset, MoonAssetGUID dependency)
    {
        auto depIt = m_dependencies.find(asset);
        if (depIt != m_dependencies.end())
        {
            depIt->second.erase(dependency);
            if (depIt->second.empty())
                m_dependencies.erase(depIt);
        }

        auto refIt = m_referencers.find(dependency);
        if (refIt != m_referencers.end())
        {
            refIt->second.erase(asset);
            if (refIt->second.empty())
                m_referencers.erase(refIt);
        }
    }

    std::vector<MoonAssetGUID> AssetDependencyGraph::GetDirectDependencies(MoonAssetGUID asset) const
    {
        std::vector<MoonAssetGUID> result;
        auto it = m_dependencies.find(asset);
        if (it != m_dependencies.end())
        {
            result.reserve(it->second.size());
            for (const auto& dep : it->second)
                result.push_back(dep);
        }
        return result;
    }

    std::vector<MoonAssetGUID> AssetDependencyGraph::GetAllDependencies(MoonAssetGUID asset) const
    {
        std::vector<MoonAssetGUID> result;
        std::unordered_set<MoonAssetGUID, MoonAssetGUID::Hash> visited;
        std::vector<MoonAssetGUID> stack = { asset };

        while (!stack.empty())
        {
            MoonAssetGUID current = stack.back();
            stack.pop_back();

            auto it = m_dependencies.find(current);
            if (it != m_dependencies.end())
            {
                for (const auto& dep : it->second)
                {
                    if (visited.insert(dep).second)
                    {
                        result.push_back(dep);
                        stack.push_back(dep);
                    }
                }
            }
        }
        return result;
    }

    std::vector<MoonAssetGUID> AssetDependencyGraph::GetReferencers(MoonAssetGUID asset) const
    {
        std::vector<MoonAssetGUID> result;
        auto it = m_referencers.find(asset);
        if (it != m_referencers.end())
        {
            result.reserve(it->second.size());
            for (const auto& ref : it->second)
                result.push_back(ref);
        }
        return result;
    }

    bool AssetDependencyGraph::HasCircularDependency(MoonAssetGUID startAsset) const
    {
        std::unordered_set<MoonAssetGUID, MoonAssetGUID::Hash> visited;
        std::unordered_set<MoonAssetGUID, MoonAssetGUID::Hash> recStack;

        std::function<bool(MoonAssetGUID)> dfs = [&](MoonAssetGUID node) -> bool
        {
            visited.insert(node);
            recStack.insert(node);

            auto it = m_dependencies.find(node);
            if (it != m_dependencies.end())
            {
                for (const auto& dep : it->second)
                {
                    if (visited.find(dep) == visited.end())
                    {
                        if (dfs(dep))
                            return true;
                    }
                    else if (recStack.find(dep) != recStack.end())
                    {
                        return true;
                    }
                }
            }

            recStack.erase(node);
            return false;
        };

        return dfs(startAsset);
    }

    std::vector<MoonAssetGUID> AssetDependencyGraph::GetLoadOrder(const std::vector<MoonAssetGUID>& assets) const
    {
        // Kahn's algorithm for topological sort
        std::unordered_map<MoonAssetGUID, int, MoonAssetGUID::Hash> inDegree;
        for (const auto& asset : assets)
            inDegree[asset] = 0;

        // Count in-degrees
        for (const auto& asset : assets)
        {
            auto it = m_dependencies.find(asset);
            if (it != m_dependencies.end())
            {
                for (const auto& dep : it->second)
                {
                    if (inDegree.find(dep) != inDegree.end())
                        inDegree[asset]++;
                }
            }
        }

        std::vector<MoonAssetGUID> queue;
        for (const auto& [asset, degree] : inDegree)
        {
            if (degree == 0)
                queue.push_back(asset);
        }

        std::vector<MoonAssetGUID> result;
        while (!queue.empty())
        {
            MoonAssetGUID current = queue.back();
            queue.pop_back();
            result.push_back(current);

            auto refIt = m_referencers.find(current);
            if (refIt != m_referencers.end())
            {
                for (const auto& dependent : refIt->second)
                {
                    auto degIt = inDegree.find(dependent);
                    if (degIt != inDegree.end())
                    {
                        degIt->second--;
                        if (degIt->second == 0)
                            queue.push_back(dependent);
                    }
                }
            }
        }

        return result;
    }
}
