#ifndef BUFFERSTUCT_H
#define BUFFERSTUCT_H

#include "Source/ThirdParty/glm/glm.hpp"

#include "Source/Graphics/public/GraphicsTypes.h"

class BufferStruct
{
public:
    struct BaseVertex
    {
        glm::vec3 pos;
    };

    struct VertexPosColor : BaseVertex
    {
        glm::vec4 color;
        static VertexLayoutDesc GetVertexLayout();
    };

    struct VertexPosNormalColor : BaseVertex
    {
        glm::vec3 normal;
        glm::vec4 color;
        static VertexLayoutDesc GetVertexLayout();
    };

    struct VertexPosNormal : BaseVertex
    {
        glm::vec3 normal;
        static VertexLayoutDesc GetVertexLayout();
    };

    struct VertexPosNormalColorUV : BaseVertex
    {
        glm::vec3 normal;
        glm::vec4 color;
        glm::vec2 uv;
        static VertexLayoutDesc GetVertexLayout();
    };

    struct ConstantMVPBuffer
    {
        glm::mat4 world;
        glm::mat4 view;
        glm::mat4 proj;
        glm::mat4 worldInvTranspose;
    };

    struct ConstantPSBuffer
    {
        glm::vec4 directionalLightDirW;
    };
};

#endif
