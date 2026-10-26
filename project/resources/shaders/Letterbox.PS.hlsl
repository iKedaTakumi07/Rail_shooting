struct LetterboxData
{
    float heightRate;
    float feather;
    float2 padding;
};

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);
ConstantBuffer<LetterboxData> gLetterbox : register(b0);

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

float4 main(VSOutput input) : SV_TARGET
{
    float4 color = gTexture.Sample(gSampler, input.texcoord);
    
    // UV.y が [0.0, heightRate] または [1.0 - heightRate, 1.0] の範囲を黒にする
    float topMask = smoothstep(gLetterbox.heightRate, gLetterbox.heightRate - gLetterbox.feather, input.texcoord.y);
    float bottomMask = smoothstep(1.0f - gLetterbox.heightRate, 1.0f - gLetterbox.heightRate + gLetterbox.feather, input.texcoord.y);
    float mask = saturate(topMask + bottomMask);
    
    // マスク部分を黒に線形補間
    color.rgb = lerp(color.rgb, float3(0.0f, 0.0f, 0.0f), mask);
    return color;
}