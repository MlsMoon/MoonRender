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
    float3 normalL [[attribute(1)]];
};

struct VertexOut {
    float4 posH   [[position]];
    float3 posW;
    float3 normalW;
    float4 color;
};

vertex VertexOut VS(VertexIn vIn [[stage_in]],
                    constant VSConstantBuffer& cb [[buffer(1)]]) {
    VertexOut vOut;
    float4x4 viewProj = cb.g_Proj * cb.g_View;
    float4 posW = cb.g_World * float4(vIn.posL, 1.0f);

    vOut.posH = viewProj * posW;
    vOut.posW = posW.xyz;
    float3x3 worldInvTranspose3x3 = float3x3(
        cb.g_WorldInvTranspose[0].xyz,
        cb.g_WorldInvTranspose[1].xyz,
        cb.g_WorldInvTranspose[2].xyz);
    vOut.normalW = worldInvTranspose3x3 * vIn.normalL;
    vOut.color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    return vOut;
}
