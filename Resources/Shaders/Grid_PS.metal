#include <metal_stdlib>
using namespace metal;

struct VertexOut {
    float4 posH   [[position]];
    float3 posW;
};

fragment float4 PS(VertexOut pIn [[stage_in]]) {
    float gridSize = 1.0f;
    float2 gridUV = pIn.posW.xz / gridSize;
    float2 derivative = fwidth(gridUV);
    float2 gridLine = abs(fract(gridUV - 0.5) - 0.5) / derivative;
    float line = min(gridLine.x, gridLine.y);
    float alpha = 1.0 - saturate(line);

    float thickLineX = abs(pIn.posW.z) < 0.05 ? 1.0 : 0.0;
    float thickLineZ = abs(pIn.posW.x) < 0.05 ? 1.0 : 0.0;

    float3 gridColor = float3(0.4, 0.4, 0.4);
    if (thickLineX > 0.5)
        gridColor = float3(1.0, 0.2, 0.2);
    if (thickLineZ > 0.5)
        gridColor = float3(0.2, 0.2, 1.0);

    return float4(gridColor, alpha * 0.5);
}
