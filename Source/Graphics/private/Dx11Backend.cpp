#include "Source/Graphics/public/GraphicsBackend.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "D3DCompiler.lib")
#pragma comment(lib, "winmm.lib")

#include <cassert>
#include <cstring>
#include <memory>
#include <vector>

#include <d3d11_1.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

#include "Source/AppWin/public/D3DUtil.h"
#include "Source/ThirdParty/ImGui/imgui.h"
#include "Source/ThirdParty/ImGui/imgui_impl_dx11.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace
{
    using Microsoft::WRL::ComPtr;

    class Dx11GraphicsBuffer final : public IGraphicsBuffer
    {
    public:
        ComPtr<ID3D11Buffer> buffer;
    };

    class Dx11ShaderBytecode final : public IGraphicsShaderBytecode
    {
    public:
        ComPtr<ID3DBlob> blob;
    };

    class Dx11VertexShader final : public IGraphicsVertexShader
    {
    public:
        ComPtr<ID3D11VertexShader> shader;
    };

    class Dx11PixelShader final : public IGraphicsPixelShader
    {
    public:
        ComPtr<ID3D11PixelShader> shader;
    };

    class Dx11InputLayout final : public IGraphicsInputLayout
    {
    public:
        ComPtr<ID3D11InputLayout> inputLayout;
    };

    class Dx11RasterizerState final : public IGraphicsRasterizerState
    {
    public:
        ComPtr<ID3D11RasterizerState> rasterizerState;
    };

    class Dx11DepthStencilState final : public IGraphicsDepthStencilState
    {
    public:
        ComPtr<ID3D11DepthStencilState> state;
    };

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

    D3D11_USAGE ToD3D11Usage(GraphicsBufferUsage usage)
    {
        switch (usage)
        {
        case GraphicsBufferUsage::Dynamic:
            return D3D11_USAGE_DYNAMIC;
        case GraphicsBufferUsage::Immutable:
        default:
            return D3D11_USAGE_IMMUTABLE;
        }
    }

    D3D11_PRIMITIVE_TOPOLOGY ToD3D11PrimitiveTopology(GraphicsPrimitiveTopology topology)
    {
        switch (topology)
        {
        case GraphicsPrimitiveTopology::TriangleList:
        default:
            return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
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

    D3D11_FILL_MODE ToD3D11FillMode(GraphicsFillMode mode)
    {
        switch (mode)
        {
        case GraphicsFillMode::Wireframe:
            return D3D11_FILL_WIREFRAME;
        case GraphicsFillMode::Solid:
        default:
            return D3D11_FILL_SOLID;
        }
    }

    D3D11_CULL_MODE ToD3D11CullMode(GraphicsCullMode mode)
    {
        switch (mode)
        {
        case GraphicsCullMode::None:
            return D3D11_CULL_NONE;
        case GraphicsCullMode::Front:
            return D3D11_CULL_FRONT;
        case GraphicsCullMode::Back:
        default:
            return D3D11_CULL_BACK;
        }
    }

    D3D11_COMPARISON_FUNC ToD3D11ComparisonFunc(GraphicsComparisonFunc func)
    {
        switch (func)
        {
        case GraphicsComparisonFunc::Never:
            return D3D11_COMPARISON_NEVER;
        case GraphicsComparisonFunc::Less:
            return D3D11_COMPARISON_LESS;
        case GraphicsComparisonFunc::Equal:
            return D3D11_COMPARISON_EQUAL;
        case GraphicsComparisonFunc::LessEqual:
            return D3D11_COMPARISON_LESS_EQUAL;
        case GraphicsComparisonFunc::Greater:
            return D3D11_COMPARISON_GREATER;
        case GraphicsComparisonFunc::NotEqual:
            return D3D11_COMPARISON_NOT_EQUAL;
        case GraphicsComparisonFunc::GreaterEqual:
            return D3D11_COMPARISON_GREATER_EQUAL;
        case GraphicsComparisonFunc::Always:
            return D3D11_COMPARISON_ALWAYS;
        default:
            return D3D11_COMPARISON_LESS;
        }
    }

    class Dx11Backend final : public IGraphicsBackend
    {
    public:
        ~Dx11Backend() override
        {
            ShutdownImGui();
            if (m_immediateContext)
            {
                m_immediateContext->ClearState();
                m_immediateContext->Flush();
            }
        }

        GraphicsBackendType GetType() const override
        {
            return GraphicsBackendType::DX11;
        }

        bool Initialize(void* nativeWindowHandle, int width, int height, bool enable4xMsaa) override
        {
            m_hwnd = glfwGetWin32Window(static_cast<GLFWwindow*>(nativeWindowHandle));
            m_width = width;
            m_height = height;
            m_enable4xMsaa = enable4xMsaa;

            if (!CreateDeviceAndSwapChain())
            {
                return false;
            }

            Resize(width, height);
            return true;
        }

        void Resize(int width, int height) override
        {
            if (!m_swapChain || !m_device || !m_immediateContext || width <= 0 || height <= 0)
            {
                return;
            }

            m_width = width;
            m_height = height;

            m_renderTargetView.Reset();
            m_depthStencilView.Reset();
            m_depthStencilBuffer.Reset();

            if (FAILED(m_swapChain->ResizeBuffers(1, m_width, m_height, DXGI_FORMAT_R8G8B8A8_UNORM, 0)))
            {
                return;
            }

            ComPtr<ID3D11Texture2D> backBuffer;
            if (FAILED(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()))))
            {
                return;
            }

            if (FAILED(m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf())))
            {
                return;
            }

            D3D11SetDebugObjectName(backBuffer.Get(), "BackBuffer[0]");

            D3D11_TEXTURE2D_DESC depthStencilDesc = {};
            depthStencilDesc.Width = m_width;
            depthStencilDesc.Height = m_height;
            depthStencilDesc.MipLevels = 1;
            depthStencilDesc.ArraySize = 1;
            depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            depthStencilDesc.SampleDesc.Count = m_enable4xMsaa ? 4 : 1;
            depthStencilDesc.SampleDesc.Quality = m_enable4xMsaa ? m_4xMsaaQuality - 1 : 0;
            depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
            depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

            if (FAILED(m_device->CreateTexture2D(&depthStencilDesc, nullptr, m_depthStencilBuffer.GetAddressOf())))
            {
                return;
            }

            if (FAILED(m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), nullptr, m_depthStencilView.GetAddressOf())))
            {
                return;
            }

            m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

            GraphicsViewport viewport = {};
            viewport.width = static_cast<float>(m_width);
            viewport.height = static_cast<float>(m_height);
            SetViewport(viewport);
        }

        bool InitializeImGui(void*) override
        {
            if (m_imguiInitialized)
            {
                return true;
            }

            m_imguiInitialized = ImGui_ImplDX11_Init(m_device.Get(), m_immediateContext.Get());
            return m_imguiInitialized;
        }

        void BeginImGuiFrame() override
        {
            if (m_imguiInitialized)
            {
                ImGui_ImplDX11_NewFrame();
            }
        }

        void RenderImGuiDrawData() override
        {
            if (m_imguiInitialized)
            {
                ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
            }
        }

        void ShutdownImGui() override
        {
            if (m_imguiInitialized)
            {
                ImGui_ImplDX11_Shutdown();
                m_imguiInitialized = false;
            }
        }

        void Clear(const float color[4], float depth, std::uint8_t stencil) override
        {
            if (!m_renderTargetView || !m_depthStencilView)
            {
                return;
            }

            m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
            m_immediateContext->RSSetViewports(1, &m_screenViewport);
            m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), color);
            m_immediateContext->ClearDepthStencilView(
                m_depthStencilView.Get(),
                D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
                depth,
                stencil);
        }

        void Present() override
        {
            if (m_swapChain)
            {
                m_swapChain->Present(0, 0);
            }
        }

        void SetViewport(const GraphicsViewport& viewport) override
        {
            m_screenViewport.TopLeftX = viewport.x;
            m_screenViewport.TopLeftY = viewport.y;
            m_screenViewport.Width = viewport.width;
            m_screenViewport.Height = viewport.height;
            m_screenViewport.MinDepth = viewport.minDepth;
            m_screenViewport.MaxDepth = viewport.maxDepth;

            if (m_immediateContext)
            {
                m_immediateContext->RSSetViewports(1, &m_screenViewport);
            }
        }

        std::shared_ptr<IGraphicsBuffer> CreateBuffer(const GraphicsBufferDesc& desc, const void* initialData) override
        {
            D3D11_BUFFER_DESC bufferDesc = {};
            bufferDesc.ByteWidth = desc.byteWidth;
            bufferDesc.Usage = ToD3D11Usage(desc.usage);
            bufferDesc.CPUAccessFlags = desc.cpuWrite ? D3D11_CPU_ACCESS_WRITE : 0;

            if (HasAnyFlag(desc.bindFlags, GraphicsBufferBindFlags::VertexBuffer))
            {
                bufferDesc.BindFlags |= D3D11_BIND_VERTEX_BUFFER;
            }
            if (HasAnyFlag(desc.bindFlags, GraphicsBufferBindFlags::IndexBuffer))
            {
                bufferDesc.BindFlags |= D3D11_BIND_INDEX_BUFFER;
            }
            if (HasAnyFlag(desc.bindFlags, GraphicsBufferBindFlags::ConstantBuffer))
            {
                bufferDesc.BindFlags |= D3D11_BIND_CONSTANT_BUFFER;
            }

            D3D11_SUBRESOURCE_DATA subresourceData = {};
            D3D11_SUBRESOURCE_DATA* initData = nullptr;
            if (initialData != nullptr)
            {
                subresourceData.pSysMem = initialData;
                initData = &subresourceData;
            }

            auto buffer = std::make_shared<Dx11GraphicsBuffer>();
            if (FAILED(m_device->CreateBuffer(&bufferDesc, initData, buffer->buffer.GetAddressOf())))
            {
                return nullptr;
            }

            if (!desc.debugName.empty())
            {
                D3D11SetDebugObjectName(buffer->buffer.Get(), desc.debugName);
            }

            return buffer;
        }

        void UpdateBuffer(IGraphicsBuffer& buffer, const void* data, std::size_t dataSize) override
        {
            Dx11GraphicsBuffer* dx11Buffer = CheckedCast<Dx11GraphicsBuffer>(&buffer);
            D3D11_MAPPED_SUBRESOURCE mappedData = {};
            if (FAILED(m_immediateContext->Map(dx11Buffer->buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData)))
            {
                return;
            }

            std::memcpy(mappedData.pData, data, dataSize);
            m_immediateContext->Unmap(dx11Buffer->buffer.Get(), 0);
        }

        std::shared_ptr<IGraphicsShaderBytecode> CompileShader(const GraphicsShaderDesc& desc) override
        {
            auto bytecode = std::make_shared<Dx11ShaderBytecode>();
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
            const char* debugName) override
        {
            const Dx11ShaderBytecode* dx11Bytecode = CheckedCast<const Dx11ShaderBytecode>(
                const_cast<IGraphicsShaderBytecode*>(&bytecode));
            auto shader = std::make_shared<Dx11VertexShader>();
            if (FAILED(m_device->CreateVertexShader(
                dx11Bytecode->blob->GetBufferPointer(),
                dx11Bytecode->blob->GetBufferSize(),
                nullptr,
                shader->shader.GetAddressOf())))
            {
                return nullptr;
            }

            if (debugName != nullptr)
            {
                D3D11SetDebugObjectName(shader->shader.Get(), debugName, static_cast<UINT>(std::strlen(debugName)));
            }

            return shader;
        }

        std::shared_ptr<IGraphicsPixelShader> CreatePixelShader(
            const IGraphicsShaderBytecode& bytecode,
            const char* debugName) override
        {
            const Dx11ShaderBytecode* dx11Bytecode = CheckedCast<const Dx11ShaderBytecode>(
                const_cast<IGraphicsShaderBytecode*>(&bytecode));
            auto shader = std::make_shared<Dx11PixelShader>();
            if (FAILED(m_device->CreatePixelShader(
                dx11Bytecode->blob->GetBufferPointer(),
                dx11Bytecode->blob->GetBufferSize(),
                nullptr,
                shader->shader.GetAddressOf())))
            {
                return nullptr;
            }

            if (debugName != nullptr)
            {
                D3D11SetDebugObjectName(shader->shader.Get(), debugName, static_cast<UINT>(std::strlen(debugName)));
            }

            return shader;
        }

        std::shared_ptr<IGraphicsInputLayout> CreateInputLayout(
            const VertexLayoutDesc& layoutDesc,
            const IGraphicsShaderBytecode& bytecode,
            const char* debugName) override
        {
            const Dx11ShaderBytecode* dx11Bytecode = CheckedCast<const Dx11ShaderBytecode>(
                const_cast<IGraphicsShaderBytecode*>(&bytecode));

            std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements;
            inputElements.reserve(layoutDesc.attributeCount);
            for (std::uint32_t i = 0; i < layoutDesc.attributeCount; ++i)
            {
                const VertexAttributeDesc& attribute = layoutDesc.attributes[i];
                D3D11_INPUT_ELEMENT_DESC element = {};
                element.SemanticName = attribute.semanticName;
                element.SemanticIndex = attribute.semanticIndex;
                element.Format = ToDxgiFormat(attribute.format);
                element.InputSlot = 0;
                element.AlignedByteOffset = attribute.alignedByteOffset;
                element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
                inputElements.push_back(element);
            }

            auto inputLayout = std::make_shared<Dx11InputLayout>();
            if (FAILED(m_device->CreateInputLayout(
                inputElements.data(),
                static_cast<UINT>(inputElements.size()),
                dx11Bytecode->blob->GetBufferPointer(),
                dx11Bytecode->blob->GetBufferSize(),
                inputLayout->inputLayout.GetAddressOf())))
            {
                return nullptr;
            }

            if (debugName != nullptr)
            {
                D3D11SetDebugObjectName(inputLayout->inputLayout.Get(), debugName, static_cast<UINT>(std::strlen(debugName)));
            }

            return inputLayout;
        }

        std::shared_ptr<IGraphicsRasterizerState> CreateRasterizerState(const GraphicsRasterizerDesc& desc) override
        {
            D3D11_RASTERIZER_DESC rasterizerDesc = {};
            rasterizerDesc.FillMode = ToD3D11FillMode(desc.fillMode);
            rasterizerDesc.CullMode = ToD3D11CullMode(desc.cullMode);
            rasterizerDesc.FrontCounterClockwise = desc.frontCounterClockwise;
            rasterizerDesc.DepthClipEnable = desc.depthClipEnable;

            auto rasterizerState = std::make_shared<Dx11RasterizerState>();
            if (FAILED(m_device->CreateRasterizerState(&rasterizerDesc, rasterizerState->rasterizerState.GetAddressOf())))
            {
                return nullptr;
            }

            if (!desc.debugName.empty())
            {
                D3D11SetDebugObjectName(rasterizerState->rasterizerState.Get(), desc.debugName);
            }

            return rasterizerState;
        }

        std::shared_ptr<IGraphicsDepthStencilState> CreateDepthStencilState(const GraphicsDepthStencilDesc& desc) override
        {
            D3D11_DEPTH_STENCIL_DESC dsDesc = {};
            dsDesc.DepthEnable = desc.depthEnable ? TRUE : FALSE;
            dsDesc.DepthWriteMask = (desc.depthWriteMask == GraphicsDepthWriteMask::All) ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
            dsDesc.DepthFunc = ToD3D11ComparisonFunc(desc.depthFunc);
            dsDesc.StencilEnable = desc.stencilEnable ? TRUE : FALSE;

            auto depthStencilState = std::make_shared<Dx11DepthStencilState>();
            if (FAILED(m_device->CreateDepthStencilState(&dsDesc, depthStencilState->state.GetAddressOf())))
            {
                return nullptr;
            }

            if (!desc.debugName.empty())
            {
                D3D11SetDebugObjectName(depthStencilState->state.Get(), desc.debugName);
            }

            return depthStencilState;
        }

        void SetDepthStencilState(const IGraphicsDepthStencilState* depthStencilState) override
        {
            const Dx11DepthStencilState* dx11DepthStencilState = CheckedCast<const Dx11DepthStencilState>(
                const_cast<IGraphicsDepthStencilState*>(depthStencilState));
            m_immediateContext->OMSetDepthStencilState(
                dx11DepthStencilState != nullptr ? dx11DepthStencilState->state.Get() : nullptr, 0);
        }

        void SetVertexBuffer(const IGraphicsBuffer& buffer, std::uint32_t stride, std::uint32_t offset) override
        {
            const Dx11GraphicsBuffer* dx11Buffer = CheckedCast<const Dx11GraphicsBuffer>(const_cast<IGraphicsBuffer*>(&buffer));
            ID3D11Buffer* nativeBuffer = dx11Buffer->buffer.Get();
            m_immediateContext->IASetVertexBuffers(0, 1, &nativeBuffer, &stride, &offset);
        }

        void SetIndexBuffer(const IGraphicsBuffer& buffer, GraphicsIndexFormat format, std::uint32_t offset) override
        {
            const Dx11GraphicsBuffer* dx11Buffer = CheckedCast<const Dx11GraphicsBuffer>(const_cast<IGraphicsBuffer*>(&buffer));
            m_immediateContext->IASetIndexBuffer(dx11Buffer->buffer.Get(), ToDxgiIndexFormat(format), offset);
        }

        void SetPrimitiveTopology(GraphicsPrimitiveTopology topology) override
        {
            m_immediateContext->IASetPrimitiveTopology(ToD3D11PrimitiveTopology(topology));
        }

        void SetInputLayout(const IGraphicsInputLayout* inputLayout) override
        {
            const Dx11InputLayout* dx11InputLayout = CheckedCast<const Dx11InputLayout>(const_cast<IGraphicsInputLayout*>(inputLayout));
            m_immediateContext->IASetInputLayout(dx11InputLayout != nullptr ? dx11InputLayout->inputLayout.Get() : nullptr);
        }

        void SetVertexShader(const IGraphicsVertexShader* shader) override
        {
            const Dx11VertexShader* dx11Shader = CheckedCast<const Dx11VertexShader>(const_cast<IGraphicsVertexShader*>(shader));
            m_immediateContext->VSSetShader(dx11Shader != nullptr ? dx11Shader->shader.Get() : nullptr, nullptr, 0);
        }

        void SetPixelShader(const IGraphicsPixelShader* shader) override
        {
            const Dx11PixelShader* dx11Shader = CheckedCast<const Dx11PixelShader>(const_cast<IGraphicsPixelShader*>(shader));
            m_immediateContext->PSSetShader(dx11Shader != nullptr ? dx11Shader->shader.Get() : nullptr, nullptr, 0);
        }

        void SetVertexConstantBuffer(std::uint32_t slot, const IGraphicsBuffer* buffer) override
        {
            const Dx11GraphicsBuffer* dx11Buffer = CheckedCast<const Dx11GraphicsBuffer>(const_cast<IGraphicsBuffer*>(buffer));
            ID3D11Buffer* nativeBuffer = dx11Buffer != nullptr ? dx11Buffer->buffer.Get() : nullptr;
            m_immediateContext->VSSetConstantBuffers(slot, 1, &nativeBuffer);
        }

        void SetPixelConstantBuffer(std::uint32_t slot, const IGraphicsBuffer* buffer) override
        {
            const Dx11GraphicsBuffer* dx11Buffer = CheckedCast<const Dx11GraphicsBuffer>(const_cast<IGraphicsBuffer*>(buffer));
            ID3D11Buffer* nativeBuffer = dx11Buffer != nullptr ? dx11Buffer->buffer.Get() : nullptr;
            m_immediateContext->PSSetConstantBuffers(slot, 1, &nativeBuffer);
        }

        void SetRasterizerState(const IGraphicsRasterizerState* rasterizerState) override
        {
            const Dx11RasterizerState* dx11RasterizerState = CheckedCast<const Dx11RasterizerState>(
                const_cast<IGraphicsRasterizerState*>(rasterizerState));
            m_immediateContext->RSSetState(dx11RasterizerState != nullptr ? dx11RasterizerState->rasterizerState.Get() : nullptr);
        }

        void DrawIndexed(std::uint32_t indexCount, std::uint32_t startIndexLocation, std::int32_t baseVertexLocation) override
        {
            m_immediateContext->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
        }

    private:
        bool CreateDeviceAndSwapChain()
        {
            UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
            createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
            const UINT baseCreateDeviceFlags = createDeviceFlags;

            D3D_DRIVER_TYPE driverTypes[] =
            {
                D3D_DRIVER_TYPE_HARDWARE,
                D3D_DRIVER_TYPE_WARP,
                D3D_DRIVER_TYPE_REFERENCE,
            };

            D3D_FEATURE_LEVEL featureLevels[] =
            {
                D3D_FEATURE_LEVEL_11_1,
                D3D_FEATURE_LEVEL_11_0,
            };

            HRESULT hr = S_OK;
            D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

            for (D3D_DRIVER_TYPE driverType : driverTypes)
            {
                createDeviceFlags = baseCreateDeviceFlags;
                hr = D3D11CreateDevice(
                    nullptr,
                    driverType,
                    nullptr,
                    createDeviceFlags,
                    featureLevels,
                    ARRAYSIZE(featureLevels),
                    D3D11_SDK_VERSION,
                    m_device.GetAddressOf(),
                    &featureLevel,
                    m_immediateContext.GetAddressOf());

                if (hr == E_INVALIDARG)
                {
                    hr = D3D11CreateDevice(
                        nullptr,
                        driverType,
                        nullptr,
                        createDeviceFlags,
                        &featureLevels[1],
                        ARRAYSIZE(featureLevels) - 1,
                        D3D11_SDK_VERSION,
                        m_device.GetAddressOf(),
                        &featureLevel,
                        m_immediateContext.GetAddressOf());
                }

#if defined(DEBUG) || defined(_DEBUG)
                if (FAILED(hr) && (createDeviceFlags & D3D11_CREATE_DEVICE_DEBUG))
                {
                    createDeviceFlags &= ~D3D11_CREATE_DEVICE_DEBUG;
                    hr = D3D11CreateDevice(
                        nullptr,
                        driverType,
                        nullptr,
                        createDeviceFlags,
                        featureLevels,
                        ARRAYSIZE(featureLevels),
                        D3D11_SDK_VERSION,
                        m_device.GetAddressOf(),
                        &featureLevel,
                        m_immediateContext.GetAddressOf());

                    if (hr == E_INVALIDARG)
                    {
                        hr = D3D11CreateDevice(
                            nullptr,
                            driverType,
                            nullptr,
                            createDeviceFlags,
                            &featureLevels[1],
                            ARRAYSIZE(featureLevels) - 1,
                            D3D11_SDK_VERSION,
                            m_device.GetAddressOf(),
                            &featureLevel,
                            m_immediateContext.GetAddressOf());
                    }
                }
#endif

                if (SUCCEEDED(hr))
                {
                    break;
                }
            }

            if (FAILED(hr) || !m_device || !m_immediateContext)
            {
                return false;
            }

            if (featureLevel != D3D_FEATURE_LEVEL_11_0 && featureLevel != D3D_FEATURE_LEVEL_11_1)
            {
                return false;
            }

            m_device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m_4xMsaaQuality);
            if (m_enable4xMsaa && m_4xMsaaQuality == 0)
            {
                m_enable4xMsaa = false;
            }

            ComPtr<IDXGIDevice> dxgiDevice;
            ComPtr<IDXGIAdapter> dxgiAdapter;
            ComPtr<IDXGIFactory1> dxgiFactory1;
            ComPtr<IDXGIFactory2> dxgiFactory2;

            if (FAILED(m_device.As(&dxgiDevice)))
            {
                return false;
            }
            if (FAILED(dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf())))
            {
                return false;
            }
            if (FAILED(dxgiAdapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(dxgiFactory1.GetAddressOf()))))
            {
                return false;
            }

            dxgiFactory1.As(&dxgiFactory2);

            if (dxgiFactory2)
            {
                if (FAILED(m_device.As(&m_device1)) || FAILED(m_immediateContext.As(&m_immediateContext1)))
                {
                    return false;
                }

                DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
                swapChainDesc.Width = m_width;
                swapChainDesc.Height = m_height;
                swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                swapChainDesc.SampleDesc.Count = m_enable4xMsaa ? 4 : 1;
                swapChainDesc.SampleDesc.Quality = m_enable4xMsaa ? m_4xMsaaQuality - 1 : 0;
                swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
                swapChainDesc.BufferCount = 1;
                swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

                DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenDesc = {};
                fullscreenDesc.RefreshRate.Numerator = 60;
                fullscreenDesc.RefreshRate.Denominator = 1;
                fullscreenDesc.Windowed = TRUE;

                if (FAILED(dxgiFactory2->CreateSwapChainForHwnd(
                    m_device.Get(),
                    m_hwnd,
                    &swapChainDesc,
                    &fullscreenDesc,
                    nullptr,
                    m_swapChain1.GetAddressOf())))
                {
                    return false;
                }

                if (FAILED(m_swapChain1.As(&m_swapChain)))
                {
                    return false;
                }
            }
            else
            {
                DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
                swapChainDesc.BufferDesc.Width = m_width;
                swapChainDesc.BufferDesc.Height = m_height;
                swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
                swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
                swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                swapChainDesc.SampleDesc.Count = m_enable4xMsaa ? 4 : 1;
                swapChainDesc.SampleDesc.Quality = m_enable4xMsaa ? m_4xMsaaQuality - 1 : 0;
                swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
                swapChainDesc.BufferCount = 1;
                swapChainDesc.OutputWindow = m_hwnd;
                swapChainDesc.Windowed = TRUE;
                swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

                if (FAILED(dxgiFactory1->CreateSwapChain(m_device.Get(), &swapChainDesc, m_swapChain.GetAddressOf())))
                {
                    return false;
                }
            }

            dxgiFactory1->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER | DXGI_MWA_NO_WINDOW_CHANGES);
            D3D11SetDebugObjectName(m_immediateContext.Get(), "ImmediateContext");
            DXGISetDebugObjectName(m_swapChain.Get(), "SwapChain");
            return true;
        }

    private:
        HWND m_hwnd = nullptr;
        int m_width = 0;
        int m_height = 0;
        bool m_enable4xMsaa = true;
        bool m_imguiInitialized = false;
        UINT m_4xMsaaQuality = 0;

        ComPtr<ID3D11Device> m_device;
        ComPtr<ID3D11DeviceContext> m_immediateContext;
        ComPtr<IDXGISwapChain> m_swapChain;

        ComPtr<ID3D11Device1> m_device1;
        ComPtr<ID3D11DeviceContext1> m_immediateContext1;
        ComPtr<IDXGISwapChain1> m_swapChain1;

        ComPtr<ID3D11Texture2D> m_depthStencilBuffer;
        ComPtr<ID3D11RenderTargetView> m_renderTargetView;
        ComPtr<ID3D11DepthStencilView> m_depthStencilView;
        D3D11_VIEWPORT m_screenViewport = {};
    };
}

std::unique_ptr<IGraphicsBackend> CreateDx11Backend()
{
    return std::make_unique<Dx11Backend>();
}
