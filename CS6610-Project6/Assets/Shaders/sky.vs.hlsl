struct VSInput
{
    float3 position : SV_Position;

};

struct VSOutput
{
    float4 position : SV_Position;
    float3 Texture : TEXCOORD0;
};

cbuffer PerFrame : register(b0)
{
    matrix viewprojection;
    float4 viewPos;
};


VSOutput Main(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    output.position = mul(viewprojection, float4(input.position + viewPos.xyz, 1.0));
    output.Texture = input.position;
    return output;
}
