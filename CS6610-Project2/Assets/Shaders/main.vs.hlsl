struct VSInput
{
    float3 position : SV_Position;
    float3 normal : NORMAL;
    float2 Texture : TEXCOORD0;
};

struct VSOutput
{
    float4 position : SV_Position;
};

cbuffer PerFrame : register(b0)
{
    matrix viewprojection;
};

cbuffer PerObject : register(b1)
{
    matrix modelmatrix;
};

VSOutput Main(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    matrix world = mul(viewprojection, modelmatrix);

    float4 vPos = float4(input.position * 0.05, 1.0);
    output.position = mul(world, vPos);
    return output;
}