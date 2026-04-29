#include "Source/Graphics/public/GraphicsBackend.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <Cocoa/Cocoa.h>

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>

#include "Source/AppWin/public/D3DUtil.h"
#include "Source/ThirdParty/ImGui/imgui.h"
#include "Source/ThirdParty/ImGui/imgui_impl_metal.h"

namespace
{
    class MetalGraphicsBuffer final : public IGraphicsBuffer
    {
    public:
        id<MTLBuffer> buffer = nil;
    };

    class MetalShaderBytecode final : public IGraphicsShaderBytecode
    {
    public:
        id<MTLLibrary> library = nil;
        std::string entryPointName;
    };

    class MetalVertexShader final : public IGraphicsVertexShader
    {
    public:
        id<MTLFunction> function = nil;
    };

    class MetalPixelShader final : public IGraphicsPixelShader
    {
    public:
        id<MTLFunction> function = nil;
    };

    class MetalInputLayout final : public IGraphicsInputLayout
    {
    public:
        MTLVertexDescriptor* vertexDescriptor = nil;
    };

    class MetalRasterizerState final : public IGraphicsRasterizerState
    {
    public:
        MTLCullMode cullMode = MTLCullModeNone;
        MTLTriangleFillMode fillMode = MTLTriangleFillModeFill;
    };

    class MetalDepthStencilState final : public IGraphicsDepthStencilState
    {
    public:
        id<MTLDepthStencilState> state = nil;
    };

    class MetalRenderTarget final : public IGraphicsRenderTarget
    {
    public:
        id<MTLTexture> colorTexture = nil;
        id<MTLTexture> depthTexture = nil;
        int width = 0;
        int height = 0;

        void* GetImGuiTextureId() const override { return (__bridge void*)colorTexture; }
        int GetWidth() const override { return width; }
        int GetHeight() const override { return height; }
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

    MTLPixelFormat ToMetalPixelFormat(GraphicsFormat format)
    {
        switch (format)
        {
        case GraphicsFormat::R32G32_FLOAT:
            return MTLPixelFormatRG32Float;
        case GraphicsFormat::R32G32B32_FLOAT:
            return MTLPixelFormatRGB9E5Float; // No direct RGB32Float in Metal vertex formats; handled separately
        case GraphicsFormat::R32G32B32A32_FLOAT:
            return MTLPixelFormatRGBA32Float;
        case GraphicsFormat::R8G8B8A8_UNORM:
            return MTLPixelFormatBGRA8Unorm;
        default:
            return MTLPixelFormatInvalid;
        }
    }

    MTLVertexFormat ToMetalVertexFormat(GraphicsFormat format)
    {
        switch (format)
        {
        case GraphicsFormat::R32G32_FLOAT:
            return MTLVertexFormatFloat2;
        case GraphicsFormat::R32G32B32_FLOAT:
            return MTLVertexFormatFloat3;
        case GraphicsFormat::R32G32B32A32_FLOAT:
            return MTLVertexFormatFloat4;
        case GraphicsFormat::R8G8B8A8_UNORM:
            return MTLVertexFormatUChar4Normalized;
        default:
            return MTLVertexFormatInvalid;
        }
    }

    MTLPrimitiveType ToMetalPrimitiveType(GraphicsPrimitiveTopology topology)
    {
        switch (topology)
        {
        case GraphicsPrimitiveTopology::TriangleList:
        default:
            return MTLPrimitiveTypeTriangle;
        }
    }

    MTLIndexType ToMetalIndexType(GraphicsIndexFormat format)
    {
        switch (format)
        {
        case GraphicsIndexFormat::UInt16:
            return MTLIndexTypeUInt16;
        case GraphicsIndexFormat::UInt32:
        default:
            return MTLIndexTypeUInt32;
        }
    }

    MTLTriangleFillMode ToMetalFillMode(GraphicsFillMode mode)
    {
        switch (mode)
        {
        case GraphicsFillMode::Wireframe:
            return MTLTriangleFillModeLines;
        case GraphicsFillMode::Solid:
        default:
            return MTLTriangleFillModeFill;
        }
    }

