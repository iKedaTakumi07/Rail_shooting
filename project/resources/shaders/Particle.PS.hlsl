#include "Particle.hlsli"

struct Material
{
    float32_t4x4 uvTransform;
    float32_t4 color;
    int32_t enableLighting;
    int32_t useClampSampler;
    int32_t padding[2];
};

ConstantBuffer<Material> gMaterial : register(b0);
Texture2D<float32_t4> gTextures[] : register(t0);
SamplerState gSamplerWrap : register(s0);
SamplerState gSamplerClamp : register(s1);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float32_t4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    
    float32_t4 textureColor;
    if (gMaterial.useClampSampler != 0)
    {
        textureColor = gTextures[input.textureIndex].Sample(gSamplerClamp, transformedUV.xy);
    }
    else
    {
        textureColor = gTextures[input.textureIndex].Sample(gSamplerWrap, transformedUV.xy);
    }
    
    output.color = gMaterial.color * textureColor * input.color;
    if (output.color.a <= 0.0f)
    {
        discard;
    }
    
    return output;
}