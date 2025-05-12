struct VSInput
{
    float3 position : SV_Position;
    float3 normal : NORMAL;
};

struct VSOutput
{
    float4 position : SV_Position;
    float3 normal : NORMAL;
    float3 reflectedVec : TEXCOORD0;
    float3 viewDir : TEXCOORD1;
    float3 lightDir : TEXCOORD2;
    float3 lightCol : TEXCOORD3;
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

cbuffer Light : register(b2)
{
    float4 lightPos;
    float4 lightColor;
};

VSOutput Main(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    matrix world = mul(viewprojection, modelmatrix);
    float4 vPos = float4(input.position * 0.05, 1.0);
    
    output.normal = normalize(mul(invTranspose, float4(input.normal, 0.0)).xyz);
    
    output.viewDir = normalize(viewPos.xyz - vPos.xyz);
    output.reflectedVec = reflect(-output.viewDir, output.normal);
    
    output.lightDir = normalize(lightPos.xyz - vPos.xyz);
    output.lightCol = lightColor.xyz;
    
    output.position = mul(world, vPos);
    return output;
}