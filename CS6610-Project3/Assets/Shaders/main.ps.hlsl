struct PSInput
{
    float4 position : SV_Position;
    float3 normal : NORMAL;
    float3 viewDir : TEXCOORD1;
    float3 lightDir : TEXCOORD2;
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

PSOutput Main(PSInput input)
{
    PSOutput output = (PSOutput) 0;
    
    //ambient
    float3 ambient = matColor * 0.025;
    
    //Diffuse component
    float3 diffuse = 0.5 * max(dot(input.normal, input.lightDir), 0.0) * matColor;
    
    //Specular 
    float3 halfVec = normalize(input.lightDir + input.viewDir);
    float3 specular = pow(max(dot(input.normal, halfVec), 0.0), shininess) * float(1.0);
    
    float3 color = ambient+diffuse+specular;
    output.color = float4(color, 1.0);
    return output;
}