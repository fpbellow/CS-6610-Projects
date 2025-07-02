struct VS_INPUT
{
    float3 position : SV_Position;
    float3 normal : NORMAL;
    float2 Texture : TEXCOORD0;
};

struct VS_OUTPUT
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 Texture : TEXCOORD0;
};


VS_OUTPUT Main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    
    output.normal = input.normal;
    output.Texture = input.Texture;
    output.position = input.position;
    
    return output;
}