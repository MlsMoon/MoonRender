#include "Source/AssetSystem/public/MoonAssetImporter.h"

#include "Source/Logging/public/LogSystem.h"
#include "Source/ResourcesProcess/public/MoonMeshLoader.h"
#include "Source/ResourcesProcess/public/Mesh.h"
#include "Source/ResourcesProcess/public/BufferStruct.h"

#include <filesystem>
#include <fstream>
#include <chrono>

namespace AssetSystem
{
    // ============================================================
    // MeshImportSettings JSON
    // ============================================================
    std::string MeshImportSettings::ToJson() const
    {
        nlohmann::json j;
        switch (coordinateSystem)
        {
        case CoordinateSystem::YUpToZUp: j["coordinateSystem"] = "YUpToZUp"; break;
        case CoordinateSystem::ZUpToYUp: j["coordinateSystem"] = "ZUpToYUp"; break;
        default: j["coordinateSystem"] = "KeepOriginal"; break;
        }
        switch (vertexFormat)
        {
        case VertexFormat::PosOnly: j["vertexFormat"] = "PosOnly"; break;
        case VertexFormat::PosNormalColor: j["vertexFormat"] = "PosNormalColor"; break;
        case VertexFormat::PosNormalUV: j["vertexFormat"] = "PosNormalUV"; break;
        case VertexFormat::PosNormalColorUV: j["vertexFormat"] = "PosNormalColorUV"; break;
        default: j["vertexFormat"] = "PosNormal"; break;
        }
        switch (normalGeneration)
        {
        case NormalGeneration::AlwaysImport: j["normalGeneration"] = "AlwaysImport"; break;
        case NormalGeneration::AlwaysGenerate: j["normalGeneration"] = "AlwaysGenerate"; break;
        case NormalGeneration::SmoothGenerate: j["normalGeneration"] = "SmoothGenerate"; break;
        default: j["normalGeneration"] = "ImportIfAvailable"; break;
        }
        j["generateTangents"] = generateTangents;
        j["flipUVs"] = flipUVs;
        j["flipWindingOrder"] = flipWindingOrder;
        j["combineMeshes"] = combineMeshes;
        j["scaleFactor"] = scaleFactor;
        return j.dump();
    }

