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
    float3 viewDir : TEXCOORD0;
    float3 lightDir : TEXCOORD1;
    float3 lightCol : TEXCOORD2;
    float4 shadowCoord : TEXCOORD3;
};

cbuffer PerFrame : register(b0)
{
    matrix view;
    matrix projection;
    float4 viewPos;

};

cbuffer Light : register(b1)
{
    float4 lightPos;
    float4 lightColor;
    matrix lightViewProj;
};

cbuffer PerObject : register(b2)
{
    matrix modelmatrix;
    matrix invTranspose;
};

VSOutput Main(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    matrix viewprojection = mul(projection, view);
    
    float4 vPos = float4(input.position, 1.0);
    float4 world = mul(modelmatrix, vPos);
    
    output.shadowCoord = mul(lightViewProj, world);
    
    output.normal = normalize(mul(invTranspose, float4(input.normal, 0.0)).xyz);

 
    output.viewDir = normalize(viewPos.xyz - world.xyz);
    
    output.lightDir = normalize(lightPos.xyz - world.xyz);
    output.lightCol = lightColor.xyz;
    
    output.position = mul(viewprojection, world);
    return output;
}