    MTLCullMode ToMetalCullMode(GraphicsCullMode mode)
    {
        switch (mode)
        {
        case GraphicsCullMode::None:
            return MTLCullModeNone;
        case GraphicsCullMode::Front:
            return MTLCullModeFront;
        case GraphicsCullMode::Back:
        default:
            return MTLCullModeBack;
        }
    }

    MTLCompareFunction ToMetalComparisonFunc(GraphicsComparisonFunc func)
    {
        switch (func)
        {
        case GraphicsComparisonFunc::Never:
            return MTLCompareFunctionNever;
        case GraphicsComparisonFunc::Less:
            return MTLCompareFunctionLess;
        case GraphicsComparisonFunc::Equal:
            return MTLCompareFunctionEqual;
        case GraphicsComparisonFunc::LessEqual:
            return MTLCompareFunctionLessEqual;
        case GraphicsComparisonFunc::Greater:
            return MTLCompareFunctionGreater;
        case GraphicsComparisonFunc::NotEqual:
            return MTLCompareFunctionNotEqual;
        case GraphicsComparisonFunc::GreaterEqual:
            return MTLCompareFunctionGreaterEqual;
        case GraphicsComparisonFunc::Always:
            return MTLCompareFunctionAlways;
        default:
            return MTLCompareFunctionLess;
        }
    }

    struct PipelineStateKey
    {
        id<MTLFunction> vertexFunction = nil;
        id<MTLFunction> fragmentFunction = nil;
        MTLVertexDescriptor* vertexDescriptor = nil;
        MTLCullMode cullMode = MTLCullModeNone;
        MTLTriangleFillMode fillMode = MTLTriangleFillModeFill;
        MTLPixelFormat colorFormat = MTLPixelFormatBGRA8Unorm;
        MTLPixelFormat depthFormat = MTLPixelFormatDepth32Float_Stencil8;

        bool operator==(const PipelineStateKey& other) const
        {
            return vertexFunction == other.vertexFunction &&
                   fragmentFunction == other.fragmentFunction &&
                   vertexDescriptor == other.vertexDescriptor &&
                   cullMode == other.cullMode &&
                   fillMode == other.fillMode &&
                   colorFormat == other.colorFormat &&
                   depthFormat == other.depthFormat;
        }
    };

    struct PipelineStateKeyHash
    {
        std::size_t operator()(const PipelineStateKey& key) const
        {
            std::size_t h1 = std::hash<void*>{}((__bridge void*)key.vertexFunction);
            std::size_t h2 = std::hash<void*>{}((__bridge void*)key.fragmentFunction);
            std::size_t h3 = std::hash<void*>{}((__bridge void*)key.vertexDescriptor);
            std::size_t h4 = static_cast<std::size_t>(key.cullMode);
            std::size_t h5 = static_cast<std::size_t>(key.fillMode);
            std::size_t h6 = static_cast<std::size_t>(key.colorFormat);
            std::size_t h7 = static_cast<std::size_t>(key.depthFormat);
            return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3) ^ (h5 << 4) ^ (h6 << 5) ^ (h7 << 6);
        }
    };

    std::string WStringToUTF8(const std::wstring& wstr)
    {
        if (wstr.empty()) return std::string();
        std::vector<char> buffer(wstr.size() * 4);
        std::size_t len = std::wcstombs(buffer.data(), wstr.c_str(), buffer.size());
        if (len == static_cast<std::size_t>(-1)) return std::string();
        return std::string(buffer.data(), len);
    }

