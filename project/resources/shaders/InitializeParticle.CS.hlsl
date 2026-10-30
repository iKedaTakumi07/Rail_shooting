struct Particle
{
    float32_t3 translate;
    float32_t3 scale;
    float32_t lifeTime;
    float32_t3 velocity;
    float32_t currentTime;
    float32_t4 color;
    uint32_t textureIndex;
};

static const uint32_t kMaxParticles = 1024;
RWStructuredBuffer<Particle> gParticles : register(u0);
RWStructuredBuffer<int32_t> gFreeListIndex : register(u1);
RWStructuredBuffer<int32_t> gFreeList : register(u2);

[numthreads(1024, 1, 1)]
void main(uint32_t3 DTid : SV_DispatchThreadID)
{
    uint32_t particleIndex = DTid.x;
    if (particleIndex < kMaxParticles)
    {
        // Particle構造体の全要素を0で埋めるという書き方
        gParticles[particleIndex] = (Particle) 0;
        gParticles[particleIndex].scale = float32_t3(0.5f, 0.5f, 0.5f);
        gParticles[particleIndex].color = float32_t4(1.0f, 1.0f, 1.0f, 0.0f);
        gParticles[particleIndex].currentTime = 0.01f;
        gParticles[particleIndex].lifeTime = 2.0f;
        gFreeList[particleIndex] = particleIndex;
        if (particleIndex < kMaxParticles)
        {
            gParticles[particleIndex] = (Particle) 0;
            gFreeList[particleIndex] = particleIndex;
        }
        if (particleIndex == 0)
        {
            gFreeListIndex[0] = kMaxParticles - 1;
        }
    }

}