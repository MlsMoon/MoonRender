#include "../public/D3DUtil.h"

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_set>

#ifdef _WIN32
#define SAFE_RELEASE(p) { if ((p)) { (p)->Release(); (p) = nullptr; } }
#endif

namespace
{
    std::filesystem::path GetProjectRoot()
    {
#ifdef MOONRENDER_PROJECT_ROOT
        return std::filesystem::path(MOONRENDER_PROJECT_ROOT);
#else
        const std::filesystem::path currentFilePath(__FILE__);
        return currentFilePath.parent_path().parent_path().parent_path().parent_path();
#endif
    }

#ifdef _WIN32
    bool AnyIncludeNewer(const std::filesystem::path& sourcePath,
                         std::filesystem::file_time_type csoTime,
                         std::unordered_set<std::filesystem::path>& visited)
    {
        if (!visited.insert(sourcePath).second)
            return false;

        if (std::filesystem::last_write_time(sourcePath) > csoTime)
            return true;

        std::ifstream file(sourcePath);
        if (!file.is_open())
            return false;

        std::string line;
        while (std::getline(file, line))
        {
            const char* p = line.c_str();
            while (*p == ' ' || *p == '\t') ++p;
            if (*p != '#') continue;
            ++p;
            while (*p == ' ' || *p == '\t') ++p;
            if (std::strncmp(p, "include", 7) != 0) continue;
            p += 7;
            while (*p == ' ' || *p == '\t') ++p;
            if (*p != '"' && *p != '<') continue;
            const char delim = (*p == '"') ? '"' : '>';
            ++p;
            const char* start = p;
            while (*p != '\0' && *p != delim) ++p;
            if (*p == '\0') continue;

            std::filesystem::path includePath = sourcePath.parent_path() / std::string(start, p - start);
            if (std::filesystem::exists(includePath))
            {
                if (AnyIncludeNewer(includePath, csoTime, visited))
                    return true;
            }
        }

        return false;
    }

    bool IsShaderCacheFresh(const WCHAR* csoFileName, const WCHAR* hlslFileName)
    {
        if (csoFileName == nullptr || hlslFileName == nullptr)
            return false;

        const std::filesystem::path csoPath(csoFileName);
        const std::filesystem::path hlslPath(hlslFileName);
        if (!std::filesystem::exists(csoPath) || !std::filesystem::exists(hlslPath))
            return false;

        auto csoTime = std::filesystem::last_write_time(csoPath);

        if (std::filesystem::last_write_time(hlslPath) > csoTime)
            return false;

        std::unordered_set<std::filesystem::path> visited;
        visited.insert(csoPath);
        return !AnyIncludeNewer(hlslPath, csoTime, visited);
    }
#endif
}

#ifdef _WIN32

HRESULT CreateShaderFromFile(
    const WCHAR* csoFileNameInOut,
    const WCHAR* hlslFileName,
    LPCSTR entryPoint,
    LPCSTR shaderModel,
    ID3DBlob** ppBlobOut)
{
    HRESULT hr = S_OK;

    if (IsShaderCacheFresh(csoFileNameInOut, hlslFileName) &&
        D3DReadFileToBlob(csoFileNameInOut, ppBlobOut) == S_OK)
    {
        return hr;
    }

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    dwShaderFlags |= D3DCOMPILE_DEBUG;
    dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ID3DBlob* errorBlob = nullptr;
    hr = D3DCompileFromFile(
        hlslFileName,
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint,
        shaderModel,
        dwShaderFlags,
        0,
        ppBlobOut,
        &errorBlob);

    if (FAILED(hr))
    {
        if (errorBlob != nullptr)
        {
            const char* errorMsg = reinterpret_cast<const char*>(errorBlob->GetBufferPointer());
            OutputDebugStringA(errorMsg);
            printf("%s\n", errorMsg);
        }
        else
        {
            printf("D3DCompileFromFile failed: 0x%08X\n", static_cast<unsigned int>(hr));
        }
        SAFE_RELEASE(errorBlob);
        return hr;
    }

    if (csoFileNameInOut)
    {
        return D3DWriteBlobToFile(*ppBlobOut, csoFileNameInOut, TRUE);
    }

    return hr;
}

HRESULT MoonCreateShaderFromFile(const WCHAR* hlslFileName, CompileShaderType shaderType, ID3DBlob** ppBlobOut)
{
    const std::filesystem::path projectDir = GetProjectRoot();
    const std::filesystem::path csoPath = projectDir / "Builds" / "Cache" / "CSO";

    if (!std::filesystem::exists(csoPath))
    {
        std::filesystem::create_directories(csoPath);
    }

    const WCHAR* csoFile = L".cso";
    const WCHAR* lastComponent = nullptr;
    const WCHAR* current = hlslFileName;

    while (*current != L'\0')
    {
        const WCHAR* next = wcschr(current, L'\\');
        if (next == nullptr)
        {
            next = wcschr(current, L'/');
        }

        if (next == nullptr)
        {
            lastComponent = current;
        }
        else if (*(next + 1) != L'\0')
        {
            lastComponent = next + 1;
        }

        current = next != nullptr ? next + 1 : current + wcslen(current);
    }

    const std::wstring csoDir = csoPath.wstring() + L"\\";
    const size_t totalLength = csoDir.length() + wcslen(lastComponent) + wcslen(csoFile) + 1;
    WCHAR* result = new WCHAR[totalLength];
    wcscpy_s(result, totalLength, L"");
    wcscat_s(result, totalLength, csoDir.c_str());
    if (lastComponent != nullptr)
    {
        wcscat_s(result, totalLength, lastComponent);
    }
    wcscat_s(result, totalLength, csoFile);

    LPCSTR entryPoint = nullptr;
    LPCSTR shaderModel = nullptr;

    switch (shaderType)
    {
    case VS:
        entryPoint = "VS";
        shaderModel = "vs_5_0";
        break;
    case PS:
        entryPoint = "PS";
        shaderModel = "ps_5_0";
        break;
    case CS:
        entryPoint = "CS";
        shaderModel = "cs_5_0";
        break;
    }

    const HRESULT hr = CreateShaderFromFile(result, hlslFileName, entryPoint, shaderModel, ppBlobOut);
    delete[] result;
    return hr;
}

#endif // _WIN32

std::string MoonGetProjectRootPath()
{
    return GetProjectRoot().string();
}

std::string MoonGetAssetPath(const std::string& relativePath)
{
    return (GetProjectRoot() / relativePath).string();
}

std::wstring MoonGetAssetPathW(const std::wstring& relativePath)
{
    return (GetProjectRoot() / relativePath).wstring();
}

bool MoonEnsureDirectory(const std::string& relativePath)
{
    return std::filesystem::create_directories(GetProjectRoot() / relativePath) ||
        std::filesystem::exists(GetProjectRoot() / relativePath);
}
