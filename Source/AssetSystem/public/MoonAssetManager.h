#pragma once

#include "Source/AssetSystem/public/MoonAssetFormat.h"
#include "Source/AssetSystem/public/MoonAssetHandle.h"
#include "Source/AssetSystem/public/MoonAssetRegistry.h"
#include "Source/AssetSystem/public/MoonAssetDependencyGraph.h"

#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

namespace AssetSystem
{
    // ============================================================
    // IAsset - 资产基类接口
    // ============================================================
    class IAsset
    {
    public:
        virtual ~IAsset() = default;
        virtual AssetType GetAssetType() const = 0;
        virtual MoonAssetGUID GetGUID() const = 0;
        virtual size_t GetMemorySize() const = 0;
        virtual bool Deserialize(const uint8_t* processedData, size_t processedSize) = 0;
    };

    // ============================================================
    // AssetLoadCallback
    // ============================================================
    using AssetLoadCallback = std::function<void(IAsset* asset, bool success)>;

    // ============================================================
    // AssetManager - 单例
    // ============================================================
    class AssetManager
    {
    public:
        static AssetManager& Get();

        void Initialize(const std::string& projectRoot);
        void Shutdown();

        // 同步加载
        IAsset* LoadSync(MoonAssetGUID guid);
        IAsset* LoadSync(const std::string& path);

        // 异步加载（预留接口）
        void LoadAsync(MoonAssetGUID guid, AssetLoadCallback callback);

        // 查询
        bool IsLoaded(MoonAssetGUID guid) const;
        IAsset* GetLoadedAsset(MoonAssetGUID guid) const;

        // 卸载
        void UnloadAsset(MoonAssetGUID guid);
        void UnloadAllAssets();

        // 注册表
        AssetRegistry& GetRegistry() { return m_registry; }
        const AssetRegistry& GetRegistry() const { return m_registry; }

        // 依赖图
        AssetDependencyGraph& GetDependencyGraph() { return m_dependencyGraph; }

        // 注册资产工厂
        using AssetFactoryFunc = std::function<std::unique_ptr<IAsset>()>;
        void RegisterAssetFactory(AssetType type, AssetFactoryFunc factory);

    private:
        AssetManager() = default;
        ~AssetManager() = default;
        AssetManager(const AssetManager&) = delete;
        AssetManager& operator=(const AssetManager&) = delete;

        std::unique_ptr<IAsset> LoadAssetFromFile(const std::string& filePath);

        AssetRegistry m_registry;
        AssetDependencyGraph m_dependencyGraph;

        struct CachedAsset
        {
            std::unique_ptr<IAsset> asset;
            uint32_t refCount = 0;
        };
        std::unordered_map<MoonAssetGUID, CachedAsset, MoonAssetGUID::Hash> m_cache;

        struct AssetTypeHash {
            size_t operator()(AssetType type) const {
                return std::hash<uint32_t>{}(static_cast<uint32_t>(type));
            }
        };
        std::unordered_map<AssetType, AssetFactoryFunc, AssetTypeHash> m_factories;

        std::string m_projectRoot;
        bool m_initialized = false;
    };
}
