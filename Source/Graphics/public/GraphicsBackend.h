#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

#include "GraphicsTypes.h"

class IGraphicsBuffer
{
public:
    virtual ~IGraphicsBuffer() = default;
};

class IGraphicsShaderBytecode
{
public:
    virtual ~IGraphicsShaderBytecode() = default;
};

class IGraphicsVertexShader
{
public:
    virtual ~IGraphicsVertexShader() = default;
};

class IGraphicsPixelShader
{
public:
    virtual ~IGraphicsPixelShader() = default;
};

class IGraphicsInputLayout
{
public:
    virtual ~IGraphicsInputLayout() = default;
};

class IGraphicsRasterizerState
{
public:
    virtual ~IGraphicsRasterizerState() = default;
};

class IGraphicsDepthStencilState
{
public:
    virtual ~IGraphicsDepthStencilState() = default;
};

class IGraphicsRenderTarget
{
public:
    virtual ~IGraphicsRenderTarget() = default;
    virtual void* GetImGuiTextureId() const = 0;
    virtual int GetWidth() const = 0;
    virtual int GetHeight() const = 0;
};

class IGraphicsBackend
{
public:
    virtual ~IGraphicsBackend() = default;

    virtual GraphicsBackendType GetType() const = 0;
    virtual bool Initialize(void* nativeWindowHandle, int width, int height, bool enable4xMsaa) = 0;
    virtual void Resize(int width, int height) = 0;

    virtual bool InitializeImGui(void* nativeWindowHandle) = 0;
    virtual void BeginImGuiFrame() = 0;
    virtual void RenderImGuiDrawData() = 0;
    virtual void ShutdownImGui() = 0;

    virtual void Clear(const float color[4], float depth, std::uint8_t stencil) = 0;
    virtual void Present() = 0;
    virtual void SetViewport(const GraphicsViewport& viewport) = 0;

    virtual std::shared_ptr<IGraphicsBuffer> CreateBuffer(const GraphicsBufferDesc& desc, const void* initialData) = 0;
    virtual void UpdateBuffer(IGraphicsBuffer& buffer, const void* data, std::size_t dataSize) = 0;

    virtual std::shared_ptr<IGraphicsShaderBytecode> CompileShader(const GraphicsShaderDesc& desc) = 0;
    virtual std::shared_ptr<IGraphicsVertexShader> CreateVertexShader(
        const IGraphicsShaderBytecode& bytecode,
        const char* debugName) = 0;
    virtual std::shared_ptr<IGraphicsPixelShader> CreatePixelShader(
        const IGraphicsShaderBytecode& bytecode,
        const char* debugName) = 0;
    virtual std::shared_ptr<IGraphicsInputLayout> CreateInputLayout(
        const VertexLayoutDesc& layoutDesc,
        const IGraphicsShaderBytecode& bytecode,
        const char* debugName) = 0;
    virtual std::shared_ptr<IGraphicsRasterizerState> CreateRasterizerState(const GraphicsRasterizerDesc& desc) = 0;
    virtual std::shared_ptr<IGraphicsDepthStencilState> CreateDepthStencilState(const GraphicsDepthStencilDesc& desc) = 0;
    virtual void SetDepthStencilState(const IGraphicsDepthStencilState* depthStencilState) = 0;

    virtual std::shared_ptr<IGraphicsRenderTarget> CreateRenderTarget(int width, int height) = 0;
    virtual void SetViewportRenderTarget(IGraphicsRenderTarget* rt) = 0;
    virtual void ClearViewportRenderTarget(IGraphicsRenderTarget* rt, const float color[4], float depth, std::uint8_t stencil) = 0;

    virtual void SetVertexBuffer(const IGraphicsBuffer& buffer, std::uint32_t stride, std::uint32_t offset) = 0;
    virtual void SetIndexBuffer(const IGraphicsBuffer& buffer, GraphicsIndexFormat format, std::uint32_t offset) = 0;
    virtual void SetPrimitiveTopology(GraphicsPrimitiveTopology topology) = 0;
    virtual void SetInputLayout(const IGraphicsInputLayout* inputLayout) = 0;
    virtual void SetVertexShader(const IGraphicsVertexShader* shader) = 0;
    virtual void SetPixelShader(const IGraphicsPixelShader* shader) = 0;
    virtual void SetVertexConstantBuffer(std::uint32_t slot, const IGraphicsBuffer* buffer) = 0;
    virtual void SetPixelConstantBuffer(std::uint32_t slot, const IGraphicsBuffer* buffer) = 0;
    virtual void SetRasterizerState(const IGraphicsRasterizerState* rasterizerState) = 0;
    virtual void DrawIndexed(std::uint32_t indexCount, std::uint32_t startIndexLocation, std::int32_t baseVertexLocation) = 0;
};
