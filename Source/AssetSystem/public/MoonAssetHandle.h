#pragma once

#include "Source/AssetSystem/public/MoonAssetFormat.h"

#include <string>

namespace AssetSystem
{
    class AssetManager;
    class IAsset;

    // ============================================================
    // AssetHandle - 运行时轻量引用句柄
    // ============================================================
    class AssetHandle
    {
    public:
        AssetHandle() = default;
        explicit AssetHandle(MoonAssetGUID guid) : m_guid(guid) {}

        bool IsValid() const { return m_guid.IsValid(); }
        MoonAssetGUID GetGUID() const { return m_guid; }

        bool operator==(const AssetHandle& other) const { return m_guid == other.m_guid; }
        bool operator!=(const AssetHandle& other) const { return !(*this == other); }
        bool operator<(const AssetHandle& other) const { return m_guid < other.m_guid; }

        struct Hash
        {
            size_t operator()(const AssetHandle& handle) const
            {
                return MoonAssetGUID::Hash{}(handle.m_guid);
            }
        };

    private:
        MoonAssetGUID m_guid;
    };

    // ============================================================
    // SoftAssetPath - 软引用路径（延迟解析）
    // ============================================================
    struct SoftAssetPath
    {
        std::string path;
        MoonAssetGUID guid;

        bool HasGUID() const { return guid.IsValid(); }
        bool IsValid() const { return !path.empty() || guid.IsValid(); }
    };
}
