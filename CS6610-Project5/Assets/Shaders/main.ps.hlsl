struct PSInput
{
    float4 position : SV_Position;
    float2 Texture : TEXCOORD0;
};

struct PSOutput
{
    float4 color : SV_Target0;
};

Texture2D renderTexture : register(t0);
sampler samplerState : register(s0);

PSOutput Main(PSInput input)
{
    PSOutput output = (PSOutput) 0;
    float4 renTex = renderTexture.Sample(samplerState, input.Texture);
    
    output.color = float4(1.0, 1.0, 1.0, 1.0) * renTex;
    return output;
}