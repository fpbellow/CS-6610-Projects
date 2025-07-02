struct PS_INPUT
{
    float4 position : SV_Position;
    float3 normal : NORMAL;
    float2 Texture : TEXCOORD0;
    float3 viewDir : TEXCOORD1;
    float3 lightDir : TEXCOORD2;
    float3 lightCol : TEXCOORD3;
    float4 shadowCoord : TEXCOORD4;
};

struct PS_OUTPUT
{
    float4 color : SV_Target0;
};

Texture2D normalMap : register(t0);
Texture2D shadowMap : register(t1);


SamplerState samLinear : register(s0);
SamplerComparisonState shadowSampler : register(s1);


PS_OUTPUT Main(PS_INPUT input)
{
    PS_OUTPUT output = (PS_OUTPUT) 0;
   
    float2 shadowTexCoords;
    shadowTexCoords.x = 0.5f + (input.shadowCoord.x / input.shadowCoord.w * 0.5f);
    shadowTexCoords.y = 0.5f - (input.shadowCoord.y / input.shadowCoord.w * 0.5f);
    float pixelDepth = input.shadowCoord.z / input.shadowCoord.w;
    
    float3 N = normalize(input.normal);
    float3 T = float3(1, 0, 0); // Tangent (U direction)
    float3 B = float3(0, 0, -1); // Bitangent (V direction, might be (0,0,1) if your V increases up)
    float3x3 TBN = float3x3(T, B, N);
    
    float3 tangentNormal = normalMap.Sample(samLinear, input.Texture).xyz * 2.0f - 1.0f;
    float3 normalWS = normalize(mul(tangentNormal, TBN));
    
    float shadow = 1;
    float NdotL = dot(normalWS, input.lightDir);
    
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
    float3 specular = pow(max(dot(normalWS, halfVec), 0.0), 32.0f) * 1.0f;
    
    float3 lighting = ambient + diffuse + specular;
    float3 shading = (1.0f - shadow) * ambient;

    output.color = float4((lighting * shadow) + shading, 1.0f);
    return output;
}