#include "ExplosionParticle.h"
#include "../../Engine/3d/CPUParticle/CPUParticleManager.h"
#include "../../Engine/base/Math.h"

void ExplosionParticle::Initialize()
{ // 事前に読み込ませるため多分?
    CPUParticleManager::getInstance()->CreateParticleGroup("ExplosionFire", "resources/Cloud_Mask.png", ParticleMeshType::kPlane, BlendMode::kBlendModeAdd);

    StartColor = Vector4(1.0f, 0.5f, 0.1f, 1.0f);
    EndColor = Vector4(0.8f, 0.1f, 0.0f, 0.0f);

    CoreStartColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    CoreEndColor = Vector4(1.0f, 0.4f, 0.0f, 0.0f);

    std::random_device seedGenerator;
    randomEngine = std::mt19937(seedGenerator());
}

void ExplosionParticle::NewParticle(const Transform& emitterTransform)
{
    EmitterParam fireParam;

    auto randomFloat = [&](float min, float max) {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(randomEngine);
    };

    const int kCoreParticleCount = 6;
    for (int i = 0; i < kCoreParticleCount; i++) {
        Transform coreTrans = emitterTransform;

        // 中心の狭い範囲に配置
        coreTrans.translate.x += randomFloat(-0.05f, 0.05f);
        coreTrans.translate.y += randomFloat(-0.05f, 0.05f);
        coreTrans.translate.z += randomFloat(-0.05f, 0.05f);
        coreTrans.rotate.z = randomFloat(0.0f, 6.28f);

        // 速度は抑えめにして中心にとどまらせる
        Vector3 velocity = {
            randomFloat(-1.0f, 1.0f),
            randomFloat(-1.0f, 1.0f),
            randomFloat(-1.0f, 1.0f)
        };

        // 小さいサイズから控えめに拡大
        float startScale = randomFloat(0.3f, 0.6f);
        float endScale = startScale * randomFloat(1.2f, 1.8f);
        float lifeTimer = randomFloat(0.12f, 0.25f); // 短寿命で一瞬の閃光にする

        fireParam.SetRotate(coreTrans.rotate);
        fireParam.minScale = { startScale, startScale, startScale };
        fireParam.maxScale = { startScale, startScale, startScale };
        fireParam.minEndScale = { endScale, endScale, endScale };
        fireParam.maxEndScale = { endScale, endScale, endScale };

        // 純白 (1.0, 1.0, 1.0) を設定
        fireParam.SetStartColor(CoreStartColor);
        fireParam.SetEndColor(CoreEndColor);
        fireParam.SetVelocity(velocity);
        fireParam.SetLifeTime(lifeTimer);

        CPUParticleManager::getInstance()->Emit("ExplosionFire", coreTrans, 1, fireParam);
    }

    const int kParticleCount = 14;
    for (int i = 0; i < kParticleCount; i++) {
        Transform particleTrans = emitterTransform;

        particleTrans.translate.x += randomFloat(-0.2f, 0.2f);
        particleTrans.translate.y += randomFloat(-0.2f, 0.2f);
        particleTrans.translate.z += randomFloat(-0.2f, 0.2f);
        particleTrans.rotate.z = randomFloat(0.0f, 6.28f);

        // 大きめの速度で外側へ飛ばす
        Vector3 velocity = {
            randomFloat(-4.0f, 4.0f),
            randomFloat(-4.0f, 4.0f),
            randomFloat(-4.0f, 4.0f)
        };

        // コアより一回り大きいスケールで設定
        float startScale = randomFloat(0.7f, 1.3f);
        float endScale = startScale * randomFloat(2.5f, 3.5f);
        float lifeTimer = randomFloat(0.25f, 0.45f);

        fireParam.SetRotate(particleTrans.rotate);
        fireParam.minScale = { startScale, startScale, startScale };
        fireParam.maxScale = { startScale, startScale, startScale };
        fireParam.minEndScale = { endScale, endScale, endScale };
        fireParam.maxEndScale = { endScale, endScale, endScale };

        // オレンジカラーを設定
        fireParam.SetStartColor(StartColor);
        fireParam.SetEndColor(EndColor);
        fireParam.SetVelocity(velocity);
        fireParam.SetLifeTime(lifeTimer);

        CPUParticleManager::getInstance()->Emit("ExplosionFire", particleTrans, 1, fireParam);
    }
}
