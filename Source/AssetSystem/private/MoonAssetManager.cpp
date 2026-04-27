#include "Source/AssetSystem/public/MoonAssetManager.h"

#include "Source/Logging/public/LogSystem.h"

#include <filesystem>
#include <fstream>

namespace AssetSystem
{
    AssetManager& AssetManager::Get()
    {
        static AssetManager instance;
        return instance;
    }

    void AssetManager::Initialize(const std::string& projectRoot)
    {
        m_projectRoot = projectRoot;
        m_initialized = true;

        // Scan for .moonasset files
        std::string assetsPath = projectRoot + "/Resources/MoonAssets";
        if (std::filesystem::exists(assetsPath))
        {
            m_registry.ScanDirectory(assetsPath);
        }

        MOON_LOG("AssetManager initialized. Found " + std::to_string(m_registry.GetAllAssets().size()) + " assets.");
    }

    void AssetManager::Shutdown()
    {
        UnloadAllAssets();
        m_initialized = false;
    }

    IAsset* AssetManager::LoadSync(MoonAssetGUID guid)
    {
        if (!guid.IsValid())
            return nullptr;

        // Check cache
        auto it = m_cache.find(guid);
        if (it != m_cache.end())
        {
            it->second.refCount++;
            return it->second.asset.get();
        }

        // Find file path
        std::string filePath = m_registry.ResolveGUIDToPath(guid);
        if (filePath.empty())
        {
            MOON_LOG("Asset not found in registry: " + guid.ToString());
            return nullptr;
        }

        // Load from file
        auto asset = LoadAssetFromFile(filePath);
        if (!asset)
        {
            MOON_LOG("Failed to load asset: " + filePath);
            return nullptr;
        }

        // Cache
        CachedAsset cached;
        cached.asset = std::move(asset);
        cached.refCount = 1;
        IAsset* result = cached.asset.get();
        m_cache[guid] = std::move(cached);

        // Register dependencies
        MoonAssetMeta meta;
        {
            std::ifstream file(filePath, std::ios::binary);
            if (file.is_open())
            {
                MoonAssetFileHeader header;
                file.read(reinterpret_cast<char*>(&header), sizeof(header));
                std::vector<char> metaJson(header.metaSize + 1, '\0');
                file.seekg(header.metaOffset);
                file.read(metaJson.data(), header.metaSize);
                meta.FromJson(std::string(metaJson.data(), header.metaSize));
            }
        }
        for (const auto& dep : meta.dependencies)
        {
            if (dep.refType == AssetReferenceType::Hard)
            {
                m_dependencyGraph.AddDependency(guid, dep.guid);
            }
        }

        return result;
    }

    IAsset* AssetManager::LoadSync(const std::string& path)
    {
        MoonAssetGUID guid = m_registry.ResolvePathToGUID(path);
        if (!guid.IsValid())
        {
            // Try to load from absolute/relative path
            std::ifstream file(path, std::ios::binary);
            if (file.is_open())
            {
                MoonAssetFileHeader header;
                file.read(reinterpret_cast<char*>(&header), sizeof(header));
                if (header.IsValidMagic())
                {
                    std::vector<char> metaJson(header.metaSize + 1, '\0');
                    file.seekg(header.metaOffset);
                    file.read(metaJson.data(), header.metaSize);
                    MoonAssetMeta meta;
                    if (meta.FromJson(std::string(metaJson.data(), header.metaSize)))
                    {
                        guid = meta.guid;
                        // Register if not already in registry
                        AssetRegistryEntry entry;
                        entry.guid = guid;
                        entry.type = meta.assetType;
                        entry.filePath = path;
                        entry.displayName = meta.displayName;
                        m_registry.RegisterAsset(entry);
                    }
                }
            }
        }
        return LoadSync(guid);
    }

    void AssetManager::LoadAsync(MoonAssetGUID guid, AssetLoadCallback callback)
    {
        // For now, just load synchronously and call callback
        IAsset* asset = LoadSync(guid);
        if (callback)
            callback(asset, asset != nullptr);
    }

    bool AssetManager::IsLoaded(MoonAssetGUID guid) const
    {
        return m_cache.find(guid) != m_cache.end();
    }

    IAsset* AssetManager::GetLoadedAsset(MoonAssetGUID guid) const
    {
        auto it = m_cache.find(guid);
        if (it != m_cache.end())
            return it->second.asset.get();
        return nullptr;
    }

    void AssetManager::UnloadAsset(MoonAssetGUID guid)
    {
        auto it = m_cache.find(guid);
        if (it != m_cache.end())
        {
            if (it->second.refCount > 0)
                it->second.refCount--;

            if (it->second.refCount == 0)
            {
                m_cache.erase(it);
            }
        }
    }

    void AssetManager::UnloadAllAssets()
    {
        m_cache.clear();
    }

    void AssetManager::RegisterAssetFactory(AssetType type, AssetFactoryFunc factory)
    {
        m_factories[type] = factory;
    }

    std::unique_ptr<IAsset> AssetManager::LoadAssetFromFile(const std::string& filePath)
    {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open())
        {
            MOON_LOG("Failed to open asset file: " + filePath);
            return nullptr;
        }

        MoonAssetFileHeader header;
        file.read(reinterpret_cast<char*>(&header), sizeof(header));
        if (!file.good() || !header.IsValidMagic())
        {
            MOON_LOG("Invalid asset file header: " + filePath);
            return nullptr;
        }

        if (!header.IsValidVersion())
        {
            MOON_LOG("Unsupported asset version in: " + filePath);
            return nullptr;
        }

        // Read meta
        std::vector<char> metaJson(header.metaSize + 1, '\0');
        file.seekg(header.metaOffset);
        file.read(metaJson.data(), header.metaSize);

        MoonAssetMeta meta;
        if (!meta.FromJson(std::string(metaJson.data(), header.metaSize)))
        {
            MOON_LOG("Failed to parse asset meta: " + filePath);
            return nullptr;
        }

        // Read processed data
        std::vector<uint8_t> processedData(header.processedDataSize);
        if (header.processedDataSize > 0)
        {
            file.seekg(header.processedDataOffset);
            file.read(reinterpret_cast<char*>(processedData.data()), header.processedDataSize);
        }

        // Find factory and create asset
        AssetType assetType = static_cast<AssetType>(header.assetType);
        auto factoryIt = m_factories.find(assetType);
        if (factoryIt == m_factories.end())
        {
            MOON_LOG("No factory registered for asset type: " + std::to_string(static_cast<uint32_t>(assetType)));
            return nullptr;
        }

        auto asset = factoryIt->second();
        if (!asset)
        {
            MOON_LOG("Factory failed to create asset");
            return nullptr;
        }

        // Deserialize processed data into asset
        if (!processedData.empty())
        {
            if (!asset->Deserialize(processedData.data(), processedData.size()))
            {
                MOON_LOG("Failed to deserialize asset processed data");
                return nullptr;
            }
        }

        return asset;
    }
}