    bool MeshImportSettings::FromJson(const std::string& jsonStr)
    {
        try
        {
            nlohmann::json j = nlohmann::json::parse(jsonStr);
            if (j.contains("coordinateSystem"))
            {
                std::string cs = j["coordinateSystem"];
                if (cs == "YUpToZUp") coordinateSystem = CoordinateSystem::YUpToZUp;
                else if (cs == "ZUpToYUp") coordinateSystem = CoordinateSystem::ZUpToYUp;
                else coordinateSystem = CoordinateSystem::KeepOriginal;
            }
            if (j.contains("vertexFormat"))
            {
                std::string vf = j["vertexFormat"];
                if (vf == "PosOnly") vertexFormat = VertexFormat::PosOnly;
                else if (vf == "PosNormalColor") vertexFormat = VertexFormat::PosNormalColor;
                else if (vf == "PosNormalUV") vertexFormat = VertexFormat::PosNormalUV;
                else if (vf == "PosNormalColorUV") vertexFormat = VertexFormat::PosNormalColorUV;
                else vertexFormat = VertexFormat::PosNormal;
            }
            if (j.contains("normalGeneration"))
            {
                std::string ng = j["normalGeneration"];
                if (ng == "AlwaysImport") normalGeneration = NormalGeneration::AlwaysImport;
                else if (ng == "AlwaysGenerate") normalGeneration = NormalGeneration::AlwaysGenerate;
                else if (ng == "SmoothGenerate") normalGeneration = NormalGeneration::SmoothGenerate;
                else normalGeneration = NormalGeneration::ImportIfAvailable;
            }
            if (j.contains("generateTangents")) generateTangents = j["generateTangents"];
            if (j.contains("flipUVs")) flipUVs = j["flipUVs"];
            if (j.contains("flipWindingOrder")) flipWindingOrder = j["flipWindingOrder"];
            if (j.contains("combineMeshes")) combineMeshes = j["combineMeshes"];
            if (j.contains("scaleFactor")) scaleFactor = j["scaleFactor"];
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    // ============================================================
    // MeshImporter
    // ============================================================
    bool MeshImporter::CanImport(const std::string& filePath) const
    {
        std::filesystem::path p(filePath);
        std::string ext = p.extension().string();
        for (const auto& supported : GetSupportedExtensions())
        {
            if (ext == supported)
                return true;
        }
        return false;
    }

    std::vector<std::string> MeshImporter::GetSupportedExtensions() const
    {
        return { ".obj", ".fbx", ".usd" };
    }

    ImportResult MeshImporter::Import(const ImportContext& context)
    {
        std::filesystem::path sourcePath(context.sourceFilePath);
        std::string ext = sourcePath.extension().string();

        if (ext == ".obj")
        {
            return ImportOBJ(context);
        }
        else if (ext == ".fbx")
        {
            ImportResult result;
            result.errorMessage = "FBX import not yet implemented";
            return result;
        }
        else if (ext == ".usd")
        {
            ImportResult result;
            result.errorMessage = "USD import not yet implemented";
            return result;
        }

        ImportResult result;
        result.errorMessage = "Unsupported file format: " + ext;
        return result;
    }

    ImportResult MeshImporter::Reimport(const std::string& moonassetPath)
    {
        ImportResult result;

        MoonAssetMeta meta;
        MoonAssetFileHeader header;
        if (!ReadMetaFromFile(moonassetPath, meta, header))
        {
            result.errorMessage = "Failed to read meta from: " + moonassetPath;
            return result;
        }

        if (!std::filesystem::exists(meta.sourceFilePath))
        {
            result.errorMessage = "Source file not found: " + meta.sourceFilePath;
            return result;
        }

        // Parse import settings from meta
        MeshImportSettings settings;
        if (!meta.importSettingsJson.empty())
        {
            settings.FromJson(meta.importSettingsJson);
        }

        ImportContext context;
        context.sourceFilePath = meta.sourceFilePath;
        context.outputDirectory = std::filesystem::path(moonassetPath).parent_path().string();
        context.displayName = meta.displayName;
        context.meshSettings = settings;
        context.overwriteExisting = true;

        result = Import(context);
        if (result.success)
        {
            // Force GUID to match original
            // This requires rewriting the file with the original GUID
            result.guid = meta.guid;
        }

        return result;
    }

    ImportResult MeshImporter::ImportOBJ(const ImportContext& context)
    {
        ImportResult result;

        // Load source OBJ
        tinyobj::ObjReader reader = MoonMeshLoader::LoadObjFile(context.sourceFilePath);
        if (!reader.Valid())
        {
            result.errorMessage = "Failed to load OBJ: " + reader.Error();
            return result;
        }

        auto& attrib = reader.GetAttrib();
        auto& shapes = reader.GetShapes();

        if (shapes.empty())
        {
            result.errorMessage = "No shapes found in OBJ";
            return result;
        }

        // Process vertices
        std::vector<BufferStruct::VertexPosNormal> vertices;
        std::vector<uint32_t> indices;

        for (size_t s = 0; s < shapes.size(); ++s)
        {
            size_t index_offset = 0;
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); ++f)
            {
                size_t fv = static_cast<size_t>(shapes[s].mesh.num_face_vertices[f]);

                for (size_t v = 0; v < fv; ++v)
                {
                    tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                    BufferStruct::VertexPosNormal vert;

                    // Position
                    float vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                    float vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                    float vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

                    // Apply scale factor
                    vx *= context.meshSettings.scaleFactor;
                    vy *= context.meshSettings.scaleFactor;
                    vz *= context.meshSettings.scaleFactor;

                    // Coordinate system conversion
                    if (context.meshSettings.coordinateSystem == CoordinateSystem::YUpToZUp)
                    {
                        float temp = vy;
                        vy = vz;
                        vz = -temp;
                    }
                    else if (context.meshSettings.coordinateSystem == CoordinateSystem::ZUpToYUp)
                    {
                        float temp = vy;
                        vy = vz;
                        vz = temp;
                    }

                    vert.pos = MoonVector3(vx, vy, vz);

                    // Normal
                    if (idx.normal_index >= 0)
                    {
                        float nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                        float ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                        float nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
                        vert.normal = MoonVector3(nx, ny, nz);
                    }
                    else
                    {
                        vert.normal = MoonVector3(0.0f, 1.0f, 0.0f);
                    }

                    vertices.push_back(vert);
                    indices.push_back(static_cast<uint32_t>(vertices.size() - 1));
                }

                index_offset += fv;
            }
        }

        // Generate output filename
        std::filesystem::path sourcePath(context.sourceFilePath);
        std::string baseName = sourcePath.stem().string();
        std::string outputPath = context.outputDirectory + "/" + baseName + ".moonasset";

        // Create MeshAsset
        MoonAssetGUID guid = MoonAssetGUID::Generate();
        auto meshAsset = MeshAsset::CreateFromData(guid, std::move(vertices), std::move(indices),
                                                   context.meshSettings, context.sourceFilePath);

        // Build meta
        MoonAssetMeta meta;
        meta.guid = guid;
        meta.assetType = AssetType::Mesh;
        meta.displayName = context.displayName.empty() ? baseName : context.displayName;
        meta.sourceFilePath = context.sourceFilePath;
        meta.importTimestamp = static_cast<uint64_t>(
            std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
        meta.importerVersion = 1;
        meta.importSettingsJson = context.meshSettings.ToJson();

        // Serialize processed data
        std::vector<uint8_t> processedData;
        meshAsset->SerializeProcessedData(processedData);

        // Read raw source data
        std::vector<uint8_t> rawSourceData;
        {
            std::ifstream srcFile(context.sourceFilePath, std::ios::binary | std::ios::ate);
            if (srcFile.is_open())
            {
                auto size = srcFile.tellg();
                srcFile.seekg(0, std::ios::beg);
                rawSourceData.resize(static_cast<size_t>(size));
                srcFile.read(reinterpret_cast<char*>(rawSourceData.data()), size);
            }
        }

        // Write file
        if (!WriteMoonAssetFile(outputPath, meta, processedData, rawSourceData))
        {
            result.errorMessage = "Failed to write .moonasset file";
            return result;
        }

        result.success = true;
        result.outputPath = outputPath;
        result.guid = guid;
        return result;
    }

    // ============================================================
    // Utility functions
    // ============================================================
    bool ReadMetaFromFile(const std::string& filePath, MoonAssetMeta& outMeta, MoonAssetFileHeader& outHeader)
    {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open())
            return false;

        file.read(reinterpret_cast<char*>(&outHeader), sizeof(outHeader));
        if (!file.good() || !outHeader.IsValidMagic())
            return false;

        std::vector<char> metaJson(outHeader.metaSize + 1, '\0');
        file.seekg(outHeader.metaOffset);
        file.read(metaJson.data(), outHeader.metaSize);

        return outMeta.FromJson(std::string(metaJson.data(), outHeader.metaSize));
    }

