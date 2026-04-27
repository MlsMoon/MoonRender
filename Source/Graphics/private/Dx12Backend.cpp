#include "Source/Graphics/public/GraphicsBackend.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "D3DCompiler.lib")

#include <array>
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#define NOMINMAX
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include "Source/AppWin/public/D3DUtil.h"
#include "Source/ThirdParty/ImGui/imgui.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace
{
    using Microsoft::WRL::ComPtr;

    constexpr UINT kFrameCount = 2;

    UINT AlignConstantBufferSize(UINT size)
    {
        // D3D12 要求 CBV 按 256 字节对齐，这里统一向上取整。
        return (size + 255u) & ~255u;
    }

    D3D12_PRIMITIVE_TOPOLOGY ToD3D12PrimitiveTopology(GraphicsPrimitiveTopology topology)
    {
        switch (topology)
        {
        case GraphicsPrimitiveTopology::TriangleList:
        default:
            return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        }
    }

    DXGI_FORMAT ToDxgiFormat(GraphicsFormat format)
    {
        switch (format)
        {
        case GraphicsFormat::R32G32_FLOAT:
            return DXGI_FORMAT_R32G32_FLOAT;
        case GraphicsFormat::R32G32B32_FLOAT:
            return DXGI_FORMAT_R32G32B32_FLOAT;
        case GraphicsFormat::R32G32B32A32_FLOAT:
            return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case GraphicsFormat::R8G8B8A8_UNORM:
            return DXGI_FORMAT_R8G8B8A8_UNORM;
        default:
            return DXGI_FORMAT_UNKNOWN;
        }
    }

    D3D12_FILL_MODE ToD3D12FillMode(GraphicsFillMode mode)
    {
        switch (mode)
        {
        case GraphicsFillMode::Wireframe:
            return D3D12_FILL_MODE_WIREFRAME;
        case GraphicsFillMode::Solid:
        default:
            return D3D12_FILL_MODE_SOLID;
        }
    }

    D3D12_CULL_MODE ToD3D12CullMode(GraphicsCullMode mode)
    {
        switch (mode)
        {
        case GraphicsCullMode::None:
            return D3D12_CULL_MODE_NONE;
        case GraphicsCullMode::Front:
            return D3D12_CULL_MODE_FRONT;
        case GraphicsCullMode::Back:
        default:
            return D3D12_CULL_MODE_BACK;
        }
    }

    D3D12_COMPARISON_FUNC ToD3D12ComparisonFunc(GraphicsComparisonFunc func)
    {
        switch (func)
        {
        case GraphicsComparisonFunc::Never:
            return D3D12_COMPARISON_FUNC_NEVER;
        case GraphicsComparisonFunc::Less:
            return D3D12_COMPARISON_FUNC_LESS;
        case GraphicsComparisonFunc::Equal:
            return D3D12_COMPARISON_FUNC_EQUAL;
        case GraphicsComparisonFunc::LessEqual:
            return D3D12_COMPARISON_FUNC_LESS_EQUAL;
        case GraphicsComparisonFunc::Greater:
            return D3D12_COMPARISON_FUNC_GREATER;
        case GraphicsComparisonFunc::NotEqual:
            return D3D12_COMPARISON_FUNC_NOT_EQUAL;
        case GraphicsComparisonFunc::GreaterEqual:
            return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
        case GraphicsComparisonFunc::Always:
            return D3D12_COMPARISON_FUNC_ALWAYS;
        default:
            return D3D12_COMPARISON_FUNC_LESS;
        }
    }

    DXGI_FORMAT ToDxgiIndexFormat(GraphicsIndexFormat format)
    {
        switch (format)
        {
        case GraphicsIndexFormat::UInt16:
            return DXGI_FORMAT_R16_UINT;
        case GraphicsIndexFormat::UInt32:
        default:
            return DXGI_FORMAT_R32_UINT;
        }
    }

    template <typename TTarget, typename TSource>
    TTarget* CheckedCast(TSource* source)
    {
        if (source == nullptr)
        {
            return nullptr;
        }

        TTarget* result = dynamic_cast<TTarget*>(source);
        assert(result != nullptr);
        return result;
    }

    class Dx12GraphicsBuffer final : public IGraphicsBuffer
    {
    public:
        ComPtr<ID3D12Resource> resource;
        D3D12_VERTEX_BUFFER_VIEW vertexView = {};
        D3D12_INDEX_BUFFER_VIEW indexView = {};
        void* mappedData = nullptr;
        UINT sizeInBytes = 0;
    };

    class Dx12ShaderBytecode final : public IGraphicsShaderBytecode
    {
    public:
        ComPtr<ID3DBlob> blob;
    };

    class Dx12VertexShader final : public IGraphicsVertexShader
    {
    public:
        ComPtr<ID3DBlob> blob;
    };

    class Dx12PixelShader final : public IGraphicsPixelShader
    {
    public:
        ComPtr<ID3DBlob> blob;
    };

    class Dx12InputLayout final : public IGraphicsInputLayout
    {
    public:
        std::vector<std::string> semanticNames;
        std::vector<D3D12_INPUT_ELEMENT_DESC> elements;
    };

    class Dx12RasterizerState final : public IGraphicsRasterizerState
    {
    public:
        D3D12_RASTERIZER_DESC desc = {};
    };

    class Dx12DepthStencilState final : public IGraphicsDepthStencilState
    {
    public:
        D3D12_DEPTH_STENCIL_DESC desc = {};
    };

    struct Dx12ImGuiFrameResources
    {
        // ImGui 每帧都会重新生成顶点/索引数据。
        // 这里给每个 back buffer 准备一套 upload buffer，避免 CPU/GPU 同时踩同一块内存。
        ComPtr<ID3D12Resource> vertexBuffer;
        ComPtr<ID3D12Resource> indexBuffer;
        ImDrawVert* mappedVertexData = nullptr;
        ImDrawIdx* mappedIndexData = nullptr;
        int vertexBufferCapacity = 0;
        int indexBufferCapacity = 0;
    };

    class Dx12Backend final : public IGraphicsBackend
    {
    public:
        ~Dx12Backend() override
        {
            ShutdownImGui();
            WaitForGpu();
            if (m_fenceEvent != nullptr)
            {
                CloseHandle(m_fenceEvent);
                m_fenceEvent = nullptr;
            }
        }

        GraphicsBackendType GetType() const override
        {
            return GraphicsBackendType::DX12;
        }

        bool Initialize(void* nativeWindowHandle, int width, int height, bool enable4xMsaa) override
        {
            // DX12 鍒濆鍖栫殑鎬讳綋椤哄簭鏄細
            // 1. 创建设备/命令队列/交换链
            // 2. 创建 RTV/DSV/fence/命令列表等长期对象
            // 3. 根据当前窗口尺寸创建 back buffer 和 depth buffer
            m_hwnd = glfwGetWin32Window(static_cast<GLFWwindow*>(nativeWindowHandle));
            m_width = width;
            m_height = height;
            m_enable4xMsaa = enable4xMsaa;

            if (!CreateDeviceObjects())
            {
                return false;
            }

            Resize(width, height);
            return true;
        }

        void Resize(int width, int height) override
        {
            if (!m_swapChain || width <= 0 || height <= 0)
            {
                return;
            }

            // Resize 前先等 GPU，把旧尺寸下正在使用的 back buffer / depth buffer 完全消费掉。
            WaitForGpu();

            m_width = width;
            m_height = height;
            m_hasOpenCommandList = false;

            for (UINT i = 0; i < kFrameCount; ++i)
            {
                m_renderTargets[i].Reset();
                m_fenceValues[i] = m_fenceValue;
            }
            m_depthStencil.Reset();

            DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
            m_swapChain->GetDesc(&swapChainDesc);
            if (FAILED(m_swapChain->ResizeBuffers(kFrameCount, width, height, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags)))
            {
                return;
            }

            m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

            // 交换链只负责生成 back buffer 资源；
            // 真正给渲染管线绑定时，还需要我们自己创建 RTV。
            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
            for (UINT i = 0; i < kFrameCount; ++i)
            {
                if (FAILED(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i]))))
                {
                    return;
                }
                m_device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);
                rtvHandle.ptr += m_rtvDescriptorSize;
            }

            D3D12_HEAP_PROPERTIES heapProperties = {};
            heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

            D3D12_RESOURCE_DESC depthDesc = {};
            depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            depthDesc.Width = static_cast<UINT64>(width);
            depthDesc.Height = static_cast<UINT>(height);
            depthDesc.DepthOrArraySize = 1;
            depthDesc.MipLevels = 1;
            depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            depthDesc.SampleDesc.Count = 1;
            depthDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
            depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

            D3D12_CLEAR_VALUE clearValue = {};
            clearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            clearValue.DepthStencil.Depth = 1.0f;
            clearValue.DepthStencil.Stencil = 0;

            if (FAILED(m_device->CreateCommittedResource(
                &heapProperties,
                D3D12_HEAP_FLAG_NONE,
                &depthDesc,
                D3D12_RESOURCE_STATE_DEPTH_WRITE,
                &clearValue,
                IID_PPV_ARGS(&m_depthStencil))))
            {
                return;
            }

            D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
            dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
            m_device->CreateDepthStencilView(m_depthStencil.Get(), &dsvDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

            // 视口和裁剪矩形通常跟窗口尺寸同步更新。
            GraphicsViewport viewport = {};
            viewport.width = static_cast<float>(width);
            viewport.height = static_cast<float>(height);
            SetViewport(viewport);
        }

        bool InitializeImGui(void*) override
        {
            if (m_imguiInitialized)
            {
                return true;
            }

            // DX12 娌℃湁鍍?DX11 閭ｆ牱鐨勨€滆澶囦笂涓嬫枃鍗虫覆鏌撳櫒鈥濇ā寮忥紝
            // 所以 ImGui 需要单独准备 root signature / PSO / SRV heap / 字体纹理。
            if (!CreateImGuiDeviceObjects())
            {
                ShutdownImGui();
                return false;
            }

            ImGuiIO& io = ImGui::GetIO();
            io.BackendRendererName = "MoonRender_DX12";
            io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
            m_imguiInitialized = true;
            return true;
        }

        void BeginImGuiFrame() override
        {
        }

        void RenderImGuiDrawData() override
        {
            if (!m_imguiInitialized)
            {
                return;
            }

            ImDrawData* drawData = ImGui::GetDrawData();
            if (drawData == nullptr || drawData->CmdListsCount == 0 ||
                drawData->DisplaySize.x <= 0.0f || drawData->DisplaySize.y <= 0.0f)
            {
                return;
            }

            if (!BeginFrameRecording())
            {
                return;
            }

            if (!EnsureImGuiFrameResources(drawData))
            {
                return;
            }

            // ImGui 每帧先把 CPU 侧 draw list 拷进 upload buffer，
            // 再把这些缓冲绑定到命令列表里发 draw call。
            UploadImGuiDrawData(*drawData, m_imguiFrameResources[m_frameIndex]);
            RecordImGuiDrawCommands(*drawData, m_imguiFrameResources[m_frameIndex]);
        }

        void ShutdownImGui() override
        {
            if (m_imguiInitialized && ImGui::GetCurrentContext() != nullptr)
            {
                ImGuiIO& io = ImGui::GetIO();
                io.BackendRendererName = nullptr;
                io.BackendFlags &= ~ImGuiBackendFlags_RendererHasVtxOffset;
                if (io.Fonts != nullptr)
                {
                    io.Fonts->SetTexID(nullptr);
                }
            }

            for (Dx12ImGuiFrameResources& frameResources : m_imguiFrameResources)
            {
                frameResources.mappedVertexData = nullptr;
                frameResources.mappedIndexData = nullptr;
                frameResources.vertexBuffer.Reset();
                frameResources.indexBuffer.Reset();
                frameResources.vertexBufferCapacity = 0;
                frameResources.indexBufferCapacity = 0;
            }

            m_imguiFontTexture.Reset();
            m_imguiFontUpload.Reset();
            m_imguiSrvHeap.Reset();
            m_imguiRootSignature.Reset();
            m_imguiPipelineState.Reset();
            m_imguiVertexShader.Reset();
            m_imguiPixelShader.Reset();
            m_imguiInitialized = false;
        }

        void Clear(const float color[4], float depth, std::uint8_t stencil) override
        {
            if (!BeginFrameRecording())
            {
                return;
            }

            // DX12 需要显式资源状态切换。
            // back buffer 在 Present 之后处于 PRESENT，清屏/绘制前要切回 RENDER_TARGET。
            const D3D12_RESOURCE_BARRIER toRenderTarget = TransitionBarrier(
                m_renderTargets[m_frameIndex].Get(),
                D3D12_RESOURCE_STATE_PRESENT,
                D3D12_RESOURCE_STATE_RENDER_TARGET);
            m_commandList->ResourceBarrier(1, &toRenderTarget);

            const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = GetCurrentRtvHandle();
            const D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
            m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
            m_commandList->ClearRenderTargetView(rtvHandle, color, 0, nullptr);
            m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, depth, stencil, 0, nullptr);
            m_commandList->RSSetViewports(1, &m_viewport);
            m_commandList->RSSetScissorRects(1, &m_scissorRect);
            m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
        }

        void Present() override
        {
            if (!m_hasOpenCommandList)
            {
                return;
            }

            // 提交前切回 PRESENT，然后关闭命令列表并提交到队列。
            const D3D12_RESOURCE_BARRIER toPresent = TransitionBarrier(
                m_renderTargets[m_frameIndex].Get(),
                D3D12_RESOURCE_STATE_RENDER_TARGET,
                D3D12_RESOURCE_STATE_PRESENT);
            m_commandList->ResourceBarrier(1, &toPresent);

            m_commandList->Close();
            ID3D12CommandList* commandLists[] = { m_commandList.Get() };
            m_commandQueue->ExecuteCommandLists(1, commandLists);
            m_swapChain->Present(0, 0);
            MoveToNextFrame();
            m_hasOpenCommandList = false;
        }

        void SetViewport(const GraphicsViewport& viewport) override
        {
            m_viewport.TopLeftX = viewport.x;
            m_viewport.TopLeftY = viewport.y;
            m_viewport.Width = viewport.width;
            m_viewport.Height = viewport.height;
            m_viewport.MinDepth = viewport.minDepth;
            m_viewport.MaxDepth = viewport.maxDepth;

            m_scissorRect.left = static_cast<LONG>(viewport.x);
            m_scissorRect.top = static_cast<LONG>(viewport.y);
            m_scissorRect.right = static_cast<LONG>(viewport.x + viewport.width);
            m_scissorRect.bottom = static_cast<LONG>(viewport.y + viewport.height);
        }

        std::shared_ptr<IGraphicsBuffer> CreateBuffer(const GraphicsBufferDesc& desc, const void* initialData) override
        {
            const bool isConstantBuffer = HasAnyFlag(desc.bindFlags, GraphicsBufferBindFlags::ConstantBuffer);
            const UINT requestedSize = desc.byteWidth;
            const UINT resourceSize = isConstantBuffer ? AlignConstantBufferSize(requestedSize) : requestedSize;

            // 这里当前统一用 UPLOAD heap，优点是实现简单、CPU 可直接写。
            // 代价是性能不如 DEFAULT heap + staging/upload 的正式做法。
            D3D12_HEAP_PROPERTIES heapProperties = {};
            heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

            D3D12_RESOURCE_DESC resourceDesc = {};
            resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            resourceDesc.Width = resourceSize;
            resourceDesc.Height = 1;
            resourceDesc.DepthOrArraySize = 1;
            resourceDesc.MipLevels = 1;
            resourceDesc.SampleDesc.Count = 1;
            resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

            auto buffer = std::make_shared<Dx12GraphicsBuffer>();
            if (FAILED(m_device->CreateCommittedResource(
                &heapProperties,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&buffer->resource))))
            {
                return nullptr;
            }

            buffer->sizeInBytes = resourceSize;

            if (desc.cpuWrite || isConstantBuffer)
            {
                if (FAILED(buffer->resource->Map(0, nullptr, &buffer->mappedData)))
                {
                    return nullptr;
                }
            }

            if (initialData != nullptr)
            {
                if (buffer->mappedData == nullptr)
                {
                    void* mappedData = nullptr;
                    if (FAILED(buffer->resource->Map(0, nullptr, &mappedData)))
                    {
                        return nullptr;
                    }
                    std::memcpy(mappedData, initialData, requestedSize);
                    buffer->resource->Unmap(0, nullptr);
                }
                else
                {
                    std::memcpy(buffer->mappedData, initialData, requestedSize);
                }
            }

            return buffer;
        }

        void UpdateBuffer(IGraphicsBuffer& buffer, const void* data, std::size_t dataSize) override
        {
            Dx12GraphicsBuffer* dx12Buffer = CheckedCast<Dx12GraphicsBuffer>(&buffer);
            if (dx12Buffer->mappedData == nullptr)
            {
                void* mappedData = nullptr;
                if (FAILED(dx12Buffer->resource->Map(0, nullptr, &mappedData)))
                {
                    return;
                }
                std::memcpy(mappedData, data, dataSize);
                dx12Buffer->resource->Unmap(0, nullptr);
                return;
            }

            std::memcpy(dx12Buffer->mappedData, data, dataSize);
        }

        std::shared_ptr<IGraphicsShaderBytecode> CompileShader(const GraphicsShaderDesc& desc) override
        {
            auto bytecode = std::make_shared<Dx12ShaderBytecode>();
            const CompileShaderType shaderType =
                desc.stage == GraphicsShaderStage::Vertex ? CompileShaderType::VS :
                desc.stage == GraphicsShaderStage::Pixel ? CompileShaderType::PS :
                CompileShaderType::CS;

            if (FAILED(MoonCreateShaderFromFile(desc.filePath.c_str(), shaderType, bytecode->blob.GetAddressOf())))
            {
                return nullptr;
            }

            return bytecode;
        }

        std::shared_ptr<IGraphicsVertexShader> CreateVertexShader(
            const IGraphicsShaderBytecode& bytecode,
            const char*) override
        {
            const Dx12ShaderBytecode* dx12Bytecode = CheckedCast<const Dx12ShaderBytecode>(
                const_cast<IGraphicsShaderBytecode*>(&bytecode));
            auto shader = std::make_shared<Dx12VertexShader>();
            shader->blob = dx12Bytecode->blob;
            return shader;
        }

        std::shared_ptr<IGraphicsPixelShader> CreatePixelShader(
            const IGraphicsShaderBytecode& bytecode,
            const char*) override
        {
            const Dx12ShaderBytecode* dx12Bytecode = CheckedCast<const Dx12ShaderBytecode>(
                const_cast<IGraphicsShaderBytecode*>(&bytecode));
            auto shader = std::make_shared<Dx12PixelShader>();
            shader->blob = dx12Bytecode->blob;
            return shader;
        }

        std::shared_ptr<IGraphicsInputLayout> CreateInputLayout(
            const VertexLayoutDesc& layoutDesc,
            const IGraphicsShaderBytecode&,
            const char*) override
        {
            auto inputLayout = std::make_shared<Dx12InputLayout>();
            inputLayout->semanticNames.reserve(layoutDesc.attributeCount);
            inputLayout->elements.reserve(layoutDesc.attributeCount);

            for (std::uint32_t i = 0; i < layoutDesc.attributeCount; ++i)
            {
                const VertexAttributeDesc& attribute = layoutDesc.attributes[i];
                inputLayout->semanticNames.emplace_back(attribute.semanticName);

                D3D12_INPUT_ELEMENT_DESC element = {};
                element.SemanticName = inputLayout->semanticNames.back().c_str();
                element.SemanticIndex = attribute.semanticIndex;
                element.Format = ToDxgiFormat(attribute.format);
                element.InputSlot = 0;
                element.AlignedByteOffset = attribute.alignedByteOffset;
                element.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
                element.InstanceDataStepRate = 0;
                inputLayout->elements.push_back(element);
            }

            return inputLayout;
        }

        std::shared_ptr<IGraphicsRasterizerState> CreateRasterizerState(const GraphicsRasterizerDesc& desc) override
        {
            auto rasterizerState = std::make_shared<Dx12RasterizerState>();
            rasterizerState->desc.FillMode = ToD3D12FillMode(desc.fillMode);
            rasterizerState->desc.CullMode = ToD3D12CullMode(desc.cullMode);
            rasterizerState->desc.FrontCounterClockwise = desc.frontCounterClockwise;
            rasterizerState->desc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
            rasterizerState->desc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
            rasterizerState->desc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
            rasterizerState->desc.DepthClipEnable = desc.depthClipEnable;
            rasterizerState->desc.MultisampleEnable = FALSE;
            rasterizerState->desc.AntialiasedLineEnable = FALSE;
            rasterizerState->desc.ForcedSampleCount = 0;
            rasterizerState->desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
            return rasterizerState;
        }

        std::shared_ptr<IGraphicsDepthStencilState> CreateDepthStencilState(const GraphicsDepthStencilDesc& desc) override
        {
            auto depthStencilState = std::make_shared<Dx12DepthStencilState>();
            D3D12_DEPTH_STENCIL_DESC& dsDesc = depthStencilState->desc;
            dsDesc.DepthEnable = desc.depthEnable ? TRUE : FALSE;
            dsDesc.DepthWriteMask = (desc.depthWriteMask == GraphicsDepthWriteMask::All) ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
            dsDesc.DepthFunc = ToD3D12ComparisonFunc(desc.depthFunc);
            dsDesc.StencilEnable = desc.stencilEnable ? TRUE : FALSE;
            dsDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
            dsDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
            dsDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
            dsDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
            dsDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
            dsDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
            dsDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
            dsDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
            dsDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
            dsDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
            return depthStencilState;
        }

        void SetDepthStencilState(const IGraphicsDepthStencilState* depthStencilState) override
        {
            m_currentDepthStencilState = CheckedCast<const Dx12DepthStencilState>(
                const_cast<IGraphicsDepthStencilState*>(depthStencilState));
            m_pipelineDirty = true;
        }

        void SetVertexBuffer(const IGraphicsBuffer& buffer, std::uint32_t stride, std::uint32_t offset) override
        {
            Dx12GraphicsBuffer* dx12Buffer = CheckedCast<Dx12GraphicsBuffer>(const_cast<IGraphicsBuffer*>(&buffer));
            dx12Buffer->vertexView.BufferLocation = dx12Buffer->resource->GetGPUVirtualAddress() + offset;
            dx12Buffer->vertexView.SizeInBytes = dx12Buffer->sizeInBytes - offset;
            dx12Buffer->vertexView.StrideInBytes = stride;
            m_currentVertexBuffer = dx12Buffer;
        }

        void SetIndexBuffer(const IGraphicsBuffer& buffer, GraphicsIndexFormat format, std::uint32_t offset) override
        {
            Dx12GraphicsBuffer* dx12Buffer = CheckedCast<Dx12GraphicsBuffer>(const_cast<IGraphicsBuffer*>(&buffer));
            dx12Buffer->indexView.BufferLocation = dx12Buffer->resource->GetGPUVirtualAddress() + offset;
            dx12Buffer->indexView.SizeInBytes = dx12Buffer->sizeInBytes - offset;
            dx12Buffer->indexView.Format = ToDxgiIndexFormat(format);
            m_currentIndexBuffer = dx12Buffer;
        }

        void SetPrimitiveTopology(GraphicsPrimitiveTopology topology) override
        {
            m_currentTopology = topology;
        }

        void SetInputLayout(const IGraphicsInputLayout* inputLayout) override
        {
            m_currentInputLayout = CheckedCast<const Dx12InputLayout>(const_cast<IGraphicsInputLayout*>(inputLayout));
            m_pipelineDirty = true;
        }

        void SetVertexShader(const IGraphicsVertexShader* shader) override
        {
            m_currentVertexShader = CheckedCast<const Dx12VertexShader>(const_cast<IGraphicsVertexShader*>(shader));
            m_pipelineDirty = true;
        }

        void SetPixelShader(const IGraphicsPixelShader* shader) override
        {
            m_currentPixelShader = CheckedCast<const Dx12PixelShader>(const_cast<IGraphicsPixelShader*>(shader));
            m_pipelineDirty = true;
        }

        void SetVertexConstantBuffer(std::uint32_t slot, const IGraphicsBuffer* buffer) override
        {
            if (slot < m_vertexConstantBuffers.size())
            {
                m_vertexConstantBuffers[slot] = CheckedCast<const Dx12GraphicsBuffer>(const_cast<IGraphicsBuffer*>(buffer));
            }
        }

        void SetPixelConstantBuffer(std::uint32_t slot, const IGraphicsBuffer* buffer) override
        {
            if (slot < m_pixelConstantBuffers.size())
            {
                m_pixelConstantBuffers[slot] = CheckedCast<const Dx12GraphicsBuffer>(const_cast<IGraphicsBuffer*>(buffer));
            }
        }

        void SetRasterizerState(const IGraphicsRasterizerState* rasterizerState) override
        {
            m_currentRasterizerState = CheckedCast<const Dx12RasterizerState>(
                const_cast<IGraphicsRasterizerState*>(rasterizerState));
            m_pipelineDirty = true;
        }

        void DrawIndexed(std::uint32_t indexCount, std::uint32_t startIndexLocation, std::int32_t baseVertexLocation) override
        {
            if (!BeginFrameRecording())
            {
                return;
            }

            // DX12 娌℃湁鈥滀复鏃剁姸鎬佹満鑷姩鎷艰绠＄嚎鈥濊繖涓€灞傦紝
            // 所以 draw 前要确保当前 input layout / shader / rasterizer 对应的 PSO 已经准备好。
            if (!EnsurePipelineState())
            {
                return;
            }

            m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
            m_commandList->SetPipelineState(m_pipelineState.Get());
            m_commandList->RSSetViewports(1, &m_viewport);
            m_commandList->RSSetScissorRects(1, &m_scissorRect);
            m_commandList->IASetPrimitiveTopology(ToD3D12PrimitiveTopology(m_currentTopology));

            if (m_currentVertexBuffer != nullptr)
            {
                m_commandList->IASetVertexBuffers(0, 1, &m_currentVertexBuffer->vertexView);
            }
            if (m_currentIndexBuffer != nullptr)
            {
                m_commandList->IASetIndexBuffer(&m_currentIndexBuffer->indexView);
            }
            if (m_vertexConstantBuffers[0] != nullptr)
            {
                m_commandList->SetGraphicsRootConstantBufferView(0, m_vertexConstantBuffers[0]->resource->GetGPUVirtualAddress());
            }
            if (m_pixelConstantBuffers[1] != nullptr)
            {
                m_commandList->SetGraphicsRootConstantBufferView(1, m_pixelConstantBuffers[1]->resource->GetGPUVirtualAddress());
            }

            m_commandList->DrawIndexedInstanced(indexCount, 1, startIndexLocation, baseVertexLocation, 0);
        }

    private:
        bool CreateImGuiDeviceObjects()
        {
            // ImGui 在 DX12 下最核心的设备对象有四类：
            // 1. Shader-visible SRV heap
            // 2. 涓撶敤 root signature
            // 3. 涓撶敤 PSO
            // 4. 瀛椾綋绾圭悊鍙婂叾 SRV
            if (!CreateImGuiDescriptorHeap())
            {
                return false;
            }

            if (!CreateImGuiPipelineState())
            {
                return false;
            }

            return CreateImGuiFontTexture();
        }

        bool CreateImGuiDescriptorHeap()
        {
            D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
            heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            heapDesc.NumDescriptors = 1;
            heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            return SUCCEEDED(m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_imguiSrvHeap)));
        }

        bool CreateImGuiPipelineState()
        {
            // 这里直接内嵌了一套最小 HLSL，而不是依赖外部文件。
            // 这样 ImGui backend 自己就能自洽，不影响主工程原有 shader 组织方式。
            static constexpr char kImGuiVertexShaderSource[] =
                "cbuffer vertexBuffer : register(b0) { float4x4 ProjectionMatrix; };"
                "struct VS_INPUT { float2 pos : POSITION; float4 col : COLOR0; float2 uv : TEXCOORD0; };"
                "struct PS_INPUT { float4 pos : SV_POSITION; float4 col : COLOR0; float2 uv : TEXCOORD0; };"
                "PS_INPUT VS(VS_INPUT input) {"
                "  PS_INPUT output;"
                "  output.pos = mul(ProjectionMatrix, float4(input.pos.xy, 0.0f, 1.0f));"
                "  output.col = input.col;"
                "  output.uv = input.uv;"
                "  return output;"
                "}";

            static constexpr char kImGuiPixelShaderSource[] =
                "SamplerState sampler0 : register(s0);"
                "Texture2D texture0 : register(t0);"
                "struct PS_INPUT { float4 pos : SV_POSITION; float4 col : COLOR0; float2 uv : TEXCOORD0; };"
                "float4 PS(PS_INPUT input) : SV_Target {"
                "  return input.col * texture0.Sample(sampler0, input.uv);"
                "}";

            if (!CompileImGuiShader(kImGuiVertexShaderSource, "VS", "vs_5_0", m_imguiVertexShader.ReleaseAndGetAddressOf()))
            {
                return false;
            }

            if (!CompileImGuiShader(kImGuiPixelShaderSource, "PS", "ps_5_0", m_imguiPixelShader.ReleaseAndGetAddressOf()))
            {
                return false;
            }

            D3D12_DESCRIPTOR_RANGE descriptorRange = {};
            descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            descriptorRange.NumDescriptors = 1;
            descriptorRange.BaseShaderRegister = 0;
            descriptorRange.RegisterSpace = 0;
            descriptorRange.OffsetInDescriptorsFromTableStart = 0;

            D3D12_ROOT_PARAMETER rootParameters[2] = {};
            // root parameter 0: 鐩存帴濉?4x4 鎶曞奖鐭╅樀甯搁噺
            rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
            rootParameters[0].Constants.ShaderRegister = 0;
            rootParameters[0].Constants.RegisterSpace = 0;
            rootParameters[0].Constants.Num32BitValues = 16;
            rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

            // root parameter 1: 字体/贴图使用的 SRV 描述符表
            rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
            rootParameters[1].DescriptorTable.pDescriptorRanges = &descriptorRange;
            rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

            D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
            samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            samplerDesc.MipLODBias = 0.0f;
            samplerDesc.MaxAnisotropy = 0;
            samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
            samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
            samplerDesc.MinLOD = 0.0f;
            samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
            samplerDesc.ShaderRegister = 0;
            samplerDesc.RegisterSpace = 0;
            samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

            D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
            rootSignatureDesc.NumParameters = _countof(rootParameters);
            rootSignatureDesc.pParameters = rootParameters;
            rootSignatureDesc.NumStaticSamplers = 1;
            rootSignatureDesc.pStaticSamplers = &samplerDesc;
            rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

            ComPtr<ID3DBlob> serializedRootSignature;
            ComPtr<ID3DBlob> errorBlob;
            if (FAILED(D3D12SerializeRootSignature(
                &rootSignatureDesc,
                D3D_ROOT_SIGNATURE_VERSION_1,
                &serializedRootSignature,
                &errorBlob)))
            {
                if (errorBlob != nullptr)
                {
                    OutputDebugStringA(static_cast<const char*>(errorBlob->GetBufferPointer()));
                }
                return false;
            }

            if (FAILED(m_device->CreateRootSignature(
                0,
                serializedRootSignature->GetBufferPointer(),
                serializedRootSignature->GetBufferSize(),
                IID_PPV_ARGS(&m_imguiRootSignature))))
            {
                return false;
            }

            const D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
                // ImGui 椤剁偣鏍煎紡鍥哄畾灏辨槸 pos / uv / color
                { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, static_cast<UINT>(IM_OFFSETOF(ImDrawVert, pos)), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
                { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, static_cast<UINT>(IM_OFFSETOF(ImDrawVert, uv)), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
                { "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, static_cast<UINT>(IM_OFFSETOF(ImDrawVert, col)), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
            };

            D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc = {};
            pipelineDesc.pRootSignature = m_imguiRootSignature.Get();
            pipelineDesc.VS = { m_imguiVertexShader->GetBufferPointer(), m_imguiVertexShader->GetBufferSize() };
            pipelineDesc.PS = { m_imguiPixelShader->GetBufferPointer(), m_imguiPixelShader->GetBufferSize() };
            pipelineDesc.BlendState.AlphaToCoverageEnable = FALSE;
            pipelineDesc.BlendState.IndependentBlendEnable = FALSE;
            D3D12_RENDER_TARGET_BLEND_DESC& renderTargetBlendDesc = pipelineDesc.BlendState.RenderTarget[0];
            renderTargetBlendDesc.BlendEnable = TRUE;
            renderTargetBlendDesc.LogicOpEnable = FALSE;
            renderTargetBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
            renderTargetBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
            renderTargetBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
            renderTargetBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
            renderTargetBlendDesc.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
            renderTargetBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
            renderTargetBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
            renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
            pipelineDesc.SampleMask = UINT_MAX;
            pipelineDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
            pipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
            pipelineDesc.RasterizerState.FrontCounterClockwise = FALSE;
            pipelineDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
            pipelineDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
            pipelineDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
            pipelineDesc.RasterizerState.DepthClipEnable = TRUE;
            pipelineDesc.RasterizerState.MultisampleEnable = FALSE;
            pipelineDesc.RasterizerState.AntialiasedLineEnable = FALSE;
            pipelineDesc.RasterizerState.ForcedSampleCount = 0;
            pipelineDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
            pipelineDesc.DepthStencilState.DepthEnable = FALSE;
            pipelineDesc.DepthStencilState.StencilEnable = FALSE;
            pipelineDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
            pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            pipelineDesc.NumRenderTargets = 1;
            pipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
            pipelineDesc.SampleDesc.Count = 1;

            return SUCCEEDED(m_device->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&m_imguiPipelineState)));
        }

        bool CompileImGuiShader(
            const char* source,
            const char* entryPoint,
            const char* target,
            ID3DBlob** blob)
        {
            UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
            compileFlags |= D3DCOMPILE_DEBUG;
            compileFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

            ComPtr<ID3DBlob> errorBlob;
            if (FAILED(D3DCompile(
                source,
                std::strlen(source),
                nullptr,
                nullptr,
                nullptr,
                entryPoint,
                target,
                compileFlags,
                0,
                blob,
                &errorBlob)))
            {
                if (errorBlob != nullptr)
                {
                    OutputDebugStringA(static_cast<const char*>(errorBlob->GetBufferPointer()));
                }
                return false;
            }

            return true;
        }

        bool CreateImGuiFontTexture()
        {
            ImGuiIO& io = ImGui::GetIO();
            unsigned char* pixels = nullptr;
            int width = 0;
            int height = 0;
            io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
            if (pixels == nullptr || width <= 0 || height <= 0)
            {
                return false;
            }

            // 字体纹理放在 DEFAULT heap，真正给像素数据时通过 upload buffer 中转。
            D3D12_HEAP_PROPERTIES textureHeapProperties = {};
            textureHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

            D3D12_RESOURCE_DESC textureDesc = {};
            textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            textureDesc.Width = static_cast<UINT64>(width);
            textureDesc.Height = static_cast<UINT>(height);
            textureDesc.DepthOrArraySize = 1;
            textureDesc.MipLevels = 1;
            textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            textureDesc.SampleDesc.Count = 1;
            textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

            if (FAILED(m_device->CreateCommittedResource(
                &textureHeapProperties,
                D3D12_HEAP_FLAG_NONE,
                &textureDesc,
                D3D12_RESOURCE_STATE_COPY_DEST,
                nullptr,
                IID_PPV_ARGS(&m_imguiFontTexture))))
            {
                return false;
            }

            // GetCopyableFootprints 浼氬憡璇夋垜浠細
            // 这张纹理拷贝到 buffer 时，每行需要怎样对齐，整个 upload buffer 需要多大。
            UINT64 uploadBufferSize = 0;
            D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout = {};
            UINT numRows = 0;
            UINT64 rowSizeInBytes = 0;
            m_device->GetCopyableFootprints(&textureDesc, 0, 1, 0, &layout, &numRows, &rowSizeInBytes, &uploadBufferSize);

            D3D12_HEAP_PROPERTIES uploadHeapProperties = {};
            uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

            D3D12_RESOURCE_DESC uploadDesc = {};
            uploadDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            uploadDesc.Width = uploadBufferSize;
            uploadDesc.Height = 1;
            uploadDesc.DepthOrArraySize = 1;
            uploadDesc.MipLevels = 1;
            uploadDesc.SampleDesc.Count = 1;
            uploadDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

            if (FAILED(m_device->CreateCommittedResource(
                &uploadHeapProperties,
                D3D12_HEAP_FLAG_NONE,
                &uploadDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&m_imguiFontUpload))))
            {
                return false;
            }

            std::uint8_t* mappedData = nullptr;
            D3D12_RANGE readRange = { 0, 0 };
            if (FAILED(m_imguiFontUpload->Map(0, &readRange, reinterpret_cast<void**>(&mappedData))))
            {
                return false;
            }

            const std::size_t sourceRowPitch = static_cast<std::size_t>(width) * 4u;
            for (UINT row = 0; row < numRows; ++row)
            {
                // 纹理上传不是简单的一次 memcpy，因为 D3D12 对每行 pitch 有对齐要求。
                std::memcpy(
                    mappedData + row * layout.Footprint.RowPitch,
                    pixels + row * sourceRowPitch,
                    sourceRowPitch);
            }
            m_imguiFontUpload->Unmap(0, nullptr);

            if (!ResetCommandListForUpload())
            {
                return false;
            }

            D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
            dstLocation.pResource = m_imguiFontTexture.Get();
            dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
            dstLocation.SubresourceIndex = 0;

            D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
            srcLocation.pResource = m_imguiFontUpload.Get();
            srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
            srcLocation.PlacedFootprint = layout;

            // 先从 upload buffer 复制到默认堆纹理，再切到可供像素着色器读取的状态。
            m_commandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);

            const D3D12_RESOURCE_BARRIER shaderResourceBarrier = TransitionBarrier(
                m_imguiFontTexture.Get(),
                D3D12_RESOURCE_STATE_COPY_DEST,
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            m_commandList->ResourceBarrier(1, &shaderResourceBarrier);

            if (!ExecuteUploadCommandList())
            {
                return false;
            }

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Texture2D.MipLevels = 1;
            m_device->CreateShaderResourceView(
                m_imguiFontTexture.Get(),
                &srvDesc,
                m_imguiSrvHeap->GetCPUDescriptorHandleForHeapStart());

            io.Fonts->SetTexID(reinterpret_cast<ImTextureID>(m_imguiSrvHeap->GetGPUDescriptorHandleForHeapStart().ptr));
            return true;
        }

        bool EnsureImGuiFrameResources(const ImDrawData* drawData)
        {
            if (drawData == nullptr)
            {
                return false;
            }

            Dx12ImGuiFrameResources& frameResources = m_imguiFrameResources[m_frameIndex];
            const int requiredVertexCount = drawData->TotalVtxCount > 0 ? drawData->TotalVtxCount : 1;
            const int requiredIndexCount = drawData->TotalIdxCount > 0 ? drawData->TotalIdxCount : 1;

            if (!EnsureImGuiUploadBuffer(
                requiredVertexCount,
                sizeof(ImDrawVert),
                frameResources.vertexBuffer,
                reinterpret_cast<void**>(&frameResources.mappedVertexData),
                frameResources.vertexBufferCapacity))
            {
                return false;
            }

            if (!EnsureImGuiUploadBuffer(
                requiredIndexCount,
                sizeof(ImDrawIdx),
                frameResources.indexBuffer,
                reinterpret_cast<void**>(&frameResources.mappedIndexData),
                frameResources.indexBufferCapacity))
            {
                return false;
            }

            return true;
        }

        bool EnsureImGuiUploadBuffer(
            int requiredElementCount,
            std::size_t elementSize,
            ComPtr<ID3D12Resource>& buffer,
            void** mappedData,
            int& capacity)
        {
            if (buffer != nullptr && capacity >= requiredElementCount)
            {
                return true;
            }

            // 容量按 1.5 倍扩容，避免窗口里控件一多时每帧都重新分配 upload buffer。
            capacity = requiredElementCount + requiredElementCount / 2 + 256;
            buffer.Reset();
            *mappedData = nullptr;

            D3D12_HEAP_PROPERTIES heapProperties = {};
            heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

            D3D12_RESOURCE_DESC resourceDesc = {};
            resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            resourceDesc.Width = static_cast<UINT64>(capacity) * static_cast<UINT64>(elementSize);
            resourceDesc.Height = 1;
            resourceDesc.DepthOrArraySize = 1;
            resourceDesc.MipLevels = 1;
            resourceDesc.SampleDesc.Count = 1;
            resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

            if (FAILED(m_device->CreateCommittedResource(
                &heapProperties,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&buffer))))
            {
                return false;
            }

            D3D12_RANGE readRange = { 0, 0 };
            return SUCCEEDED(buffer->Map(0, &readRange, mappedData));
        }

        void UploadImGuiDrawData(const ImDrawData& drawData, Dx12ImGuiFrameResources& frameResources)
        {
            ImDrawVert* vertexDst = frameResources.mappedVertexData;
            ImDrawIdx* indexDst = frameResources.mappedIndexData;
            // ImGui 最终给我们的就是一串已经拍平的 draw list。
            // 这里做的事情很朴素：把 CPU 内存里的顶点/索引顺序拷到 GPU 可见的 upload buffer。
            for (int listIndex = 0; listIndex < drawData.CmdListsCount; ++listIndex)
            {
                const ImDrawList* cmdList = drawData.CmdLists[listIndex];
                std::memcpy(vertexDst, cmdList->VtxBuffer.Data, static_cast<std::size_t>(cmdList->VtxBuffer.Size) * sizeof(ImDrawVert));
                std::memcpy(indexDst, cmdList->IdxBuffer.Data, static_cast<std::size_t>(cmdList->IdxBuffer.Size) * sizeof(ImDrawIdx));
                vertexDst += cmdList->VtxBuffer.Size;
                indexDst += cmdList->IdxBuffer.Size;
            }
        }

        void RecordImGuiDrawCommands(const ImDrawData& drawData, const Dx12ImGuiFrameResources& frameResources)
        {
            // ImGui 仍然走标准三角形列表，只是它的顶点格式和主场景不同。
            D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
            vertexBufferView.BufferLocation = frameResources.vertexBuffer->GetGPUVirtualAddress();
            vertexBufferView.SizeInBytes = frameResources.vertexBufferCapacity * static_cast<UINT>(sizeof(ImDrawVert));
            vertexBufferView.StrideInBytes = sizeof(ImDrawVert);

            D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
            indexBufferView.BufferLocation = frameResources.indexBuffer->GetGPUVirtualAddress();
            indexBufferView.SizeInBytes = frameResources.indexBufferCapacity * static_cast<UINT>(sizeof(ImDrawIdx));
            indexBufferView.Format = sizeof(ImDrawIdx) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

            const float left = drawData.DisplayPos.x;
            const float right = drawData.DisplayPos.x + drawData.DisplaySize.x;
            const float top = drawData.DisplayPos.y;
            const float bottom = drawData.DisplayPos.y + drawData.DisplaySize.y;
            const float projectionMatrix[4][4] = {
                // 这里构造的是一个 2D 正交投影矩阵，把 ImGui 的屏幕空间坐标直接映射到 NDC。
                { 2.0f / (right - left), 0.0f, 0.0f, 0.0f },
                { 0.0f, 2.0f / (top - bottom), 0.0f, 0.0f },
                { 0.0f, 0.0f, 0.5f, 0.0f },
                { (right + left) / (left - right), (top + bottom) / (bottom - top), 0.5f, 1.0f }
            };

            const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = GetCurrentRtvHandle();
            m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

            D3D12_VIEWPORT viewport = {};
            viewport.Width = drawData.DisplaySize.x;
            viewport.Height = drawData.DisplaySize.y;
            viewport.MinDepth = 0.0f;
            viewport.MaxDepth = 1.0f;
            m_commandList->RSSetViewports(1, &viewport);

            ID3D12DescriptorHeap* descriptorHeaps[] = { m_imguiSrvHeap.Get() };
            m_commandList->SetDescriptorHeaps(1, descriptorHeaps);
            m_commandList->SetGraphicsRootSignature(m_imguiRootSignature.Get());
            m_commandList->SetPipelineState(m_imguiPipelineState.Get());
            m_commandList->SetGraphicsRoot32BitConstants(0, 16, projectionMatrix, 0);
            m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            m_commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
            m_commandList->IASetIndexBuffer(&indexBufferView);

            int globalVertexOffset = 0;
            int globalIndexOffset = 0;
            for (int listIndex = 0; listIndex < drawData.CmdListsCount; ++listIndex)
            {
                const ImDrawList* cmdList = drawData.CmdLists[listIndex];
                for (int cmdIndex = 0; cmdIndex < cmdList->CmdBuffer.Size; ++cmdIndex)
                {
                    const ImDrawCmd& drawCommand = cmdList->CmdBuffer[cmdIndex];
                    if (drawCommand.UserCallback != nullptr)
                    {
                        drawCommand.UserCallback(cmdList, &drawCommand);
                        continue;
                    }

                    // ImGui 的每条命令都带一个裁剪矩形，这里把它翻译成 D3D12 的 scissor rect。
                    const ImVec2 clipMin(
                        std::max(0.0f, (drawCommand.ClipRect.x - drawData.DisplayPos.x) * drawData.FramebufferScale.x),
                        std::max(0.0f, (drawCommand.ClipRect.y - drawData.DisplayPos.y) * drawData.FramebufferScale.y));
                    const ImVec2 clipMax(
                        std::min(drawData.DisplaySize.x, (drawCommand.ClipRect.z - drawData.DisplayPos.x) * drawData.FramebufferScale.x),
                        std::min(drawData.DisplaySize.y, (drawCommand.ClipRect.w - drawData.DisplayPos.y) * drawData.FramebufferScale.y));
                    if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y)
                    {
                        continue;
                    }

                    D3D12_RECT scissorRect = {};
                    scissorRect.left = static_cast<LONG>(clipMin.x);
                    scissorRect.top = static_cast<LONG>(clipMin.y);
                    scissorRect.right = static_cast<LONG>(clipMax.x);
                    scissorRect.bottom = static_cast<LONG>(clipMax.y);
                    m_commandList->RSSetScissorRects(1, &scissorRect);

                    D3D12_GPU_DESCRIPTOR_HANDLE textureHandle = m_imguiSrvHeap->GetGPUDescriptorHandleForHeapStart();
                    if (drawCommand.TextureId != nullptr)
                    {
                        textureHandle.ptr = static_cast<UINT64>(reinterpret_cast<std::uintptr_t>(drawCommand.TextureId));
                    }
                    m_commandList->SetGraphicsRootDescriptorTable(1, textureHandle);
                    m_commandList->DrawIndexedInstanced(
                        static_cast<UINT>(drawCommand.ElemCount),
                        1,
                        static_cast<UINT>(drawCommand.IdxOffset + globalIndexOffset),
                        static_cast<INT>(drawCommand.VtxOffset + globalVertexOffset),
                        0);
                }
                globalIndexOffset += cmdList->IdxBuffer.Size;
                globalVertexOffset += cmdList->VtxBuffer.Size;
            }

            m_commandList->RSSetViewports(1, &m_viewport);
            m_commandList->RSSetScissorRects(1, &m_scissorRect);
        }

        bool ResetCommandListForUpload()
        {
            // 这条路径专门服务于一次性的上传命令，例如字体纹理初始化。
            // 和常规每帧录制不同，这里会立即 reset / record / execute / wait。
            m_hasOpenCommandList = false;

            if (FAILED(m_commandAllocators[m_frameIndex]->Reset()))
            {
                return false;
            }

            return SUCCEEDED(m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), nullptr));
        }

        bool ExecuteUploadCommandList()
        {
            if (FAILED(m_commandList->Close()))
            {
                return false;
            }

            ID3D12CommandList* commandLists[] = { m_commandList.Get() };
            m_commandQueue->ExecuteCommandLists(1, commandLists);
            WaitForGpu();
            return true;
        }

        bool CreateDeviceObjects()
        {
            UINT factoryFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
            ComPtr<ID3D12Debug> debugController;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
            {
                // 有 debug layer 时再打开 DXGI debug factory，避免没装调试组件时初始化直接失败。
                debugController->EnableDebugLayer();
                factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
            }
#endif

            ComPtr<IDXGIFactory4> factory;
            if (FAILED(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&factory))))
            {
                return false;
            }
            m_factory = factory;

            if (!CreateDevice(factory.Get()))
            {
                return false;
            }

            // DX12 的所有 GPU 工作都通过 command queue 提交。
            D3D12_COMMAND_QUEUE_DESC queueDesc = {};
            queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            if (FAILED(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue))))
            {
                return false;
            }

            DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
            swapChainDesc.BufferCount = kFrameCount;
            swapChainDesc.Width = m_width;
            swapChainDesc.Height = m_height;
            swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            swapChainDesc.SampleDesc.Count = 1;

            ComPtr<IDXGISwapChain1> swapChain1;
            if (FAILED(factory->CreateSwapChainForHwnd(
                m_commandQueue.Get(),
                m_hwnd,
                &swapChainDesc,
                nullptr,
                nullptr,
                &swapChain1)))
            {
                return false;
            }

            // 禁止 Alt+Enter 这类 DXGI 默认行为，窗口模式切换逻辑统一由应用自己控制。
            factory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER | DXGI_MWA_NO_WINDOW_CHANGES);
            if (FAILED(swapChain1.As(&m_swapChain)))
            {
                return false;
            }
            m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

            D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
            rtvHeapDesc.NumDescriptors = kFrameCount;
            rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            if (FAILED(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap))))
            {
                return false;
            }
            m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

            D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
            dsvHeapDesc.NumDescriptors = 1;
            dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
            if (FAILED(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap))))
            {
                return false;
            }

            for (UINT i = 0; i < kFrameCount; ++i)
            {
                if (FAILED(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[i]))))
                {
                    return false;
                }
            }

            if (FAILED(m_device->CreateCommandList(
                0,
                D3D12_COMMAND_LIST_TYPE_DIRECT,
                m_commandAllocators[m_frameIndex].Get(),
                nullptr,
                IID_PPV_ARGS(&m_commandList))))
            {
                return false;
            }
            m_commandList->Close();

            if (!CreateRootSignature())
            {
                return false;
            }

            // fence 是 DX12 CPU/GPU 同步的核心原语。
            if (FAILED(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence))))
            {
                return false;
            }

            m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            if (m_fenceEvent == nullptr)
            {
                return false;
            }

            m_fenceValues.fill(0);
            m_fenceValue = 1;
            return true;
        }

        bool CreateDevice(IDXGIFactory4* factory)
        {
            ComPtr<IDXGIAdapter1> hardwareAdapter;
            for (UINT adapterIndex = 0;
                 factory->EnumAdapters1(adapterIndex, &hardwareAdapter) != DXGI_ERROR_NOT_FOUND;
                 ++adapterIndex)
            {
                DXGI_ADAPTER_DESC1 desc = {};
                hardwareAdapter->GetDesc1(&desc);
                if ((desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0)
                {
                    continue;
                }

                // 先尝试真实硬件适配器；成功后就停止枚举。
                if (SUCCEEDED(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device))))
                {
                    return true;
                }
            }

            // 硬件不支持时回退到 WARP，至少能保证功能路径可运行。
            ComPtr<IDXGIAdapter> warpAdapter;
            if (FAILED(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter))))
            {
                return false;
            }

            return SUCCEEDED(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)));
        }

        bool CreateRootSignature()
        {
            // 当前主渲染路径很简单：VS 用 b0，PS 用 b1。
            // 所以根签名只放两个 CBV，保持最小可用。
            D3D12_ROOT_PARAMETER rootParameters[2] = {};
            rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
            rootParameters[0].Descriptor.ShaderRegister = 0;
            rootParameters[0].Descriptor.RegisterSpace = 0;
            rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

            rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
            rootParameters[1].Descriptor.ShaderRegister = 1;
            rootParameters[1].Descriptor.RegisterSpace = 0;
            rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

            D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
            rootSignatureDesc.NumParameters = _countof(rootParameters);
            rootSignatureDesc.pParameters = rootParameters;
            rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

            ComPtr<ID3DBlob> serializedRootSignature;
            ComPtr<ID3DBlob> errorBlob;
            if (FAILED(D3D12SerializeRootSignature(
                &rootSignatureDesc,
                D3D_ROOT_SIGNATURE_VERSION_1,
                &serializedRootSignature,
                &errorBlob)))
            {
                return false;
            }

            return SUCCEEDED(m_device->CreateRootSignature(
                0,
                serializedRootSignature->GetBufferPointer(),
                serializedRootSignature->GetBufferSize(),
                IID_PPV_ARGS(&m_rootSignature)));
        }

        bool EnsurePipelineState()
        {
            if (!m_pipelineDirty && m_pipelineState != nullptr)
            {
                return true;
            }

            if (m_currentInputLayout == nullptr || m_currentVertexShader == nullptr || m_currentPixelShader == nullptr)
            {
                return false;
            }

            // DX12 鐨?PSO 鏄€滃ぇ瀵硅薄鈥濓紝鎶?shader / rasterizer / blend / depth-stencil /
            // input layout / render target format 等状态一次性打包固定。
            D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc = {};
            pipelineDesc.pRootSignature = m_rootSignature.Get();
            pipelineDesc.VS = {
                m_currentVertexShader->blob->GetBufferPointer(),
                m_currentVertexShader->blob->GetBufferSize()
            };
            pipelineDesc.PS = {
                m_currentPixelShader->blob->GetBufferPointer(),
                m_currentPixelShader->blob->GetBufferSize()
            };
            pipelineDesc.BlendState.AlphaToCoverageEnable = FALSE;
            pipelineDesc.BlendState.IndependentBlendEnable = FALSE;
            const D3D12_RENDER_TARGET_BLEND_DESC defaultBlendDesc = {
                FALSE,FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP,
                D3D12_COLOR_WRITE_ENABLE_ALL
            };
            for (auto& renderTarget : pipelineDesc.BlendState.RenderTarget)
            {
                renderTarget = defaultBlendDesc;
            }
            pipelineDesc.SampleMask = UINT_MAX;
            pipelineDesc.RasterizerState = m_currentRasterizerState != nullptr
                ? m_currentRasterizerState->desc
                : DefaultRasterizerDesc();
            pipelineDesc.DepthStencilState = m_currentDepthStencilState != nullptr
                ? m_currentDepthStencilState->desc
                : DefaultDepthStencilDesc();
            pipelineDesc.InputLayout = {
                m_currentInputLayout->elements.data(),
                static_cast<UINT>(m_currentInputLayout->elements.size())
            };
            pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            pipelineDesc.NumRenderTargets = 1;
            pipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
            pipelineDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
            pipelineDesc.SampleDesc.Count = 1;

            m_pipelineState.Reset();
            if (FAILED(m_device->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&m_pipelineState))))
            {
                return false;
            }

            m_pipelineDirty = false;
            return true;
        }

        bool BeginFrameRecording()
        {
            if (m_hasOpenCommandList)
            {
                return true;
            }

            // 每帧开始时先 reset allocator，再 reset command list。
            // allocator 存命令内存，command list 存本帧真正要执行的命令。
            if (FAILED(m_commandAllocators[m_frameIndex]->Reset()))
            {
                return false;
            }
            if (FAILED(m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), nullptr)))
            {
                return false;
            }

            m_hasOpenCommandList = true;
            return true;
        }

        void WaitForGpu()
        {
            if (m_commandQueue == nullptr || m_fence == nullptr || m_fenceEvent == nullptr)
            {
                return;
            }

            // Signal 一个新的 fence 值，然后在 CPU 侧阻塞等待。
            // 这是最直接、也最容易理解的同步方式。
            const UINT64 fenceToWaitFor = m_fenceValue++;
            m_commandQueue->Signal(m_fence.Get(), fenceToWaitFor);
            m_fence->SetEventOnCompletion(fenceToWaitFor, m_fenceEvent);
            WaitForSingleObject(m_fenceEvent, INFINITE);
            m_frameIndex = m_swapChain != nullptr ? m_swapChain->GetCurrentBackBufferIndex() : 0;
        }

        void MoveToNextFrame()
        {
            // Present 后给当前 frame 打一个 fence 标记，
            // 下一次轮回到这个 back buffer 时，先确认 GPU 已经处理完旧内容。
            const UINT64 currentFenceValue = m_fenceValue++;
            m_commandQueue->Signal(m_fence.Get(), currentFenceValue);
            m_fenceValues[m_frameIndex] = currentFenceValue;

            m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
            if (m_fence->GetCompletedValue() < m_fenceValues[m_frameIndex])
            {
                m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent);
                WaitForSingleObject(m_fenceEvent, INFINITE);
            }
        }

        D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRtvHandle() const
        {
            D3D12_CPU_DESCRIPTOR_HANDLE handle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
            handle.ptr += static_cast<SIZE_T>(m_frameIndex) * static_cast<SIZE_T>(m_rtvDescriptorSize);
            return handle;
        }

        static D3D12_RESOURCE_BARRIER TransitionBarrier(
            ID3D12Resource* resource,
            D3D12_RESOURCE_STATES before,
            D3D12_RESOURCE_STATES after)
        {
            D3D12_RESOURCE_BARRIER barrier = {};
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Transition.pResource = resource;
            barrier.Transition.StateBefore = before;
            barrier.Transition.StateAfter = after;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            return barrier;
        }

        static D3D12_RASTERIZER_DESC DefaultRasterizerDesc()
        {
            D3D12_RASTERIZER_DESC desc = {};
            desc.FillMode = D3D12_FILL_MODE_SOLID;
            desc.CullMode = D3D12_CULL_MODE_BACK;
            desc.FrontCounterClockwise = FALSE;
            desc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
            desc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
            desc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
            desc.DepthClipEnable = TRUE;
            desc.MultisampleEnable = FALSE;
            desc.AntialiasedLineEnable = FALSE;
            desc.ForcedSampleCount = 0;
            desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
            return desc;
        }

        static D3D12_DEPTH_STENCIL_DESC DefaultDepthStencilDesc()
        {
            D3D12_DEPTH_STENCIL_DESC desc = {};
            desc.DepthEnable = TRUE;
            desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
            desc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
            desc.StencilEnable = TRUE;
            desc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
            desc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
            desc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
            desc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_INCR;
            desc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
            desc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
            desc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
            desc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_DECR;
            desc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
            desc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
            return desc;
        }

    private:
        HWND m_hwnd = nullptr;
        int m_width = 0;
        int m_height = 0;
        bool m_enable4xMsaa = false;
        bool m_hasOpenCommandList = false;
        bool m_pipelineDirty = true;
        bool m_imguiInitialized = false;

        ComPtr<IDXGIFactory4> m_factory;
        ComPtr<ID3D12Device> m_device;
        ComPtr<ID3D12CommandQueue> m_commandQueue;
        ComPtr<IDXGISwapChain3> m_swapChain;
        ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
        ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
        ComPtr<ID3D12CommandAllocator> m_commandAllocators[kFrameCount];
        ComPtr<ID3D12GraphicsCommandList> m_commandList;
        ComPtr<ID3D12Fence> m_fence;
        ComPtr<ID3D12RootSignature> m_rootSignature;
        ComPtr<ID3D12PipelineState> m_pipelineState;
        ComPtr<ID3D12Resource> m_renderTargets[kFrameCount];
        ComPtr<ID3D12Resource> m_depthStencil;
        ComPtr<ID3D12DescriptorHeap> m_imguiSrvHeap;
        ComPtr<ID3D12RootSignature> m_imguiRootSignature;
        ComPtr<ID3D12PipelineState> m_imguiPipelineState;
        ComPtr<ID3DBlob> m_imguiVertexShader;
        ComPtr<ID3DBlob> m_imguiPixelShader;
        ComPtr<ID3D12Resource> m_imguiFontTexture;
        ComPtr<ID3D12Resource> m_imguiFontUpload;

        UINT m_rtvDescriptorSize = 0;
        UINT m_frameIndex = 0;
        UINT64 m_fenceValue = 0;
        std::array<UINT64, kFrameCount> m_fenceValues = {};
        HANDLE m_fenceEvent = nullptr;

        D3D12_VIEWPORT m_viewport = {};
        D3D12_RECT m_scissorRect = {};

        Dx12GraphicsBuffer* m_currentVertexBuffer = nullptr;
        Dx12GraphicsBuffer* m_currentIndexBuffer = nullptr;
        const Dx12InputLayout* m_currentInputLayout = nullptr;
        const Dx12VertexShader* m_currentVertexShader = nullptr;
        const Dx12PixelShader* m_currentPixelShader = nullptr;
        const Dx12RasterizerState* m_currentRasterizerState = nullptr;
        const Dx12DepthStencilState* m_currentDepthStencilState = nullptr;
        GraphicsPrimitiveTopology m_currentTopology = GraphicsPrimitiveTopology::TriangleList;
        std::array<const Dx12GraphicsBuffer*, 8> m_vertexConstantBuffers = {};
        std::array<const Dx12GraphicsBuffer*, 8> m_pixelConstantBuffers = {};
        std::array<Dx12ImGuiFrameResources, kFrameCount> m_imguiFrameResources = {};
    };
}

std::unique_ptr<IGraphicsBackend> CreateDx12Backend()
{
    return std::make_unique<Dx12Backend>();
}

