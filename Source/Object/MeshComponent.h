#pragma once

#include <memory>
#include <string>
#include <utility>

#include "Source/Object/MoonComponent.h"
#include "Source/ResourcesProcess/public/Mesh.h"
#include "Source/AssetSystem/public/MoonAssetHandle.h"

namespace Object
{
    class MeshComponent final : public MoonToggleableComponent
    {
    public:
        // New constructor: AssetHandle-based
        explicit MeshComponent(AssetSystem::AssetHandle assetHandle);

        // Legacy constructor: path-based (kept for backward compatibility)
        MeshComponent(std::string sourceFilePath, ResourcesProcess::MeshFileType fileType);

        MOON_COMPONENT(Mesh, "Mesh", Renderable)

        ResourcesProcess::Mesh* GetMesh();
        const ResourcesProcess::Mesh* GetMesh() const;
        const std::string& GetSourceFilePath() const { return m_legacySourcePath; }

        AssetSystem::AssetHandle GetAssetHandle() const { return m_assetHandle; }
        void SetAssetHandle(AssetSystem::AssetHandle handle);

        void Reimport();

    private:
        void EnsureMeshLoaded() const;

    private:
        AssetSystem::AssetHandle m_assetHandle;

        // Legacy fields (for backward compatibility)
        std::string m_legacySourcePath;
        ResourcesProcess::MeshFileType m_fileType = ResourcesProcess::OBJ;
        std::unique_ptr<ResourcesProcess::Mesh> m_legacyMesh;

        // Cached mesh from AssetManager
        mutable std::unique_ptr<ResourcesProcess::Mesh> m_cachedMesh;
        mutable bool m_cacheValid = false;
    };
}