    bool WriteMoonAssetFile(const std::string& filePath,
                            const MoonAssetMeta& meta,
                            const std::vector<uint8_t>& processedData,
                            const std::vector<uint8_t>& rawSourceData)
    {
        // Ensure output directory exists
        std::filesystem::path outPath(filePath);
        std::filesystem::create_directories(outPath.parent_path());

        std::ofstream file(filePath, std::ios::binary);
        if (!file.is_open())
            return false;

        // Prepare meta JSON
        std::string metaJsonStr = meta.ToJson();

        // Build header
        MoonAssetFileHeader header;
        header.magic = kMoonAssetMagic;
        header.fileFormatVersion = kMoonAssetFileFormatVersion;
        header.assetVersion = kMeshAssetVersion;
        header.assetType = static_cast<uint32_t>(meta.assetType);
        header.createTimestamp = meta.importTimestamp;
        header.customVersion = 0;

        // Calculate offsets
        header.metaOffset = AlignUp(static_cast<uint32_t>(sizeof(header)), kMoonAssetAlignment);
        header.metaSize = static_cast<uint32_t>(metaJsonStr.size());

        header.processedDataOffset = AlignUp(header.metaOffset + header.metaSize, kMoonAssetAlignment);
        header.processedDataSize = static_cast<uint32_t>(processedData.size());

        header.rawSourceOffset = AlignUp(header.processedDataOffset + header.processedDataSize, kMoonAssetAlignment);
        header.rawSourceSize = static_cast<uint32_t>(rawSourceData.size());

        header.totalFileSize = header.rawSourceOffset + header.rawSourceSize;

        // Compute CRC32
        header.metaCRC32 = ComputeCRC32(reinterpret_cast<const uint8_t*>(metaJsonStr.data()), metaJsonStr.size());
        if (!processedData.empty())
            header.processedCRC32 = ComputeCRC32(processedData.data(), processedData.size());
        if (!rawSourceData.empty())
            header.rawCRC32 = ComputeCRC32(rawSourceData.data(), rawSourceData.size());

        // Write header
        file.write(reinterpret_cast<const char*>(&header), sizeof(header));

        // Pad to meta offset
        uint32_t headerPadSize = header.metaOffset - static_cast<uint32_t>(sizeof(header));
        if (headerPadSize > 0)
        {
            std::vector<char> padding(headerPadSize, 0);
            file.write(padding.data(), headerPadSize);
        }

        // Write meta
        file.write(metaJsonStr.data(), metaJsonStr.size());

        // Pad to processed data offset
        uint32_t metaPadSize = header.processedDataOffset - (header.metaOffset + header.metaSize);
        if (metaPadSize > 0)
        {
            std::vector<char> padding(metaPadSize, 0);
            file.write(padding.data(), metaPadSize);
        }

        // Write processed data
        if (!processedData.empty())
            file.write(reinterpret_cast<const char*>(processedData.data()), processedData.size());

        // Pad to raw source offset
        uint32_t procPadSize = header.rawSourceOffset - (header.processedDataOffset + header.processedDataSize);
        if (procPadSize > 0)
        {
            std::vector<char> padding(procPadSize, 0);
            file.write(padding.data(), procPadSize);
        }

        // Write raw source data
        if (!rawSourceData.empty())
            file.write(reinterpret_cast<const char*>(rawSourceData.data()), rawSourceData.size());

        return file.good();
    }
}
