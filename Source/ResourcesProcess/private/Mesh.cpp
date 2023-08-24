#include "../public/Mesh.h"

#include "Source/Logging/public/LogSystem.h"
#include "Source/ResourcesProcess/public/MoonMeshLoader.h"

ResourcesProcess::Mesh::Mesh(std::string SourceFilePath,MeshFileType FileType)
{
    switch (FileType)
    {
        case OBJ:
            init_obj_mesh(SourceFilePath);
            break;
        case FBX:
            MOON_LOG("do not support FBX");
            break;
        case USD:
            MOON_LOG("do not support USD");
            break;
        default: ;
    }
}

ResourcesProcess::Mesh::~Mesh()
{
    
}


void* ResourcesProcess::Mesh::get_sys_mem()
{
    return VertexBufferData.data();
}

void ResourcesProcess::Mesh::init_obj_mesh(std::string FilePath)
{
    
    tinyobj::ObjReader reader = MoonMeshLoader::LoadObjFile(FilePath);
    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    //check
    if (shapes.size() == 0)
    {
        return;
    }

    //TODO:待确认这里 *3 的正确性
    VertexNum = shapes[0].mesh.num_face_vertices.size() * 3;
    
    VertexBufferData.resize(VertexNum);

     for (size_t s = 0; s < shapes.size(); s++)
     {
         // Loop over faces(polygon)
         size_t index_offset = 0;
         for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)//[Cube-tri]Size : 12 (face num)
             {
             size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

             // Loop over vertices in the face.
             for (size_t v = 0; v < fv; v++)//三角面就是3个
                 {
                 // access to vertex
                 tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                 tinyobj::real_t vx = attrib.vertices[3*size_t(idx.vertex_index)+0];
                 tinyobj::real_t vy = attrib.vertices[3*size_t(idx.vertex_index)+1];
                 tinyobj::real_t vz = attrib.vertices[3*size_t(idx.vertex_index)+2];
                
                 VertexBufferData[index_offset + v].pos = DirectX::XMFLOAT3(vx,vy,vz);

                 // Check if `normal_index` is zero or positive. negative = no normal data
                 if (idx.normal_index >= 0) {
                     tinyobj::real_t nx = attrib.normals[3*size_t(idx.normal_index)+0];
                     tinyobj::real_t ny = attrib.normals[3*size_t(idx.normal_index)+1];
                     tinyobj::real_t nz = attrib.normals[3*size_t(idx.normal_index)+2];
                    
                     VertexBufferData[index_offset + v].normal = DirectX::XMFLOAT3(nx,ny,nz);
                 }

                 // Check if `texcoord_index` is zero or positive. negative = no texcoord data
                 if (idx.texcoord_index >= 0) {
                     tinyobj::real_t tx = attrib.texcoords[2*size_t(idx.texcoord_index)+0];
                     tinyobj::real_t ty = attrib.texcoords[2*size_t(idx.texcoord_index)+1];
                 }

                 // Optional: vertex colors
                 // tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
                 // tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
                 // tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];
                 }
             index_offset += fv;

             // per-face material
             shapes[s].mesh.material_ids[f];
             }
     }
    ByteWidth=VertexNum * sizeof(BufferStruct::VertexPosNormal);
}



