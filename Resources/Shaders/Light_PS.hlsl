#include "Struct.hlsli"

float4 PS(VertexOut pIn) : SV_Target
{
    pIn.normalW = normalize(pIn.normalW);

    float diffuse = max(0 , dot( g_DirectionalLightDirW , pIn.normalW));
    float specular = 0.0f;
    float4 litColor = diffuse + specular;
    
    return litColor;
}