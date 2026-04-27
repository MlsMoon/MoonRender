#pragma once

#include "Source/AssetSystem/public/MoonAssetManager.h"
#include "Source/AssetSystem/public/MoonAssetImporter.h"
#include "Source/ResourcesProcess/public/Mesh.h"
#include "Source/ResourcesProcess/public/BufferStruct.h"

#include <vector>
#include <cstdint>

namespace AssetSystem
{
    // ============================================================
    // MeshAsset - 运行时 Mesh 资产
    // ============================================================
    class MeshAsset : public IAsset
    {
    public:
        MeshAsset() = default;

        // IAsset 接口
        AssetType GetAssetType() const override { return AssetType::Mesh; }
        MoonAssetGUID GetGUID() const override { return m_guid; }
        size_t GetMemorySize() const override;
        bool Deserialize(const uint8_t* processedData, size_t processedSize) override;

        // Mesh 数据访问
        const std::vector<BufferStruct::VertexPosNormal>& GetVertices() const { return m_vertices; }
        const std::vector<uint32_t>& GetIndices() const { return m_indices; }
        size_t GetVertexCount() const { return m_vertices.size(); }
        size_t GetIndexCount() const { return m_indices.size(); }
        uint32_t GetByteWidth() const;

        const void* GetVertexData() const { return m_vertices.data(); }
        const void* GetIndexData() const { return m_indices.data(); }

        // 导入设置
        const MeshImportSettings& GetImportSettings() const { return m_importSettings; }
        const std::string& GetSourceFilePath() const { return m_sourceFilePath; }

        // 序列化 / 反序列化
        bool SerializeProcessedData(std::vector<uint8_t>& outData) const;
        bool DeserializeProcessedData(const uint8_t* data, size_t size);

        // 从运行时数据构建
        static std::unique_ptr<MeshAsset> CreateFromData(
            MoonAssetGUID guid,
            std::vector<BufferStruct::VertexPosNormal> vertices,
            std::vector<uint32_t> indices,
            const MeshImportSettings& settings,
            const std::string& sourcePath);

        // 设置 GUID（用于导入时）
        void SetGUID(MoonAssetGUID guid) { m_guid = guid; }
        void SetSourceFilePath(const std::string& path) { m_sourceFilePath = path; }
        void SetImportSettings(const MeshImportSettings& settings) { m_importSettings = settings; }

        void SetVertices(std::vector<BufferStruct::VertexPosNormal> vertices) { m_vertices = std::move(vertices); }
        void SetIndices(std::vector<uint32_t> indices) { m_indices = std::move(indices); }

    private:
        MoonAssetGUID m_guid;
        std::vector<BufferStruct::VertexPosNormal> m_vertices;
        std::vector<uint32_t> m_indices;
        MeshImportSettings m_importSettings;
        std::string m_sourceFilePath;
    };
}
