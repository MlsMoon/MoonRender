cbuffer VSConstantBuffer : register(b0)
{
    matrix g_World;
    matrix g_View;
    matrix g_Proj;
    matrix g_WorldInvTranspose;
};

struct VertexIn
{
    float3 posL : POSITION;
    float4 color : COLOR;
};

struct VertexOut
{
    float4 posH : SV_POSITION;
    float3 posW : TEXCOORD0;
};

VertexOut VS(VertexIn vIn)
{
    VertexOut vOut;
    matrix viewProj = mul(g_Proj, g_View);
    float4 posW = mul(g_World, float4(vIn.posL, 1.0f));
    vOut.posH = mul(viewProj, posW);
    vOut.posW = posW.xyz;
    return vOut;
}
