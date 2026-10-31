#include "Particle.hlsli"

struct PerView
{
    float32_t4x4 viewProjection;
    float32_t4x4 billboardMatrix;
};
struct Particle
{
    float32_t3 translate;
    float32_t currentTime;
    float32_t3 velocity;
    float32_t lifeTime;
    float32_t3 acceleration;
    float airResistance;
    
    float32_t3 scale;
    uint32_t textureIndex;
    float32_t4 color;
    float32_t3 rotation;
    float32_t padding0;

    float32_t3 startScale;
    float32_t3 endScale;
    float32_t3 startRotation;
    float32_t3 endRotation;
    float32_t4 startColor;
    float32_t4 endColor;
};
struct VertexShaderInput
{
    float32_t4 position : POSITION0;
    float32_t2 texcoord : TEXCOORD0;
    float32_t4 color : COLOR0;
};

StructuredBuffer<Particle> gParticles : register(t0);
ConstantBuffer<PerView> gPerView : register(b0);

VertexShaderOutput main(VertexShaderInput input, uint32_t instanceId : SV_InstanceID)
{
    VertexShaderOutput output;
    Particle particle = gParticles[instanceId];
    
    // スケール適用
    float3 localPos = input.position.xyz * particle.scale;
    
    // 頂点座標に適用
    float cosZ = cos(particle.rotation.z);
    float sinZ = sin(particle.rotation.z);
    float2 rotatedXY = float2(
        localPos.x * cosZ - localPos.y * sinZ,
        localPos.x * sinZ + localPos.y * cosZ
    );
    localPos.xy = rotatedXY;

    // ビルボード変換およびワールド座標の適用
    float3 worldPos = mul(float4(localPos, 1.0f), gPerView.billboardMatrix).xyz + particle.translate;
    
    output.position = mul(float4(worldPos, 1.0f), gPerView.viewProjection);
    output.texcoord = input.texcoord;
    output.color = particle.color;
    output.textureIndex = particle.textureIndex;
    
    return output;
}