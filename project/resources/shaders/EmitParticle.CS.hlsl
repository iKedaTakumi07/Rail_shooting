struct EmitterSphere
{
    float32_t3 translate;
    float radius; // 射出半径
    uint32_t count; // 数
    float frequency; // 射出時間
    float frequencyTime; // 射出間隔調整用時間
    uint32_t emit; // 射出許可
    float32_t3 gravity; // 重力
    float airResistance; // 空気抵抗
    float32_t3 acceleration; // 加速度
    float particleLifeTime; //描画時間
    uint32_t textureIndex; // Textureのインデックス番号
    float32_t3 padding; // 12B
};
struct Particle
{
    float32_t3 translate;
    float32_t3 scale;
    float32_t lifeTime;
    float32_t3 velocity;
    float32_t currentTime;
    float32_t4 color;
    uint32_t textureIndex;
    float32_t3 acceleration;
    float airResistance;
   
};
struct PerFrame
{
    float32_t time;
    float32_t deltaTime;
};

float rand3dTo1d(float3 value, float3 dotDir = float3(12.9898, 78.233, 37.719))
{
    //make value smaller to avoid artefacts
    float3 smallValue = sin(value);
    //get scalar value from 3d vector
    float random = dot(smallValue, dotDir);
    //make value more random by making it bigger and then taking the factional part
    random = frac(sin(random) * 143758.5453);
    return random;
}

float3 rand3dTo3d(float3 value)
{
    return float3(
        rand3dTo1d(value, float3(12.989, 78.233, 37.719)),
        rand3dTo1d(value, float3(39.346, 11.135, 83.155)),
        rand3dTo1d(value, float3(73.156, 52.235, 09.151))
    );
}

class RandomGenerator
{
    float32_t3 seed;
    float32_t3 Generate3d()
    {
        seed = rand3dTo3d(seed);
        return seed;
    }
    float32_t Generate1d()
    {
        float32_t result = rand3dTo1d(seed);
        seed.x = result;
        return result;
    }
};

static const uint32_t kMaxParticles = 1024;
static const uint32_t kParticlesPerThread = 4;
ConstantBuffer<EmitterSphere> gEmitter : register(b0);
ConstantBuffer<PerFrame> gPerFrame : register(b1);
RWStructuredBuffer<Particle> gParticles : register(u0);
RWStructuredBuffer<int32_t> gFreeListIndex : register(u1);
RWStructuredBuffer<int32_t> gFreeList : register(u2);

[numthreads(64, 1, 1)]
void main(uint32_t3 DTid : SV_DispatchThreadID)
{
    if (gEmitter.emit == 0)
    {
        return;
    }

    RandomGenerator generator;
    generator.seed = (float3(DTid) + gPerFrame.time) * (gPerFrame.time + 0.1f);

    for (uint32_t i = 0; i < kParticlesPerThread; ++i)
    {
        uint32_t emitIdx = DTid.x * kParticlesPerThread + i;
        if (emitIdx >= gEmitter.count)
        {
            break;
        }

        int freeListIndex;
        InterlockedAdd(gFreeListIndex[0], -1, freeListIndex);

        if (freeListIndex >= 0 && freeListIndex < (int32_t) kMaxParticles)
        {
            uint32_t particleIndex = gFreeList[freeListIndex];

            float3 velDir = generator.Generate3d() * 2.0f - 1.0f;
            float3 offset = generator.Generate3d() * 2.0f - 1.0f;

            gParticles[particleIndex].scale = float3(0.3f, 0.3f, 0.3f);
            gParticles[particleIndex].translate = gEmitter.translate + normalize(offset) * gEmitter.radius * generator.Generate3d().x;
            gParticles[particleIndex].color = float4(generator.Generate3d(), 1.0f);
            gParticles[particleIndex].currentTime = 0.0f;
            gParticles[particleIndex].velocity = normalize(velDir) * 1.5f;
            gParticles[particleIndex].lifeTime = gEmitter.particleLifeTime;
            gParticles[particleIndex].textureIndex = gEmitter.textureIndex;
            gParticles[particleIndex].acceleration = gEmitter.acceleration + gEmitter.gravity;
            gParticles[particleIndex].airResistance = gEmitter.airResistance;
        }
        else
        {
            InterlockedAdd(gFreeListIndex[0], 1);
            break;
        }
    }
    
}