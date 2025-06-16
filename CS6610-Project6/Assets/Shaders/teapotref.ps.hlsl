struct PSInput
{
    float4 position : SV_Position;
    float3 normal : NORMAL;
    float3 reflectedVec : TEXCOORD0;
    float3 viewDir : TEXCOORD1;
    float3 lightDir : TEXCOORD2;
    float3 lightCol : TEXCOORD3;
};

struct PSOutput
{
    float4 color : SV_Target0;
};

TextureCube environmentMap : register(t0);
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
    
    float4 reflectedColor = environmentMap.Sample(samplerState, input.reflectedVec);
    output.color = float4(lighting + reflectedColor.rgb, 1.0);
    return output;
}