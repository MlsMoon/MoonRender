#include "Struct.hlsli"

VertexOut VS(VertexIn vIn)
{
    VertexOut vOut;
    matrix viewProj = mul(g_Proj, g_View);
    float4 posW = mul(g_World, float4(vIn.posL, 1.0f));

    vOut.posH = mul(viewProj, posW);
    vOut.posW = posW.xyz;
    vOut.normalW = mul((float3x3) g_WorldInvTranspose, vIn.normalL);
    vOut.color = float4(1.0f,1.0f,1.0f,1.0f);
    return vOut;
} 
