struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
};

cbuffer ModelColor : register(b2)
{
    float4 modelColor;
};

float4 main(PSInput input) : SV_TARGET
{
    const float3 lightDirection = normalize(float3(-0.4f, 0.8f, -0.3f));
    const float brightness = 0.3f + saturate(dot(normalize(input.normal), lightDirection)) * 0.7f;
    return float4(modelColor.rgb * brightness, modelColor.a);
}
