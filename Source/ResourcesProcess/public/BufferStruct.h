#ifndef BUFFERSTUCT_H
#define BUFFERSTUCT_H

#include <directxmath.h>
#include <d3d11.h>

//存储cpp中的顶点结构，和其对应的 输入布局描述，与HLSL中对应

class BufferStruct
{
public:
    
    // VertexPosColor
    // 一种顶点的储存结构
    struct VertexPosColor
    {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT4 color;
        static const D3D11_INPUT_ELEMENT_DESC inputLayout[2];
    };

    struct ConstantMVPBuffer
    {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX proj;
    };
    
};



#endif // !1

