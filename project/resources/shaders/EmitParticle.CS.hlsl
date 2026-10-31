struct EmitterSphere
{
    float32_t3 translate;
    float radius;
    uint32_t count;
    float frequency;
    float frequencyTime;
    uint32_t emit;
    float32_t3 gravity;
    float airResistance;
    float32_t3 acceleration;
    float particleLifeTime;
    uint32_t textureIndex;
    float32_t3 padding;

    float32_t3 minVelocity;
    float32_t minSpeed;
    float32_t3 maxVelocity;
    float32_t maxSpeed;

    float32_t3 minStartScale;
    float32_t pad1;
    float32_t3 maxStartScale;
    float32_t pad2;
    float32_t3 minEndScale;
    float32_t pad3;
    float32_t3 maxEndScale;
    float32_t pad4;

    float32_t3 minStartRotation;
    float32_t pad5;
    float32_t3 maxStartRotation;
    float32_t pad6;
    float32_t3 minEndRotation;
    float32_t pad7;
    float32_t3 maxEndRotation;
    float32_t pad8;

    float32_t4 minStartColor;
    float32_t4 maxStartColor;
    float32_t4 minEndColor;
    float32_t4 maxEndColor;
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
            
            float3 r3d1 = generator.Generate3d();
            float3 r3d2 = generator.Generate3d();
            float3 r3d3 = generator.Generate3d();
            float3 r3d4 = generator.Generate3d();
            float4 r4d1 = float4(generator.Generate3d(), generator.Generate1d());
            float4 r4d2 = float4(generator.Generate3d(), generator.Generate1d());

            // 座標
            float3 offset = generator.Generate3d() * 2.0f - 1.0f;
            gParticles[particleIndex].translate = gEmitter.translate + normalize(offset) * gEmitter.radius * generator.Generate3d().x;

            // 速度
            float3 velDir = lerp(gEmitter.minVelocity, gEmitter.maxVelocity, r3d1);
            float speed = lerp(gEmitter.minSpeed, gEmitter.maxSpeed, generator.Generate1d());
            gParticles[particleIndex].velocity = velDir * speed;
            
            // スケール
            gParticles[particleIndex].startScale = lerp(gEmitter.minStartScale, gEmitter.maxStartScale, r3d2);
            gParticles[particleIndex].endScale = lerp(gEmitter.minEndScale, gEmitter.maxEndScale, r3d3);
            gParticles[particleIndex].scale = gParticles[particleIndex].startScale;

            // 回転
            gParticles[particleIndex].startRotation = lerp(gEmitter.minStartRotation, gEmitter.maxStartRotation, r3d4);
            gParticles[particleIndex].endRotation = lerp(gEmitter.minEndRotation, gEmitter.maxEndRotation, generator.Generate3d());
            gParticles[particleIndex].rotation = gParticles[particleIndex].startRotation;

            // 色
            gParticles[particleIndex].startColor = lerp(gEmitter.minStartColor, gEmitter.maxStartColor, r4d1);
            gParticles[particleIndex].endColor = lerp(gEmitter.minEndColor, gEmitter.maxEndColor, r4d2);
            gParticles[particleIndex].color = gParticles[particleIndex].startColor;
            
            // その他
            gParticles[particleIndex].currentTime = 0.0f;
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