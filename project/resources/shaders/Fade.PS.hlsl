struct FadeData
{
    float4 color;
    float factor;
    float3 padding;
};

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);
ConstantBuffer<FadeData> gFade : register(b0);

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

float4 main(VSOutput input) : SV_TARGET
{
    float4 srcColor = gTexture.Sample(gSampler, input.texcoord);
    
    // 元画面の色と指定フェード色を factor (0.0 ~ 1.0) で線形補間
    float3 finalColor = lerp(srcColor.rgb, gFade.color.rgb, saturate(gFade.factor));
    
    return float4(finalColor, srcColor.a);
}