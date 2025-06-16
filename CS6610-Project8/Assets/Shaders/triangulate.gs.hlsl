struct GS_INPUT
{
    float4 position : SV_Position;
    float3 normal : NORMAL;
    float2 Texture : TEXCOORD0;
    float3 viewDir : TEXCOORD1;
    float3 lightDir : TEXCOORD2;
    float3 lightCol : TEXCOORD3;
    float4 shadowCoord : TEXCOORD4;
};

[maxvertexcount(6)]
void Main(triangle GS_INPUT input[3], inout LineStream<GS_INPUT> lineStream)
{
    GS_INPUT edge0 = input[0];
    edge0.position.y += 0.01;
    
    GS_INPUT edge1 = input[1];
    edge1.position.y += 0.01;
    
    GS_INPUT edge2 = input[2];
    edge2.position.y += 0.01;
    
    // Draw edge 0-1
    lineStream.Append(edge0);
    lineStream.Append(edge1);

    // Draw edge 1-2
    lineStream.Append(edge1);
    lineStream.Append(edge2);

    // Draw edge 2-0
    lineStream.Append(edge2);
    lineStream.Append(edge1);
}