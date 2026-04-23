#include "../public/D3DUtil.h"

#include <filesystem>
#include <string>

#define SAFE_RELEASE(p) { if ((p)) { (p)->Release(); (p) = nullptr; } }

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
}

HRESULT CreateShaderFromFile(
    const WCHAR* csoFileNameInOut,
    const WCHAR* hlslFileName,
    LPCSTR entryPoint,
    LPCSTR shaderModel,
    ID3DBlob** ppBlobOut)
{
    HRESULT hr = S_OK;

    if (csoFileNameInOut && D3DReadFileToBlob(csoFileNameInOut, ppBlobOut) == S_OK)
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
        }
        SAFE_RELEASE(errorBlob);
        return hr;
    }

    if (csoFileNameInOut)
    {
        return D3DWriteBlobToFile(*ppBlobOut, csoFileNameInOut, FALSE);
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
