#pragma once
#include <string>
#include <vector>

#include "BufferStruct.h"

namespace ResourcesProcess
{
    enum MeshFileType
    {
        OBJ,
        FBX,
        USD
    };

    
    class Mesh
    {
    public:
        size_t VertexNum;
        UINT ByteWidth;
        //TODO:支持VertexColor
        std::vector<BufferStruct::VertexPosNormal> VertexBufferData;
        
        
    public:
        //Construction
        Mesh(std::string SourceFilePath,MeshFileType FileType);
        ~Mesh();
        void* get_sys_mem();
        
    private:
        /**
         * \brief 使用tiny_obj_load进行载入 
         */
        void init_obj_mesh(std::string FilePath);
    };
}

