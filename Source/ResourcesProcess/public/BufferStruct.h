#ifndef BUFFERSTUCT_H
#define BUFFERSTUCT_H

#include <directxmath.h>
#include <d3d11.h>
#include "Source/Render/LightType.h"
#include "Source/Render/MaterialParam.h"


//存储cpp中的Buffer结构，与HLSL中对应

class BufferStruct
{
public:
    struct BaseVertex
    {
        DirectX::XMFLOAT3 pos;
    };
    
    struct VertexPosColor:BaseVertex
    {
        DirectX::XMFLOAT4 color;
        static const D3D11_INPUT_ELEMENT_DESC inputLayout[2];
    };

    struct VertexPosNormalColor:BaseVertex
    {
        DirectX::XMFLOAT3 normal;
        DirectX::XMFLOAT4 color;
        static const D3D11_INPUT_ELEMENT_DESC inputLayout[3];
    };

    struct VertexPosNormal:BaseVertex
    {
        DirectX::XMFLOAT3 normal;
        static const D3D11_INPUT_ELEMENT_DESC inputLayout[2];
    };

    struct VertexPosNormalColorUV:BaseVertex
    {
        DirectX::XMFLOAT3 normal;
        DirectX::XMFLOAT4 color;
        DirectX::XMFLOAT2 uv;
        static const D3D11_INPUT_ELEMENT_DESC inputLayout[4];
    };

    //-------------------------------------------
    //Constant:

    struct ConstantMVPBuffer
    {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX proj;
        DirectX::XMMATRIX worldInvTranspose;
    };

    struct ConstantPSBuffer
    {
        Render::DirectionalLight dirLight;
        Render::MaterialParam material;
        DirectX::XMFLOAT4 eyePos;
    };
    
    
};



#endif // !1

