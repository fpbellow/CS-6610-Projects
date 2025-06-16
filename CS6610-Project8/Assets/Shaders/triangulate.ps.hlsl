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

PS_OUTPUT Main(PS_INPUT input)
{
    PS_OUTPUT output = (PS_OUTPUT) 0;
    output.color = float4(1.0f, 1.0f, 0.0f, 1.0f);
    return output;
}