struct VS_INPUT
{
    float3 position : SV_Position;
    float3 normal : NORMAL;
    float2 Texture : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 position : SV_Position;
    float3 normal : NORMAL;
    float2 Texture : TEXCOORD0;
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

VS_OUTPUT Main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    matrix viewprojection = mul(projection, view);
    
    float4 vPos = float4(input.position.x, input.position.y+0.001, input.position.z, 1.0);
    float4 world = mul(modelmatrix, vPos);
    
    
    output.normal = normalize(mul(invTranspose, float4(input.normal, 0.0)).xyz);

    output.position = mul(viewprojection, world);
    return output;
}