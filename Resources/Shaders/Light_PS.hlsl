#include "Struct.hlsli"

float4 PS(VertexOut pIn) : SV_Target
{
    pIn.normalW = normalize(pIn.normalW);

    float ambient = 0.3f;
    float diffuse = max(0, dot(g_DirectionalLightDirW, pIn.normalW));
    float4 litColor = ambient + diffuse;

    return litColor;
}
