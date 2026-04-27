#pragma once

#include <cstdint>
#include <random>
#include <string>
#include <vector>

namespace AssetSystem
{
    // ============================================================
    // 魔数与版本
    // ============================================================
    inline constexpr uint32_t kMoonAssetMagic = 0x4E4F4F4D; // "MOON"
    inline constexpr uint16_t kMoonAssetFileFormatVersion = 1;
    inline constexpr uint16_t kMeshAssetVersion = 1;
    inline constexpr uint32_t kMoonAssetAlignment = 16;

    // ============================================================
    // 资产类型
    // ============================================================
    enum class AssetType : uint32_t
    {
        Unknown = 0,
        Mesh = 1,
        Texture = 2,
        Shader = 3,
        Material = 4,
        Font = 5,
        Scene = 6,
        Count
    };

    inline const char* AssetTypeToString(AssetType type)
    {
        switch (type)
        {
        case AssetType::Mesh:     return "Mesh";
        case AssetType::Texture:  return "Texture";
        case AssetType::Shader:   return "Shader";
        case AssetType::Material: return "Material";
        case AssetType::Font:     return "Font";
        case AssetType::Scene:    return "Scene";
        default:                  return "Unknown";
        }
    }

    inline AssetType AssetTypeFromString(const std::string& str)
    {
        if (str == "Mesh")     return AssetType::Mesh;
        if (str == "Texture")  return AssetType::Texture;
        if (str == "Shader")   return AssetType::Shader;
        if (str == "Material") return AssetType::Material;
        if (str == "Font")     return AssetType::Font;
        if (str == "Scene")    return AssetType::Scene;
        return AssetType::Unknown;
    }

    // ============================================================
    // GUID - 128-bit UUID (RFC 4122 Version 4)
    // ============================================================
    struct MoonAssetGUID
    {
        uint32_t data1 = 0;      // time_low
        uint16_t data2 = 0;      // time_mid
        uint16_t data3 = 0;      // time_high_and_version
        uint8_t  data4[8] = {};  // clock_seq + node

        bool IsValid() const
        {
            if (data1 != 0) return true;
            if (data2 != 0) return true;
            if (data3 != 0) return true;
            for (int i = 0; i < 8; ++i)
                if (data4[i] != 0) return true;
            return false;
        }

        bool operator==(const MoonAssetGUID& other) const
        {
            if (data1 != other.data1) return false;
            if (data2 != other.data2) return false;
            if (data3 != other.data3) return false;
            for (int i = 0; i < 8; ++i)
                if (data4[i] != other.data4[i]) return false;
            return true;
        }

        bool operator!=(const MoonAssetGUID& other) const { return !(*this == other); }

        bool operator<(const MoonAssetGUID& other) const
        {
            if (data1 != other.data1) return data1 < other.data1;
            if (data2 != other.data2) return data2 < other.data2;
            if (data3 != other.data3) return data3 < other.data3;
            for (int i = 0; i < 8; ++i)
                if (data4[i] != other.data4[i]) return data4[i] < other.data4[i];
            return false;
        }

        std::string ToString() const;
        static MoonAssetGUID FromString(const std::string& str);
        static MoonAssetGUID Generate();
        static MoonAssetGUID Invalid() { return MoonAssetGUID{}; }

        struct Hash
        {
            size_t operator()(const MoonAssetGUID& guid) const
            {
                // FNV-1a hash
                size_t hash = 14695981039346656037ull;
                hash ^= guid.data1; hash *= 1099511628211ull;
                hash ^= guid.data2; hash *= 1099511628211ull;
                hash ^= guid.data3; hash *= 1099511628211ull;
                for (int i = 0; i < 8; ++i)
                {
                    hash ^= guid.data4[i];
                    hash *= 1099511628211ull;
                }
                return hash;
            }
        };
    };
    static_assert(sizeof(MoonAssetGUID) == 16, "GUID must be 16 bytes");

    // ============================================================
    // 引用类型
    // ============================================================
    enum class AssetReferenceType : uint8_t
    {
        Hard = 0,
        Soft = 1
    };

    inline const char* AssetReferenceTypeToString(AssetReferenceType type)
    {
        return type == AssetReferenceType::Hard ? "Hard" : "Soft";
    }

    // ============================================================
    // 文件头 - 固定 64 字节
    // ============================================================
    struct MoonAssetFileHeader
    {
        uint32_t magic = kMoonAssetMagic;                    // [0-3]
        uint16_t fileFormatVersion = kMoonAssetFileFormatVersion; // [4-5]
        uint16_t assetVersion = 0;                           // [6-7]
        uint32_t assetType = static_cast<uint32_t>(AssetType::Unknown); // [8-11]

        // 各段偏移量和大小
        uint32_t metaOffset = 0;              // [12-15]
        uint32_t metaSize = 0;                // [16-19]
        uint32_t processedDataOffset = 0;     // [20-23]
        uint32_t processedDataSize = 0;       // [24-27]
        uint32_t rawSourceOffset = 0;         // [28-31]
        uint32_t rawSourceSize = 0;           // [32-35]
        uint32_t totalFileSize = 0;           // [36-39]

        // CRC32 校验
        uint32_t metaCRC32 = 0;               // [40-43]
        uint32_t processedCRC32 = 0;          // [44-47]
        uint32_t rawCRC32 = 0;                // [48-51]

        // 时间戳和版本
        uint64_t createTimestamp = 0;         // [52-59]
        uint32_t customVersion = 0;           // [60-63]

        bool IsValidMagic() const { return magic == kMoonAssetMagic; }
        bool IsValidVersion() const { return fileFormatVersion <= kMoonAssetFileFormatVersion; }
    };
    static_assert(sizeof(MoonAssetFileHeader) == 64, "Header must be exactly 64 bytes");

    // ============================================================
    // 辅助函数
    // ============================================================
    inline uint32_t AlignUp(uint32_t value, uint32_t alignment)
    {
        return (value + alignment - 1) & ~(alignment - 1);
    }

    // CRC32 计算
    uint32_t ComputeCRC32(const uint8_t* data, size_t size);

    // ============================================================
    // Meta 结构（内存表示）
    // ============================================================
    struct MoonAssetDependency
    {
        MoonAssetGUID guid;
        AssetReferenceType refType = AssetReferenceType::Hard;
    };

    struct MoonAssetMeta
    {
        MoonAssetGUID guid;
        AssetType assetType = AssetType::Unknown;
        std::string displayName;
        std::string sourceFilePath;
        uint64_t importTimestamp = 0;
        uint32_t importerVersion = 0;
        std::vector<MoonAssetDependency> dependencies;
        std::string importSettingsJson;  // 类型特定的导入设置 JSON

        // JSON 序列化
        std::string ToJson() const;
        bool FromJson(const std::string& jsonStr);
    };
}
