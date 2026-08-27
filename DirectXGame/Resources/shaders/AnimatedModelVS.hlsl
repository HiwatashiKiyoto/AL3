#pragma pack_matrix(row_major)

cbuffer WorldTransform : register(b0)
{
    matrix world;
};

cbuffer ViewProjection : register(b1)
{
    matrix view;
    matrix projection;
    float3 cameraPos;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
};

VSOutput main(float3 position : POSITION, float3 normal : NORMAL)
{
    VSOutput output;
    output.position = mul(float4(position, 1.0f), mul(world, mul(view, projection)));
    output.normal = normalize(mul(float4(normal, 0.0f), world).xyz);
    return output;
}
