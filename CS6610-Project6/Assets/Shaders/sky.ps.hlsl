struct PSInput
{
    float4 position : SV_Position;
    float3 Texture : TEXCOORD0;
};


struct PSOutput
{
    float4 color : SV_Target0;
};

TextureCube skymap : register(t0);
sampler samplerState : register(s0);

PSOutput Main(PSInput input)
{
    PSOutput output = (PSOutput) 0;
    output.color = skymap.Sample(samplerState, input.Texture);
    return output;
}