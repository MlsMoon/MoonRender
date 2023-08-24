#include "../public/MoonMeshLoader.h"
#include "Source/Logging/public/LogSystem.h"


tinyobj::ObjReader MoonMeshLoader::LoadObjFile(std::string inputFilePath)
{
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = "\\"; // Path to material files

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(inputFilePath, reader_config)) {
        if (!reader.Error().empty()) {
            MOON_LOG("load Obj file failed");
            MOON_LOG("Error:"+reader.Error());
        }
    }

    if (!reader.Warning().empty()) {
        MOON_LOG(reader.Warning());
    }
    
    return reader;
}
