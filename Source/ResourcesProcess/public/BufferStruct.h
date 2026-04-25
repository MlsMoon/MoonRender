#ifndef BUFFERSTUCT_H
#define BUFFERSTUCT_H

#include "Source/Math/public/MoonMath.h"

#include "Source/Graphics/public/GraphicsTypes.h"

class BufferStruct
{
public:
    struct BaseVertex
    {
        MoonVector3 pos;
    };

    struct VertexPosColor : BaseVertex
    {
        MoonVector4 color;
        static VertexLayoutDesc GetVertexLayout();
    };

    struct VertexPosNormalColor : BaseVertex
    {
        MoonVector3 normal;
        MoonVector4 color;
        static VertexLayoutDesc GetVertexLayout();
    };

    struct VertexPosNormal : BaseVertex
    {
        MoonVector3 normal;
        static VertexLayoutDesc GetVertexLayout();
    };

    struct VertexPosNormalColorUV : BaseVertex
    {
        MoonVector3 normal;
        MoonVector4 color;
        MoonVector2 uv;
        static VertexLayoutDesc GetVertexLayout();
    };

    struct ConstantMVPBuffer
    {
        MoonMatrix4x4 world;
        MoonMatrix4x4 view;
        MoonMatrix4x4 proj;
        MoonMatrix4x4 worldInvTranspose;
    };

    struct ConstantPSBuffer
    {
        MoonVector4 directionalLightDirW;
    };
};

#endif
