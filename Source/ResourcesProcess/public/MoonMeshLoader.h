//基于tiny_obj_loader.h https://github.com/tinyobjloader/tinyobjloader

#pragma once
#include <string>
#include "Source/ThirdParty/tinyobjloader/tiny_obj_loader.h"
#include "BufferStruct.h"

class MoonMeshLoader
{
public:
    static tinyobj::ObjReader LoadObjFile(std::string inputFilePath);
};
