struct VertexOut
{
    float4 posH : SV_POSITION;
    float3 posW : TEXCOORD0;
};

float4 PS(VertexOut pIn) : SV_Target
{
    float gridSize = 1.0f;
    float2 gridUV = pIn.posW.xz / gridSize;
    float2 derivative = fwidth(gridUV);
    float2 gridLine = abs(frac(gridUV - 0.5) - 0.5) / derivative;
    float lineWidth = min(gridLine.x, gridLine.y);
    float alpha = 1.0 - saturate(lineWidth);

    float thickLineX = abs(pIn.posW.z) < 0.05 ? 1.0 : 0.0;
    float thickLineZ = abs(pIn.posW.x) < 0.05 ? 1.0 : 0.0;

    float3 gridColor = float3(0.4, 0.4, 0.4);
    if (thickLineX > 0.5)
        gridColor = float3(1.0, 0.2, 0.2);
    if (thickLineZ > 0.5)
        gridColor = float3(0.2, 0.2, 1.0);

    return float4(gridColor, alpha * 0.5);
}
