#include <metal_stdlib>
using namespace metal;

struct VSConstantBuffer {
    float4x4 g_World;
    float4x4 g_View;
    float4x4 g_Proj;
    float4x4 g_WorldInvTranspose;
};

struct VertexIn {
    float3 posL    [[attribute(0)]];
    float4 color   [[attribute(1)]];
};

struct VertexOut {
    float4 posH   [[position]];
    float3 posW;
};

vertex VertexOut VS(VertexIn vIn [[stage_in]],
                    constant VSConstantBuffer& cb [[buffer(1)]]) {
    VertexOut vOut;
    float4x4 viewProj = cb.g_Proj * cb.g_View;
    float4 posW = cb.g_World * float4(vIn.posL, 1.0f);
    vOut.posH = viewProj * posW;
    vOut.posW = posW.xyz;
    return vOut;
}
