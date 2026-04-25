#include <metal_stdlib>
using namespace metal;

struct PSConstantBuffer {
    float4 g_DirectionalLightDirW;
};

struct VertexOut {
    float4 posH   [[position]];
    float3 posW;
    float3 normalW;
    float4 color;
};

fragment float4 PS(VertexOut pIn [[stage_in]],
                   constant PSConstantBuffer& cb [[buffer(1)]]) {
    float3 normalW = normalize(pIn.normalW);
    float diffuse = max(0.0, dot(cb.g_DirectionalLightDirW.xyz, normalW));
    float specular = 0.0;
    float4 litColor = float4(diffuse + specular);
    return litColor;
}
