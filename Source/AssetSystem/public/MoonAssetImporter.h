#pragma once

#include "Source/AssetSystem/public/MoonAssetFormat.h"

#include <string>
#include <memory>
#include <vector>

namespace AssetSystem
{
    // ============================================================
    // Mesh 导入设置
    // ============================================================
    enum class CoordinateSystem : uint8_t
    {
        KeepOriginal = 0,
        YUpToZUp = 1,
        ZUpToYUp = 2
    };

    enum class VertexFormat : uint8_t
    {
        PosOnly = 0,
        PosNormal = 1,
        PosNormalColor = 2,
        PosNormalUV = 3,
        PosNormalColorUV = 4
    };

    enum class NormalGeneration : uint8_t
    {
        ImportIfAvailable = 0,
        AlwaysImport = 1,
        AlwaysGenerate = 2,
        SmoothGenerate = 3
    };

    struct MeshImportSettings
    {
        CoordinateSystem coordinateSystem = CoordinateSystem::KeepOriginal;
        VertexFormat vertexFormat = VertexFormat::PosNormal;
        NormalGeneration normalGeneration = NormalGeneration::ImportIfAvailable;
        bool generateTangents = false;
        bool flipUVs = false;
        bool flipWindingOrder = false;
        bool combineMeshes = true;
        float scaleFactor = 1.0f;

        std::string ToJson() const;
        bool FromJson(const std::string& jsonStr);
    };

    // ============================================================
    // 导入结果
    // ============================================================
    struct ImportResult
    {
        bool success = false;
        std::string errorMessage;
        std::string outputPath;
        MoonAssetGUID guid;
    };

    // ============================================================
    // 导入上下文
    // ============================================================
    struct ImportContext
    {
        std::string sourceFilePath;
        std::string outputDirectory;
        std::string displayName;
        MeshImportSettings meshSettings;
        bool overwriteExisting = false;
    };

    // ============================================================
    // 导入器基类
    // ============================================================
    class IAssetImporter
    {
    public:
        virtual ~IAssetImporter() = default;
        virtual bool CanImport(const std::string& filePath) const = 0;
        virtual ImportResult Import(const ImportContext& context) = 0;
        virtual ImportResult Reimport(const std::string& moonassetPath) = 0;
        virtual std::vector<std::string> GetSupportedExtensions() const = 0;
    };

    // ============================================================
    // Mesh 导入器
    // ============================================================
    class MeshImporter : public IAssetImporter
    {
    public:
        bool CanImport(const std::string& filePath) const override;
        ImportResult Import(const ImportContext& context) override;
        ImportResult Reimport(const std::string& moonassetPath) override;
        std::vector<std::string> GetSupportedExtensions() const override;

    private:
        ImportResult ImportOBJ(const ImportContext& context);
    };

    // ============================================================
    // 工具函数
    // ============================================================
    // 读取 .moonasset 文件头的 meta（不加载完整资产）
    bool ReadMetaFromFile(const std::string& filePath, MoonAssetMeta& outMeta, MoonAssetFileHeader& outHeader);

    // 写入 .moonasset 文件（四段式）
    bool WriteMoonAssetFile(const std::string& filePath,
                            const MoonAssetMeta& meta,
                            const std::vector<uint8_t>& processedData,
                            const std::vector<uint8_t>& rawSourceData);
}