    std::string ReadFileToString(const std::string& path)
    {
        std::ifstream file(path);
        if (!file.is_open()) return std::string();
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    class MetalBackend final : public IGraphicsBackend
    {
    public:
        ~MetalBackend() override
        {
            ShutdownImGui();
        }

        GraphicsBackendType GetType() const override
        {
            return GraphicsBackendType::Metal;
        }

        bool Initialize(void* nativeWindowHandle, int width, int height, bool enable4xMsaa) override
        {
            GLFWwindow* window = static_cast<GLFWwindow*>(nativeWindowHandle);
            m_window = window;
            m_width = width;
            m_height = height;
            m_enable4xMsaa = enable4xMsaa;

            m_device = MTLCreateSystemDefaultDevice();
            if (!m_device)
            {
                return false;
            }

            m_commandQueue = [m_device newCommandQueue];
            if (!m_commandQueue)
            {
                return false;
            }

            NSWindow* cocoaWindow = glfwGetCocoaWindow(window);
            if (!cocoaWindow)
            {
                return false;
            }

            NSView* view = [cocoaWindow contentView];
            m_metalLayer = [CAMetalLayer layer];
            m_metalLayer.device = m_device;
            m_metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
            m_metalLayer.drawableSize = CGSizeMake(static_cast<CGFloat>(width), static_cast<CGFloat>(height));
            m_metalLayer.frame = view.bounds;
            m_metalLayer.contentsScale = [view.window backingScaleFactor] ?: 1.0;
            [view setLayer:m_metalLayer];
            [view setWantsLayer:YES];

            NSLog(@"MetalInit: bounds=%@ drawableSize=%@ contentsScale=%f",
                NSStringFromRect(view.bounds),
                NSStringFromSize(NSSizeFromCGSize(m_metalLayer.drawableSize)),
                m_metalLayer.contentsScale);

            MTLDepthStencilDescriptor* depthStencilDesc = [[MTLDepthStencilDescriptor alloc] init];
            depthStencilDesc.depthCompareFunction = MTLCompareFunctionLess;
            depthStencilDesc.depthWriteEnabled = YES;
            m_defaultDepthStencilState = [m_device newDepthStencilStateWithDescriptor:depthStencilDesc];

            return true;
        }

        void Resize(int width, int height) override
        {
            m_width = width;
            m_height = height;
            if (m_metalLayer)
            {
                NSWindow* cocoaWindow = glfwGetCocoaWindow(m_window);
                if (cocoaWindow)
                {
                    NSView* view = [cocoaWindow contentView];
                    m_metalLayer.frame = view.bounds;
                    m_metalLayer.contentsScale = [view.window backingScaleFactor] ?: 1.0;
                }
                // On macOS, drawableSize is derived from bounds * contentsScale when onscreen.
                // Explicitly setting it may have no effect, but we keep it for consistency.
                CGSize autoSize = CGSizeMake(m_metalLayer.bounds.size.width * m_metalLayer.contentsScale,
                                             m_metalLayer.bounds.size.height * m_metalLayer.contentsScale);
                NSLog(@"MetalResize: bounds=%@ contentsScale=%f autoDrawableSize=%@ requested=%@",
                    NSStringFromRect(m_metalLayer.bounds),
                    m_metalLayer.contentsScale,
                    NSStringFromSize(NSSizeFromCGSize(autoSize)),
                    NSStringFromSize(NSSizeFromCGSize(CGSizeMake(width, height))));
            }
        }

        bool InitializeImGui(void*) override
        {
            if (m_imguiInitialized)
            {
                return true;
            }

            m_imguiInitialized = ImGui_ImplMetal_Init(m_device);
            return m_imguiInitialized;
        }

        void BeginImGuiFrame() override
        {
            if (!m_imguiInitialized)
            {
                return;
            }

            // Acquire drawable and create command buffer for the frame
            m_currentDrawable = [m_metalLayer nextDrawable];
            if (!m_currentDrawable)
            {
                return;
            }

            m_currentCommandBuffer = [m_commandQueue commandBuffer];

            // Create render pass descriptor for ImGui to use as framebuffer descriptor
            MTLRenderPassDescriptor* renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
            renderPassDescriptor.colorAttachments[0].texture = m_currentDrawable.texture;
            renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
            renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
            renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.0, 0.0, 0.0, 0.0);
            renderPassDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
            renderPassDescriptor.depthAttachment.storeAction = MTLStoreActionDontCare;
            renderPassDescriptor.depthAttachment.clearDepth = 1.0;
            renderPassDescriptor.stencilAttachment.loadAction = MTLLoadActionClear;
            renderPassDescriptor.stencilAttachment.storeAction = MTLStoreActionDontCare;
            renderPassDescriptor.stencilAttachment.clearStencil = 0;

            m_renderPassDescriptor = renderPassDescriptor;

            ImGui_ImplMetal_NewFrame(renderPassDescriptor);
        }

        void RenderImGuiDrawData() override
        {
            if (!m_imguiInitialized || !m_currentCommandBuffer)
            {
                return;
            }

            if (m_currentEncoder)
            {
                [m_currentEncoder endEncoding];
                m_currentEncoder = nil;
            }

            MTLRenderPassDescriptor* imguiRenderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
            imguiRenderPassDescriptor.colorAttachments[0].texture = m_currentDrawable.texture;
            imguiRenderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionLoad;
            imguiRenderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;

            id<MTLRenderCommandEncoder> imguiEncoder = [m_currentCommandBuffer renderCommandEncoderWithDescriptor:imguiRenderPassDescriptor];
            ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), m_currentCommandBuffer, imguiEncoder);
            [imguiEncoder endEncoding];
        }

        void ShutdownImGui() override
        {
            if (m_imguiInitialized)
            {
                ImGui_ImplMetal_Shutdown();
                m_imguiInitialized = false;
            }
        }

        void Clear(const float color[4], float depth, std::uint8_t stencil) override
        {
            if (!m_currentCommandBuffer || !m_renderPassDescriptor)
            {
                return;
            }

            m_clearColor[0] = color[0];
            m_clearColor[1] = color[1];
            m_clearColor[2] = color[2];
            m_clearColor[3] = color[3];
            m_clearDepth = depth;
            m_clearStencil = stencil;

            m_renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
            m_renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
            m_renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(
                static_cast<double>(color[0]),
                static_cast<double>(color[1]),
                static_cast<double>(color[2]),
                static_cast<double>(color[3]));
            m_renderPassDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
            m_renderPassDescriptor.depthAttachment.storeAction = MTLStoreActionDontCare;
            m_renderPassDescriptor.depthAttachment.clearDepth = static_cast<double>(depth);
            m_renderPassDescriptor.stencilAttachment.loadAction = MTLLoadActionClear;
            m_renderPassDescriptor.stencilAttachment.storeAction = MTLStoreActionDontCare;
            m_renderPassDescriptor.stencilAttachment.clearStencil = stencil;

            m_currentEncoder = [m_currentCommandBuffer renderCommandEncoderWithDescriptor:m_renderPassDescriptor];

            // Set viewport
            MTLViewport viewport = {};
            viewport.originX = 0.0;
            viewport.originY = 0.0;
            viewport.width = static_cast<double>(m_width);
            viewport.height = static_cast<double>(m_height);
            viewport.znear = 0.0;
            viewport.zfar = 1.0;
            [m_currentEncoder setViewport:viewport];
        }

        void Present() override
        {
            if (m_currentEncoder)
            {
                [m_currentEncoder endEncoding];
                m_currentEncoder = nil;
            }

            if (m_currentCommandBuffer && m_currentDrawable)
            {
                [m_currentCommandBuffer presentDrawable:m_currentDrawable];
                [m_currentCommandBuffer commit];
            }

            m_currentDrawable = nil;
            m_currentCommandBuffer = nil;
            m_renderPassDescriptor = nil;
        }

        void SetViewport(const GraphicsViewport& viewport) override
        {
            if (m_currentEncoder)
            {
                MTLViewport vp = {};
                vp.originX = static_cast<double>(viewport.x);
                vp.originY = static_cast<double>(viewport.y);
                vp.width = static_cast<double>(viewport.width);
                vp.height = static_cast<double>(viewport.height);
                vp.znear = static_cast<double>(viewport.minDepth);
                vp.zfar = static_cast<double>(viewport.maxDepth);
                [m_currentEncoder setViewport:vp];
            }
        }

        std::shared_ptr<IGraphicsBuffer> CreateBuffer(const GraphicsBufferDesc& desc, const void* initialData) override
        {
            MTLResourceOptions options = MTLResourceStorageModeShared;
            id<MTLBuffer> buffer = [m_device newBufferWithLength:desc.byteWidth options:options];
            if (!buffer)
            {
                return nullptr;
            }

            if (initialData != nullptr)
            {
                std::memcpy([buffer contents], initialData, desc.byteWidth);
            }

            auto metalBuffer = std::make_shared<MetalGraphicsBuffer>();
            metalBuffer->buffer = buffer;
            return metalBuffer;
        }

        void UpdateBuffer(IGraphicsBuffer& buffer, const void* data, std::size_t dataSize) override
        {
            MetalGraphicsBuffer* metalBuffer = CheckedCast<MetalGraphicsBuffer>(&buffer);
            std::memcpy([metalBuffer->buffer contents], data, dataSize);
        }

        std::shared_ptr<IGraphicsShaderBytecode> CompileShader(const GraphicsShaderDesc& desc) override
        {
            std::string sourcePath = WStringToUTF8(desc.filePath);

            // Always prefer .metal over .hlsl
            std::size_t pos = sourcePath.rfind(".hlsl");
            if (pos != std::string::npos)
            {
                sourcePath.replace(pos, 5, ".metal");
            }

            std::string source = ReadFileToString(sourcePath);
            if (source.empty())
            {
                return nullptr;
            }

            NSString* sourceString = [NSString stringWithUTF8String:source.c_str()];
            MTLCompileOptions* options = [[MTLCompileOptions alloc] init];
            NSError* error = nil;
            id<MTLLibrary> library = [m_device newLibraryWithSource:sourceString options:options error:&error];
            if (error != nil)
            {
                NSLog(@"Metal shader compilation error: %@", error.localizedDescription);
            }
            if (!library)
            {
                return nullptr;
            }

            auto bytecode = std::make_shared<MetalShaderBytecode>();
            bytecode->library = library;
            bytecode->entryPointName = desc.entryPoint.empty() ? "main" : desc.entryPoint;
            return bytecode;
        }

        std::shared_ptr<IGraphicsVertexShader> CreateVertexShader(
            const IGraphicsShaderBytecode& bytecode,
            const char* debugName) override
        {
            const MetalShaderBytecode* metalBytecode = CheckedCast<const MetalShaderBytecode>(
                const_cast<IGraphicsShaderBytecode*>(&bytecode));
            NSString* entryPoint = [NSString stringWithUTF8String:metalBytecode->entryPointName.c_str()];
            id<MTLFunction> function = [metalBytecode->library newFunctionWithName:entryPoint];
            if (!function)
            {
                return nullptr;
            }

            auto shader = std::make_shared<MetalVertexShader>();
            shader->function = function;
            return shader;
        }

        std::shared_ptr<IGraphicsPixelShader> CreatePixelShader(
            const IGraphicsShaderBytecode& bytecode,
            const char* debugName) override
        {
            const MetalShaderBytecode* metalBytecode = CheckedCast<const MetalShaderBytecode>(
                const_cast<IGraphicsShaderBytecode*>(&bytecode));
            NSString* entryPoint = [NSString stringWithUTF8String:metalBytecode->entryPointName.c_str()];
            id<MTLFunction> function = [metalBytecode->library newFunctionWithName:entryPoint];
            if (!function)
            {
                return nullptr;
            }

            auto shader = std::make_shared<MetalPixelShader>();
            shader->function = function;
            return shader;
        }

        std::shared_ptr<IGraphicsInputLayout> CreateInputLayout(
            const VertexLayoutDesc& layoutDesc,
            const IGraphicsShaderBytecode& bytecode,
            const char* debugName) override
        {
            MTLVertexDescriptor* vertexDescriptor = [[MTLVertexDescriptor alloc] init];
            std::uint32_t stride = 0;

            for (std::uint32_t i = 0; i < layoutDesc.attributeCount; ++i)
            {
                const VertexAttributeDesc& attribute = layoutDesc.attributes[i];
                MTLVertexAttributeDescriptor* attrDesc = vertexDescriptor.attributes[i];
                attrDesc.format = ToMetalVertexFormat(attribute.format);
                attrDesc.offset = attribute.alignedByteOffset;
                attrDesc.bufferIndex = 0;

                std::uint32_t formatSize = 0;
                switch (attribute.format)
                {
                case GraphicsFormat::R32G32_FLOAT:
                    formatSize = 8;
                    break;
                case GraphicsFormat::R32G32B32_FLOAT:
                    formatSize = 12;
                    break;
                case GraphicsFormat::R32G32B32A32_FLOAT:
                    formatSize = 16;
                    break;
                case GraphicsFormat::R8G8B8A8_UNORM:
                    formatSize = 4;
                    break;
                default:
                    formatSize = 0;
                    break;
                }

                if (attribute.alignedByteOffset + formatSize > stride)
                {
                    stride = attribute.alignedByteOffset + formatSize;
                }
            }

            vertexDescriptor.layouts[0].stride = stride;
            vertexDescriptor.layouts[0].stepRate = 1;
            vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;

            auto inputLayout = std::make_shared<MetalInputLayout>();
            inputLayout->vertexDescriptor = vertexDescriptor;
            return inputLayout;
        }

        std::shared_ptr<IGraphicsRasterizerState> CreateRasterizerState(const GraphicsRasterizerDesc& desc) override
        {
            auto rasterizerState = std::make_shared<MetalRasterizerState>();
            rasterizerState->fillMode = ToMetalFillMode(desc.fillMode);
            rasterizerState->cullMode = ToMetalCullMode(desc.cullMode);
            return rasterizerState;
        }

        std::shared_ptr<IGraphicsDepthStencilState> CreateDepthStencilState(const GraphicsDepthStencilDesc& desc) override
        {
            MTLDepthStencilDescriptor* depthStencilDesc = [[MTLDepthStencilDescriptor alloc] init];
            depthStencilDesc.depthCompareFunction = desc.depthEnable ? ToMetalComparisonFunc(desc.depthFunc) : MTLCompareFunctionAlways;
            depthStencilDesc.depthWriteEnabled = desc.depthEnable && (desc.depthWriteMask == GraphicsDepthWriteMask::All);

            id<MTLDepthStencilState> state = [m_device newDepthStencilStateWithDescriptor:depthStencilDesc];
            if (!state)
            {
                return nullptr;
            }

            auto depthStencilState = std::make_shared<MetalDepthStencilState>();
            depthStencilState->state = state;
            return depthStencilState;
        }

        void SetDepthStencilState(const IGraphicsDepthStencilState* depthStencilState) override
        {
            const MetalDepthStencilState* metalDepthStencilState = CheckedCast<const MetalDepthStencilState>(
                const_cast<IGraphicsDepthStencilState*>(depthStencilState));
            m_currentDepthStencilState = metalDepthStencilState != nullptr ? metalDepthStencilState->state : nil;
        }

        std::shared_ptr<IGraphicsRenderTarget> CreateRenderTarget(int width, int height) override
        {
            auto rt = std::make_shared<MetalRenderTarget>();
            rt->width = width;
            rt->height = height;

            MTLTextureDescriptor* desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                                                            width:static_cast<NSUInteger>(width)
                                                                                           height:static_cast<NSUInteger>(height)
                                                                                        mipmapped:NO];
            desc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
            rt->colorTexture = [m_device newTextureWithDescriptor:desc];

            desc.pixelFormat = MTLPixelFormatDepth32Float_Stencil8;
            desc.usage = MTLTextureUsageRenderTarget;
            rt->depthTexture = [m_device newTextureWithDescriptor:desc];

            return rt->colorTexture != nil && rt->depthTexture != nil ? rt : nullptr;
        }

        void SetViewportRenderTarget(IGraphicsRenderTarget* rt) override
        {
            // Metal render pass is configured per-frame in DrawScene; this is a no-op
            // Actual render target switching is handled by the render pass descriptor.
        }

        void ClearViewportRenderTarget(IGraphicsRenderTarget* rt, const float color[4], float depth, std::uint8_t) override
        {
            // Clearing is handled inside a Metal render pass; no-op at this level.
            (void)rt; (void)color; (void)depth;
        }

        void SetVertexBuffer(const IGraphicsBuffer& buffer, std::uint32_t stride, std::uint32_t offset) override
        {
            const MetalGraphicsBuffer* metalBuffer = CheckedCast<const MetalGraphicsBuffer>(const_cast<IGraphicsBuffer*>(&buffer));
            m_vertexBuffer = metalBuffer->buffer;
            m_vertexStride = stride;
            m_vertexOffset = offset;
        }

        void SetIndexBuffer(const IGraphicsBuffer& buffer, GraphicsIndexFormat format, std::uint32_t offset) override
        {
            const MetalGraphicsBuffer* metalBuffer = CheckedCast<const MetalGraphicsBuffer>(const_cast<IGraphicsBuffer*>(&buffer));
            m_indexBuffer = metalBuffer->buffer;
            m_indexType = ToMetalIndexType(format);
            m_indexOffset = offset;
        }

        void SetPrimitiveTopology(GraphicsPrimitiveTopology topology) override
        {
            m_primitiveType = ToMetalPrimitiveType(topology);
        }

        void SetInputLayout(const IGraphicsInputLayout* inputLayout) override
        {
            const MetalInputLayout* metalInputLayout = CheckedCast<const MetalInputLayout>(const_cast<IGraphicsInputLayout*>(inputLayout));
            m_vertexDescriptor = metalInputLayout != nullptr ? metalInputLayout->vertexDescriptor : nil;
        }

        void SetVertexShader(const IGraphicsVertexShader* shader) override
        {
            const MetalVertexShader* metalShader = CheckedCast<const MetalVertexShader>(const_cast<IGraphicsVertexShader*>(shader));
            m_vertexFunction = metalShader != nullptr ? metalShader->function : nil;
        }

        void SetPixelShader(const IGraphicsPixelShader* shader) override
        {
            const MetalPixelShader* metalShader = CheckedCast<const MetalPixelShader>(const_cast<IGraphicsPixelShader*>(shader));
            m_fragmentFunction = metalShader != nullptr ? metalShader->function : nil;
        }

        void SetVertexConstantBuffer(std::uint32_t slot, const IGraphicsBuffer* buffer) override
        {
            const MetalGraphicsBuffer* metalBuffer = CheckedCast<const MetalGraphicsBuffer>(const_cast<IGraphicsBuffer*>(buffer));
            if (slot < m_vertexConstantBuffers.size())
            {
                m_vertexConstantBuffers[slot] = metalBuffer != nullptr ? metalBuffer->buffer : nil;
            }
        }

        void SetPixelConstantBuffer(std::uint32_t slot, const IGraphicsBuffer* buffer) override
        {
            const MetalGraphicsBuffer* metalBuffer = CheckedCast<const MetalGraphicsBuffer>(const_cast<IGraphicsBuffer*>(buffer));
            if (slot < m_fragmentConstantBuffers.size())
            {
                m_fragmentConstantBuffers[slot] = metalBuffer != nullptr ? metalBuffer->buffer : nil;
            }
        }

        void SetRasterizerState(const IGraphicsRasterizerState* rasterizerState) override
        {
            const MetalRasterizerState* metalRasterizerState = CheckedCast<const MetalRasterizerState>(
                const_cast<IGraphicsRasterizerState*>(rasterizerState));
            if (metalRasterizerState != nullptr)
            {
                m_cullMode = metalRasterizerState->cullMode;
                m_fillMode = metalRasterizerState->fillMode;
            }
        }

        void DrawIndexed(std::uint32_t indexCount, std::uint32_t startIndexLocation, std::int32_t baseVertexLocation) override
        {
            if (!m_currentEncoder)
            {
                return;
            }

            id<MTLRenderPipelineState> pipelineState = GetOrCreatePipelineState();
            if (!pipelineState)
            {
                return;
            }

            [m_currentEncoder setRenderPipelineState:pipelineState];
            [m_currentEncoder setCullMode:m_cullMode];
            [m_currentEncoder setTriangleFillMode:m_fillMode];
            [m_currentEncoder setFrontFacingWinding:MTLWindingCounterClockwise];

            id<MTLDepthStencilState> activeDepthStencil = m_currentDepthStencilState != nil ? m_currentDepthStencilState : m_defaultDepthStencilState;
            if (activeDepthStencil)
            {
                [m_currentEncoder setDepthStencilState:activeDepthStencil];
            }

            [m_currentEncoder setVertexBuffer:m_vertexBuffer offset:m_vertexOffset atIndex:0];
            for (std::size_t i = 0; i < m_vertexConstantBuffers.size(); ++i)
            {
                if (m_vertexConstantBuffers[i] != nil)
                {
                    [m_currentEncoder setVertexBuffer:m_vertexConstantBuffers[i] offset:0 atIndex:i + 1];
                }
            }
            for (std::size_t i = 0; i < m_fragmentConstantBuffers.size(); ++i)
            {
                if (m_fragmentConstantBuffers[i] != nil)
                {
                    [m_currentEncoder setFragmentBuffer:m_fragmentConstantBuffers[i] offset:0 atIndex:i];
                }
            }

            [m_currentEncoder drawIndexedPrimitives:m_primitiveType
                                         indexCount:indexCount
                                          indexType:m_indexType
                                        indexBuffer:m_indexBuffer
                                  indexBufferOffset:m_indexOffset + startIndexLocation * (m_indexType == MTLIndexTypeUInt16 ? 2 : 4)
                                      instanceCount:1
                                         baseVertex:baseVertexLocation
                                       baseInstance:0];
        }

    private:
        id<MTLRenderPipelineState> GetOrCreatePipelineState()
        {
            if (!m_vertexFunction || !m_fragmentFunction || !m_vertexDescriptor)
            {
                return nil;
            }

            PipelineStateKey key;
            key.vertexFunction = m_vertexFunction;
            key.fragmentFunction = m_fragmentFunction;
            key.vertexDescriptor = m_vertexDescriptor;
            key.cullMode = m_cullMode;
            key.fillMode = m_fillMode;
            key.colorFormat = m_metalLayer.pixelFormat;
            key.depthFormat = MTLPixelFormatDepth32Float_Stencil8;

            auto it = m_pipelineCache.find(key);
            if (it != m_pipelineCache.end())
            {
                return it->second;
            }

            MTLRenderPipelineDescriptor* pipelineDesc = [[MTLRenderPipelineDescriptor alloc] init];
            pipelineDesc.vertexFunction = m_vertexFunction;
            pipelineDesc.fragmentFunction = m_fragmentFunction;
            pipelineDesc.vertexDescriptor = m_vertexDescriptor;
            pipelineDesc.colorAttachments[0].pixelFormat = m_metalLayer.pixelFormat;
            pipelineDesc.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
            pipelineDesc.stencilAttachmentPixelFormat = MTLPixelFormatDepth32Float_Stencil8;

            NSError* error = nil;
            id<MTLRenderPipelineState> pipelineState = [m_device newRenderPipelineStateWithDescriptor:pipelineDesc error:&error];
            if (error != nil)
            {
                NSLog(@"Failed to create pipeline state: %@", error.localizedDescription);
                return nil;
            }

            m_pipelineCache[key] = pipelineState;
            return pipelineState;
        }

    private:
        GLFWwindow* m_window = nullptr;
        int m_width = 0;
        int m_height = 0;
        bool m_enable4xMsaa = false;
        bool m_imguiInitialized = false;

        id<MTLDevice> m_device = nil;
        id<MTLCommandQueue> m_commandQueue = nil;
        CAMetalLayer* m_metalLayer = nil;
        id<CAMetalDrawable> m_currentDrawable = nil;
        id<MTLCommandBuffer> m_currentCommandBuffer = nil;
        id<MTLRenderCommandEncoder> m_currentEncoder = nil;
        MTLRenderPassDescriptor* m_renderPassDescriptor = nil;

        id<MTLFunction> m_vertexFunction = nil;
        id<MTLFunction> m_fragmentFunction = nil;
        MTLVertexDescriptor* m_vertexDescriptor = nil;
        MTLCullMode m_cullMode = MTLCullModeNone;
        MTLTriangleFillMode m_fillMode = MTLTriangleFillModeFill;

        std::unordered_map<PipelineStateKey, id<MTLRenderPipelineState>, PipelineStateKeyHash> m_pipelineCache;

        id<MTLBuffer> m_vertexBuffer = nil;
        NSUInteger m_vertexStride = 0;
        NSUInteger m_vertexOffset = 0;
        id<MTLBuffer> m_indexBuffer = nil;
        MTLIndexType m_indexType = MTLIndexTypeUInt32;
        NSUInteger m_indexOffset = 0;
        MTLPrimitiveType m_primitiveType = MTLPrimitiveTypeTriangle;
        std::array<id<MTLBuffer>, 16> m_vertexConstantBuffers;
        std::array<id<MTLBuffer>, 16> m_fragmentConstantBuffers;

        float m_clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        float m_clearDepth = 1.0f;
        std::uint8_t m_clearStencil = 0;

        id<MTLDepthStencilState> m_defaultDepthStencilState = nil;
        id<MTLDepthStencilState> m_currentDepthStencilState = nil;
    };
}

std::unique_ptr<IGraphicsBackend> CreateMetalBackend()
{
    return std::make_unique<MetalBackend>();
}
