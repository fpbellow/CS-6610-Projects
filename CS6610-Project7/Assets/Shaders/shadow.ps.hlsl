struct PSInput
{
    float4 position : SV_Position;
};

struct PSOutput
{
    float4 color : SV_Target0;
};

PSOutput Main(PSInput input)
{
    PSOutput output = (PSOutput) 0;
    output.color = input.position;
    return output;
}