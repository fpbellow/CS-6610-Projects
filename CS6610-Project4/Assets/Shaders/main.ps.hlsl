struct PSInput
{
    float4 position : SV_Position;
    float3 normal : NORMAL;
    float2 Texture : TEXCOORD0;
    float3 viewDir : TEXCOORD1;
    float3 lightDir : TEXCOORD2;
    float3 lightCol : TEXCOORD3;
};

struct PSOutput
{
    float4 color : SV_Target0;
};

cbuffer PerObject : register(b1)
{
    matrix modelmatrix;
    matrix invTranspose;
    float3 matColor;
    float shininess;
};

Texture2D diffuseTexture : register(t0);
Texture2D specularTexture : register(t1);
sampler samplerState : register(s0);

PSOutput Main(PSInput input)
{
    PSOutput output = (PSOutput) 0;
    
    //ambient
    float4 ambient = float4(matColor * 0.025, 1.0);
    
    //Diffuse component
    float4 diffTex = diffuseTexture.Sample(samplerState, input.Texture);
    float3 diffuse = 0.5 * max(dot(input.normal, input.lightDir), 0.0);
    float4 diffColor = diffTex * float4(diffuse, 1.0);
 
    
    //Specular 
    float3 halfVec = normalize(input.lightDir + input.viewDir);
    float3 specular = pow(max(dot(input.normal, halfVec), 0.0), shininess) * float(1.0);
    
    float4 specTex = specularTexture.Sample(samplerState, input.Texture);
    float4 specColor = specTex * float4(specular, 1.0);
    
    float4 color = ambient + diffColor+ specColor;
    output.color = color;
    return output;
}