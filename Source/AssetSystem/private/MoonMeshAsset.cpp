#include "Source/AssetSystem/public/MoonMeshAsset.h"

#include <cstring>

namespace AssetSystem
{
    size_t MeshAsset::GetMemorySize() const
    {
        return sizeof(MeshAsset)
            + m_vertices.size() * sizeof(BufferStruct::VertexPosNormal)
            + m_indices.size() * sizeof(uint32_t)
            + m_sourceFilePath.size();
    }

    uint32_t MeshAsset::GetByteWidth() const
    {
        return static_cast<uint32_t>(m_vertices.size() * sizeof(BufferStruct::VertexPosNormal));
    }

    bool MeshAsset::SerializeProcessedData(std::vector<uint8_t>& outData) const
    {
        // Layout:
        // uint32_t vertexCount
        // uint32_t indexCount
        // uint32_t vertexStride
        // uint32_t vertexFormat
        // VertexPosNormal[vertexCount]
        // uint32_t indices[indexCount]

        const uint32_t vertexCount = static_cast<uint32_t>(m_vertices.size());
        const uint32_t indexCount = static_cast<uint32_t>(m_indices.size());
        const uint32_t vertexStride = sizeof(BufferStruct::VertexPosNormal);
        const uint32_t vertexFormat = static_cast<uint32_t>(m_importSettings.vertexFormat);

        const size_t totalSize = sizeof(vertexCount) + sizeof(indexCount)
            + sizeof(vertexStride) + sizeof(vertexFormat)
            + m_vertices.size() * vertexStride
            + m_indices.size() * sizeof(uint32_t);

        outData.resize(totalSize);
        size_t offset = 0;

        std::memcpy(outData.data() + offset, &vertexCount, sizeof(vertexCount));
        offset += sizeof(vertexCount);

        std::memcpy(outData.data() + offset, &indexCount, sizeof(indexCount));
        offset += sizeof(indexCount);

        std::memcpy(outData.data() + offset, &vertexStride, sizeof(vertexStride));
        offset += sizeof(vertexStride);

        std::memcpy(outData.data() + offset, &vertexFormat, sizeof(vertexFormat));
        offset += sizeof(vertexFormat);

        if (!m_vertices.empty())
        {
            std::memcpy(outData.data() + offset, m_vertices.data(), m_vertices.size() * vertexStride);
            offset += m_vertices.size() * vertexStride;
        }

        if (!m_indices.empty())
        {
            std::memcpy(outData.data() + offset, m_indices.data(), m_indices.size() * sizeof(uint32_t));
        }

        return true;
    }

    bool MeshAsset::DeserializeProcessedData(const uint8_t* data, size_t size)
    {
        if (size < 16) return false;

        size_t offset = 0;

        uint32_t vertexCount = 0;
        uint32_t indexCount = 0;
        uint32_t vertexStride = 0;
        uint32_t vertexFormat = 0;

        std::memcpy(&vertexCount, data + offset, sizeof(vertexCount));
        offset += sizeof(vertexCount);

        std::memcpy(&indexCount, data + offset, sizeof(indexCount));
        offset += sizeof(indexCount);

        std::memcpy(&vertexStride, data + offset, sizeof(vertexStride));
        offset += sizeof(vertexStride);

        std::memcpy(&vertexFormat, data + offset, sizeof(vertexFormat));
        offset += sizeof(vertexFormat);

        if (vertexStride != sizeof(BufferStruct::VertexPosNormal))
        {
            // Vertex format mismatch - would need conversion
            return false;
        }

        const size_t expectedSize = 16
            + static_cast<size_t>(vertexCount) * vertexStride
            + static_cast<size_t>(indexCount) * sizeof(uint32_t);
        if (size < expectedSize)
            return false;

        m_vertices.resize(vertexCount);
        if (vertexCount > 0)
        {
            std::memcpy(m_vertices.data(), data + offset, vertexCount * vertexStride);
            offset += vertexCount * vertexStride;
        }

        m_indices.resize(indexCount);
        if (indexCount > 0)
        {
            std::memcpy(m_indices.data(), data + offset, indexCount * sizeof(uint32_t));
        }

        m_importSettings.vertexFormat = static_cast<VertexFormat>(vertexFormat);

        return true;
    }

    bool MeshAsset::Deserialize(const uint8_t* processedData, size_t processedSize)
    {
        return DeserializeProcessedData(processedData, processedSize);
    }

    std::unique_ptr<MeshAsset> MeshAsset::CreateFromData(
        MoonAssetGUID guid,
        std::vector<BufferStruct::VertexPosNormal> vertices,
        std::vector<uint32_t> indices,
        const MeshImportSettings& settings,
        const std::string& sourcePath)
    {
        auto asset = std::make_unique<MeshAsset>();
        asset->m_guid = guid;
        asset->m_vertices = std::move(vertices);
        asset->m_indices = std::move(indices);
        asset->m_importSettings = settings;
        asset->m_sourceFilePath = sourcePath;
        return asset;
    }
}
