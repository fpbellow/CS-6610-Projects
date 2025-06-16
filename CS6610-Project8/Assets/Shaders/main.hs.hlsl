struct HS_INPUT
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 Texture : TEXCOORD0;
};

struct HS_CONSTANT_DATA_OUTPUT
{
    float Edges[4] : SV_TessFactor;
    float Inside[2] : SV_InsideTessFactor;
};

struct HS_OUTPUT
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 Texture : TEXCOORD0;
};

cbuffer TessellationBuffer : register(b0)
{
    float4 tessellationFactor;
};


HS_CONSTANT_DATA_OUTPUT HSConst(InputPatch<HS_INPUT, 4> ip, uint patchID : SV_PrimitiveID)
{
    HS_CONSTANT_DATA_OUTPUT output;
    output.Edges[0] = output.Edges[1] = output.Edges[2] = output.Edges[3] = tessellationFactor.r;
    output.Inside[0] = output.Inside[1] = tessellationFactor.r;
    return output;
}



// Hull shader main: just pass through the control points
[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("HSConst")]
HS_OUTPUT Main(InputPatch<HS_INPUT, 4> patch, uint i : SV_OutputControlPointID, uint patchID : SV_PrimitiveID)
{
    HS_OUTPUT output;
    output.position = patch[i].position;
    output.normal = patch[i].normal;
    output.Texture = patch[i].Texture;
    return output;
}