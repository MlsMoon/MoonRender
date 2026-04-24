#pragma once

#include <memory>
#include <string>
#include <utility>

#include "Source/Object/MoonComponent.h"
#include "Source/ResourcesProcess/public/Mesh.h"

namespace Object
{
    class MeshComponent final : public MoonComponent
    {
    public:
        MeshComponent(std::string sourceFilePath, ResourcesProcess::MeshFileType fileType)
            : m_sourceFilePath(std::move(sourceFilePath)),
              m_fileType(fileType),
              m_mesh(std::make_unique<ResourcesProcess::Mesh>(m_sourceFilePath, m_fileType))
        {
            RegisterProperty(MoonProp::Text("Source Path", m_sourceFilePath));
            RegisterProperty(MoonProp::Text("Vertex Count", [this]()
            {
                return m_mesh != nullptr ? std::to_string(m_mesh->VertexNum) : "0";
            }));
            RegisterProperty(MoonProp::Text("Byte Width", [this]()
            {
                return m_mesh != nullptr ? std::to_string(m_mesh->ByteWidth) : "0";
            }));
        }

        MOON_COMPONENT(Mesh, "Mesh", Renderable)

        ResourcesProcess::Mesh* GetMesh() { return m_mesh.get(); }
        const ResourcesProcess::Mesh* GetMesh() const { return m_mesh.get(); }
        const std::string& GetSourceFilePath() const { return m_sourceFilePath; }

    private:
        std::string m_sourceFilePath;
        ResourcesProcess::MeshFileType m_fileType;
        std::unique_ptr<ResourcesProcess::Mesh> m_mesh;
    };
}
