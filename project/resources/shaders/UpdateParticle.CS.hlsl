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
struct PerFrame
{
    float32_t time;
    float32_t deltaTime;
};

static const uint32_t kMaxParticles = 1024;
static const uint32_t kParticlesPerThread = 4;
ConstantBuffer<PerFrame> gPerFrame : register(b0);
RWStructuredBuffer<Particle> gParticles : register(u0);
RWStructuredBuffer<int32_t> gFreeListIndex : register(u1);
RWStructuredBuffer<int32_t> gFreeList : register(u2);

[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    for (uint32_t i = 0; i < kParticlesPerThread; ++i)
    {
        uint32_t particleIndex = DTid.x * kParticlesPerThread + i;
        if (particleIndex >= kMaxParticles)
        {
            return;
        }

        // alphaが0のparticleは死んでいるとみなして更新しない
        if (gParticles[particleIndex].color.a > 0.0f)
        {
            float deltaTime = gPerFrame.deltaTime;
            
            // 物理挙動（移動・加速度・空気抵抗）
            gParticles[particleIndex].velocity += gParticles[particleIndex].acceleration * deltaTime;
            float dragFactor = saturate(1.0f - gParticles[particleIndex].airResistance * deltaTime);
            gParticles[particleIndex].velocity *= dragFactor;
            gParticles[particleIndex].translate += gParticles[particleIndex].velocity * deltaTime;

            // 時間経過・進行率
            gParticles[particleIndex].currentTime += deltaTime;
            float t = saturate(gParticles[particleIndex].currentTime / gParticles[particleIndex].lifeTime);
            
            // グラデーション
            gParticles[particleIndex].scale = lerp(gParticles[particleIndex].startScale, gParticles[particleIndex].endScale, t);
            gParticles[particleIndex].rotation = lerp(gParticles[particleIndex].startRotation, gParticles[particleIndex].endRotation, t);
            gParticles[particleIndex].color = lerp(gParticles[particleIndex].startColor, gParticles[particleIndex].endColor, t);
            
            // alphaが0/進行率1なのでFreeにする
            if (t >= 1.0f || gParticles[particleIndex].color.a <= 0.0f)
            {
                gParticles[particleIndex].scale = float32_t3(0.0f, 0.0f, 0.0f);
                gParticles[particleIndex].color.a = -1.0f; // 非活性化フラグ

                int32_t freeListIndex;
                InterlockedAdd(gFreeListIndex[0], 1, freeListIndex);

                if ((freeListIndex + 1) < (int32_t) kMaxParticles)
                {
                    gFreeList[freeListIndex + 1] = particleIndex;
                }
                else
                {
                    InterlockedAdd(gFreeListIndex[0], -1);
                }
            }
        }
    }
}