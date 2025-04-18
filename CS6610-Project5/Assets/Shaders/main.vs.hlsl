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

VSOutput Main(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    output.position = float4(input.position, 1.0);
    output.Texture = input.Texture;
    return output;
}