#ifndef BUFFERSTUCT_H
#define BUFFERSTUCT_H

#include <directxmath.h>
#include <d3d11.h>

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
    
};



#endif // !1
