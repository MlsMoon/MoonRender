#include "../public/D3DUtil.h"
#include <string>
#include <filesystem>

// 安全COM组件释放宏
#define SAFE_RELEASE(p) { if ((p)) { (p)->Release(); (p) = nullptr; } }

// ------------------------------
// CreateShaderFromFile函数
// ------------------------------
// [In]csoFileNameInOut 编译好的着色器二进制文件(.cso)，若有指定则优先寻找该文件并读取
// [In]hlslFileName     着色器代码，若未找到着色器二进制文件则编译着色器代码
// [In]entryPoint       入口点(指定开始的函数)
// [In]shaderModel      着色器模型，格式为"*s_5_0"，*可以为c,d,g,h,p,v之一
// [Out]ppBlobOut       输出着色器二进制信息
HRESULT CreateShaderFromFile(const WCHAR * csoFileNameInOut, const WCHAR * hlslFileName,
    LPCSTR entryPoint, LPCSTR shaderModel, ID3DBlob ** ppBlobOut)
{
    HRESULT hr = S_OK;

    // 寻找是否有已经编译好的顶点着色器
    if (csoFileNameInOut && D3DReadFileToBlob(csoFileNameInOut, ppBlobOut) == S_OK)
    {
        return hr;
    }
    else
    {
        DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
        // 设置 D3DCOMPILE_DEBUG 标志用于获取着色器调试信息。该标志可以提升调试体验，
        // 但仍然允许着色器进行优化操作
        dwShaderFlags |= D3DCOMPILE_DEBUG;

        // 在Debug环境下禁用优化以避免出现一些不合理的情况
        dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
        ID3DBlob* errorBlob = nullptr;
        hr = D3DCompileFromFile(hlslFileName, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, shaderModel,
            dwShaderFlags, 0, ppBlobOut, &errorBlob);
        if (FAILED(hr))
        {
            if (errorBlob != nullptr)
            {
                const char* error_msg = reinterpret_cast<const char*>(errorBlob->GetBufferPointer());
                OutputDebugStringA(error_msg);
                
                //TODO:Shader编译出问题时需要Fallback
                
            }
            SAFE_RELEASE(errorBlob);
            return hr;
        }

        // 若指定了输出文件名，则将着色器二进制信息输出
        if (csoFileNameInOut)
        {
            return D3DWriteBlobToFile(*ppBlobOut, csoFileNameInOut, FALSE);
        }
    }

    return hr;
}


HRESULT MoonCreateShaderFromFile(const WCHAR * hlslFileName,CompileShaderType shaderType,ID3DBlob ** ppBlobOut)
{
    //TODO:优化为直接获取根目录
    const std::filesystem::path currentFilePath(__FILE__);
    std::filesystem::path projectDir = currentFilePath.parent_path().parent_path().parent_path().parent_path().string();
    std::filesystem::path csoPath = std::filesystem::path(projectDir) / "Cache" / "CSO";

    // 检查路径是否存在，如果不存在则创建文件夹
    if (!std::filesystem::exists(csoPath)) {
        std::filesystem::create_directories(csoPath);
    }
    
    // 封装
    const WCHAR* csoFile = L".cso";
    const WCHAR* csoDir = L"Cache\\cso\\";
    const WCHAR* lastComponent = nullptr;
    const WCHAR* current = hlslFileName;

    
    while (*current != '\0') {
        const WCHAR* next = wcschr(current, '\\');
        if (next == nullptr)
            next = wcschr(current, '/');
    
        if (next == nullptr)
            lastComponent = current;
        else if (*(next + 1) != '\0')
            lastComponent = next + 1;
    
        current = next != nullptr ? next + 1 : current + wcslen(current);
    }
    // 计算目标缓冲区大小
    size_t totalLength = wcslen(csoDir) + wcslen(lastComponent) + wcslen(csoFile) + 1;
    // 创建目标缓冲区
    WCHAR* result = new WCHAR[totalLength];
    // 清空目标缓冲区
    wcscpy_s(result, totalLength, L"");
    // 拼接 csoDir
    wcscat_s(result, totalLength, csoDir);
    // 拼接 lastComponent
    if (lastComponent != nullptr)
        wcscat_s(result, totalLength, lastComponent);
    // 拼接 csoFile
    wcscat_s(result, totalLength, csoFile);

    LPCSTR entryPoint = nullptr;
    LPCSTR shaderModel = nullptr;
    
    
    switch (shaderType) {
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

    return CreateShaderFromFile(result, hlslFileName,entryPoint,shaderModel, ppBlobOut);
}