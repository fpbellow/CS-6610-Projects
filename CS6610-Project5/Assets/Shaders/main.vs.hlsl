struct VSInput
{
    float3 position : SV_Position;
    float2 Texture : TEXCOORD0;
};

struct VSOutput
{
    float4 position : SV_Position;
    float2 Texture : TEXCOORD0;
};

cbuffer PerFrame : register(b0)
{
    matrix viewprojection;
    float4 viewPos;
};

cbuffer PerObject : register(b1)
{
    matrix modelmatrix;
    matrix invTranspose;
};

VSOutput Main(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    matrix world = mul(viewprojection, modelmatrix);
  
    output.position = mul(world, float4(input.position, 1.0));
    output.Texture = input.Texture;
    return output;
}