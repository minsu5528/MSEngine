cbuffer ConstantBuffer : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
};

float4 VS(float3 pos : POSITION) : SV_POSITION
{
    float4 worldPos = mul(float4(pos, 1.0f), world);
    float4 viewPos = mul(worldPos, view);
    float4 clipPos = mul(viewPos, projection);
    return clipPos;
}