struct VSInput
{
    float3 position : SV_Position;
    float3 normal : NORMAL;
    float2 Texture : TEXCOORD0;
};

struct VSOutput
{
    float4 position : SV_Position;
    float3 normal : NORMAL;
    float2 Texture : TEXCOORD0;
    float3 viewDir : TEXCOORD1;
    float3 lightDir : TEXCOORD2;
    float3 lightCol : TEXCOORD3;
};

cbuffer PerFrame : register(b0)
{
    matrix viewprojection;
    float4 viewPos;
    float4 lightPos;
    float4 lightColor;
};

cbuffer PerObject : register(b1)
{
    matrix modelmatrix;
    matrix invTranspose;
    float3 matColor;
    float shininess;
};

VSOutput Main(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    matrix world = mul(viewprojection, modelmatrix);
    float4 vPos = float4(input.position * 0.05, 1.0);
    
    output.normal = normalize(mul(invTranspose, float4(input.normal, 0.0)).xyz);

    output.Texture = input.Texture;
    output.viewDir = normalize(viewPos.xyz - vPos.xyz);
    output.lightDir = normalize(lightPos.xyz - vPos.xyz);
    output.lightCol = lightColor.xyz;
    
    output.position = mul(world, vPos);
    return output;
}