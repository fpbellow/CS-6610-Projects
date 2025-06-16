struct PSInput
{
    float4 position : SV_Position;
    float3 normal : NORMAL;
    float3 viewDir : TEXCOORD0;
    float3 lightDir : TEXCOORD1;
    float3 lightCol : TEXCOORD2;
    float4 shadowCoord : TEXCOORD3;
};

struct PSOutput
{
    float4 color : SV_Target0;
};

Texture2D shadowMap : register(t1);
SamplerComparisonState shadowSampler : register(s1);

PSOutput Main(PSInput input)
{
    PSOutput output = (PSOutput) 0;
   
    float2 shadowTexCoords;
    shadowTexCoords.x = 0.5f + (input.shadowCoord.x / input.shadowCoord.w * 0.5f);
    shadowTexCoords.y = 0.5f - (input.shadowCoord.y / input.shadowCoord.w * 0.5f);
    float pixelDepth = input.shadowCoord.z / input.shadowCoord.w;

    float shadow = 1;
    float NdotL = dot(input.normal, input.lightDir);
    
    if ((saturate(shadowTexCoords.x) == shadowTexCoords.x) &&
        (saturate(shadowTexCoords.y) == shadowTexCoords.y) &&
        (pixelDepth > 0))
    {
        float margin = acos(saturate(NdotL));
        
        float bias = max(0.002 * (1.0f - NdotL), 0.0001);
        bias = clamp(bias, 0, 0.1);
        shadow = (shadowMap.SampleCmpLevelZero(shadowSampler, shadowTexCoords, pixelDepth + bias)).r;
    }
    
    //ambient
    float ambient = 0.03f;
    
    //Diffuse component
    float3 diffuse = 0.5 * max(NdotL, 0.0) * 1.0f;
    
    //Specular 
    float3 halfVec = normalize(input.lightDir + input.viewDir);
    float3 specular = pow(max(dot(input.normal, halfVec), 0.0), 32.0f) * 1.0f;
    
    float3 lighting = ambient + diffuse + specular;
    float3 shading = (1.0f - shadow) * ambient;

    output.color = float4((lighting * shadow) + shading, 1.0f);
    return output;
}