#include "Source/Object/MeshComponent.h"

#include "Source/AssetSystem/public/MoonAssetManager.h"
#include "Source/AssetSystem/public/MoonMeshAsset.h"
#include "Source/Logging/public/LogSystem.h"

namespace Object
{
    MeshComponent::MeshComponent(AssetSystem::AssetHandle assetHandle)
        : m_assetHandle(assetHandle)
    {
        RegisterProperty(MoonProp::Text("Asset GUID", [this]()
        {
            return m_assetHandle.IsValid() ? m_assetHandle.GetGUID().ToString() : "Invalid";
        }));
        RegisterProperty(MoonProp::Text("Vertex Count", [this]()
        {
            const auto* mesh = GetMesh();
            return mesh != nullptr ? std::to_string(mesh->VertexNum) : "0";
        }));
        RegisterProperty(MoonProp::Text("Byte Width", [this]()
        {
            const auto* mesh = GetMesh();
            return mesh != nullptr ? std::to_string(mesh->ByteWidth) : "0";
        }));
    }

    MeshComponent::MeshComponent(std::string sourceFilePath, ResourcesProcess::MeshFileType fileType)
        : m_legacySourcePath(std::move(sourceFilePath)),
          m_fileType(fileType),
          m_legacyMesh(std::make_unique<ResourcesProcess::Mesh>(m_legacySourcePath, m_fileType))
    {
        RegisterProperty(MoonProp::Text("Source Path", m_legacySourcePath));
        RegisterProperty(MoonProp::Text("Vertex Count", [this]()
        {
            const auto* mesh = GetMesh();
            return mesh != nullptr ? std::to_string(mesh->VertexNum) : "0";
        }));
        RegisterProperty(MoonProp::Text("Byte Width", [this]()
        {
            const auto* mesh = GetMesh();
            return mesh != nullptr ? std::to_string(mesh->ByteWidth) : "0";
        }));
    }

    ResourcesProcess::Mesh* MeshComponent::GetMesh()
    {
        if (m_legacyMesh != nullptr)
            return m_legacyMesh.get();

        EnsureMeshLoaded();
        return m_cachedMesh.get();
    }

    const ResourcesProcess::Mesh* MeshComponent::GetMesh() const
    {
        if (m_legacyMesh != nullptr)
            return m_legacyMesh.get();

        EnsureMeshLoaded();
        return m_cachedMesh.get();
    }

    void MeshComponent::SetAssetHandle(AssetSystem::AssetHandle handle)
    {
        m_assetHandle = handle;
        m_cacheValid = false;
        m_cachedMesh.reset();
        m_legacyMesh.reset();
        m_legacySourcePath.clear();
    }

    void MeshComponent::Reimport()
    {
        if (m_assetHandle.IsValid())
        {
            MOON_LOG("Reimport asset: " + m_assetHandle.GetGUID().ToString());
            // TODO: Implement reimport logic
        }
        else if (!m_legacySourcePath.empty())
        {
            MOON_LOG("Reimport legacy mesh: " + m_legacySourcePath);
            // Reload from source path
            m_legacyMesh = std::make_unique<ResourcesProcess::Mesh>(m_legacySourcePath, m_fileType);
        }
    }

    void MeshComponent::EnsureMeshLoaded() const
    {
        if (m_cacheValid)
            return;

        if (!m_assetHandle.IsValid())
            return;

        auto* asset = AssetSystem::AssetManager::Get().LoadSync(m_assetHandle.GetGUID());
        if (!asset)
            return;

        auto* meshAsset = dynamic_cast<AssetSystem::MeshAsset*>(asset);
        if (!meshAsset)
            return;

        m_cachedMesh = std::make_unique<ResourcesProcess::Mesh>();
        m_cachedMesh->VertexNum = meshAsset->GetVertexCount();
        m_cachedMesh->ByteWidth = meshAsset->GetByteWidth();
        m_cachedMesh->VertexBufferData = meshAsset->GetVertices();
        m_cacheValid = true;
    }
}
