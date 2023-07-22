﻿//基于tiny_obj_loader.h https://github.com/tinyobjloader/tinyobjloader

#pragma once
#include <string>
#include "Source/EventSystem/LogSystem.h"
#include "Source/ThirdParty/tinyobjloader/tiny_obj_loader.h"
#include "BufferStruct.h"

class MoonObjLoader
{
public:
    static tinyobj::ObjReader LoadObjFile(std::string inputFilePath);
};
