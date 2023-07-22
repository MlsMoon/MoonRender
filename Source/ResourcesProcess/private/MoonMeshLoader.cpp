#include "../public/MoonMeshLoader.h"



tinyobj::ObjReader MoonMeshLoader::LoadObjFile(std::string inputFilePath)
{
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = "\\"; // Path to material files

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(inputFilePath, reader_config)) {
        if (!reader.Error().empty()) {
            EventSystem::LogSystem::Print("load Obj file failed");
            EventSystem::LogSystem::Print("Error:"+reader.Error());
        }
    }

    if (!reader.Warning().empty()) {
        EventSystem::LogSystem::Print(reader.Warning());
    }
    
    return reader;
}
