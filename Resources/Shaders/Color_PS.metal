#include <metal_stdlib>
using namespace metal;

struct VertexOut {
    float4 posH   [[position]];
    float4 color;
};

fragment float4 PS(VertexOut pIn [[stage_in]]) {
    return pIn.color;
}
