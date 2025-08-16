// VertexShader.hlsl
struct VS_INPUT
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
};

cbuffer ConstantBuffer : register(b0)
{
    row_major float4x4 World;
    row_major float4x4 View;
    row_major float4x4 Projection;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT o;
    float4 p = float4(input.pos, 1.0f);
    float4 wp = mul(World, p);
    float4 vp = mul(View, wp);
    o.pos = mul(Projection, vp);

    o.normal = mul((float3x3) World, input.normal);
    return o;
}
