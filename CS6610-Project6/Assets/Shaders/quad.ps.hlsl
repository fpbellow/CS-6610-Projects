struct PSInput
{
    float4 position : SV_Position;
    float3 normal : NORMAL;
    float2 Texture : TEXCOORD0;
    float3 reflectedVec : TEXCOORD1;
    float3 viewDir : TEXCOORD2;
    float3 lightDir : TEXCOORD3;
    float3 lightCol : TEXCOORD4;
};

struct PSOutput
{
    float4 color : SV_Target0;
};


TextureCube environmentMap : register(t0);
Texture2D reflectedTexture : register(t1);
sampler samplerState : register(s0);


PSOutput Main(PSInput input)
{
    PSOutput output = (PSOutput) 0;
    
    //ambient
    float3 ambient = float3(1.0, 1.0, 1.0) * 0.025;
    
    //Diffuse component
    float3 diffuse = 0.5 * max(dot(input.normal, input.lightDir), 0.0) * 1.0f;
    
    //Specular 
    float3 halfVec = normalize(input.lightDir + input.viewDir);
    float3 specular = pow(max(dot(input.normal, halfVec), 0.0), 32.0f) * 1.0f;
    
    float3 lighting = diffuse + specular;
    
    float4 envReflect = environmentMap.Sample(samplerState, input.reflectedVec);
    float4 potReflect = reflectedTexture.Sample(samplerState, input.Texture);
    
    float3 result = potReflect.rgb != float3(0.0, 0.0, 0.0) ? potReflect.rgb : envReflect.rgb;

    output.color = lerp(float4(lighting * 0.4, 1.0f), float4(result, 1.0f), 0.7f);
    return output;
}