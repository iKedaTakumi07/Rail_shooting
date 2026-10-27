#include "Particle.hlsli"

struct Material
{
    float32_t4x4 uvTransform;
    float32_t4 color;
    int32_t enableLighting;
    int32_t useClampSampler;
    int32_t padding[2];
};
struct DirectionalLight
{
    float32_t4 color;
    float32_t3 direction;
    float intensity;
};


ConstantBuffer<Material> gMaterial : register(b0);
Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSamplerWrap : register(s0); // register(s0)
SamplerState gSamplerClamp : register(s1); // register(s1)
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);


struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
   
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float32_t4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    
    // サンプリング設定の分岐
    float32_t4 textureColor;
    if (gMaterial.useClampSampler != 0)
    {
        // Ring用 (外周の巻き込みを防ぐ)
        textureColor = gTexture.Sample(gSamplerClamp, transformedUV.xy);
    }
    else
    {
        // Plane用 (通常ループ)
        textureColor = gTexture.Sample(gSamplerWrap, transformedUV.xy);
    }
    
    // 【修正1】マテリアルカラー、テクスチャカラー、頂点カラーの正確な乗算
    output.color = gMaterial.color * textureColor * input.color;
    
    // 【修正2】微小な透過漏れを防止するため、微小値以下は discard
    if (output.color.a <= 0.001f)
    {
        discard;
    }
    
    // 【修正3】加算合成時のアルファ漏れを抑制するためのRGBプレマルチプライ処理
    output.color.rgb *= output.color.a;
    
    return output;
}

