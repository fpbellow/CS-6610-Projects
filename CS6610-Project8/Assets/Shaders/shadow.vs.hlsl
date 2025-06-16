struct VS_INPUT
{
    float3 position : SV_Position;
    float3 normal : NORMAL;
    float2 Texture : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 position : SV_Position;
};


cbuffer Light : register(b0)
{
    float4 lightPos;
    float4 lightColor;
    matrix lightViewProj;
};

cbuffer PerObject : register(b1)
{
    matrix modelmatrix;
    matrix invTranspose;
};

VS_OUTPUT Main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    float4 vPos = float4(input.position, 1.0);
    float4 world = mul(modelmatrix, vPos);
    
    output.position = mul(lightViewProj, world);
    return output;
}
