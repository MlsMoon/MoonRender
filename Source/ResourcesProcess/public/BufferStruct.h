#ifndef BUFFERSTUCT_H
#define BUFFERSTUCT_H

#include <directxmath.h>

#include "Source/Graphics/public/GraphicsTypes.h"
#include "Source/Render/LightType.h"
#include "Source/Render/MaterialParam.h"

class BufferStruct
{
public:
    struct BaseVertex
    {
        DirectX::XMFLOAT3 pos;
    };

    struct VertexPosColor : BaseVertex
    {
        DirectX::XMFLOAT4 color;
        static VertexLayoutDesc GetVertexLayout();
    };

    struct VertexPosNormalColor : BaseVertex
    {
        DirectX::XMFLOAT3 normal;
        DirectX::XMFLOAT4 color;
        static VertexLayoutDesc GetVertexLayout();
    };

    struct VertexPosNormal : BaseVertex
    {
        DirectX::XMFLOAT3 normal;
        static VertexLayoutDesc GetVertexLayout();
    };

    struct VertexPosNormalColorUV : BaseVertex
    {
        DirectX::XMFLOAT3 normal;
        DirectX::XMFLOAT4 color;
        DirectX::XMFLOAT2 uv;
        static VertexLayoutDesc GetVertexLayout();
    };

    struct ConstantMVPBuffer
    {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX proj;
        DirectX::XMMATRIX worldInvTranspose;
    };

    struct ConstantPSBuffer
    {
        DirectX::XMFLOAT4 directionalLightDirW;
    };
};

#endif
