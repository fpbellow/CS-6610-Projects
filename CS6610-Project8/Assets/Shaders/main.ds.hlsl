struct DS_INPUT
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 Texture : TEXCOORD0;
};

struct DS_OUTPUT
{
    float4 position : SV_Position;
    float3 normal : NORMAL;
    float2 Texture : TEXCOORD0;
    float3 viewDir : TEXCOORD1;
    float3 lightDir : TEXCOORD2;
    float3 lightCol : TEXCOORD3;
    float4 shadowCoord : TEXCOORD4;
};

struct HS_CONSTANT_DATA_OUTPUT
{
    float Edges[4] : SV_TessFactor;
    float Inside[2] : SV_InsideTessFactor;
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

cbuffer TessellationBuffer : register(b3)
{
    float4 tessellationFactor; //x is tessellation factor, y is displacement factor
};


Texture2D displacementMap : register(t0);


[domain("quad")]
DS_OUTPUT Main(HS_CONSTANT_DATA_OUTPUT tessFactors, const OutputPatch<DS_INPUT, 4> patch, float2 uv : SV_DomainLocation)
{
    float3 pos = lerp(lerp(patch[0].position, patch[1].position, uv.y), lerp(patch[3].position, patch[2].position, uv.y), uv.x);
    
    float2 texCoord = lerp(lerp(patch[0].Texture, patch[1].Texture, uv.y), lerp(patch[3].Texture, patch[2].Texture, uv.y), uv.x);
 
    uint2 texelCoords = uint2(texCoord * uint2(512, 512));
    float displacement = displacementMap.Load(int3(texelCoords, 0));
    
    DS_OUTPUT output;
    pos.y += displacement * tessellationFactor.y;
    
    float4 vPos = float4(pos, 1.0f);
    matrix viewprojection = mul(projection, view);

    float4 world = mul(modelmatrix, vPos);
    output.shadowCoord = mul(lightViewProj, world);
    
    output.normal = normalize(mul(invTranspose, float4(patch[0].normal, 0.0)).xyz);
    output.Texture = texCoord;
 
    output.viewDir = normalize(viewPos.xyz - world.xyz);
    
    output.lightDir = normalize(lightPos.xyz - world.xyz);
    output.lightCol = lightColor.xyz;
    
    output.position = mul(viewprojection, world);

   
    return output;
}