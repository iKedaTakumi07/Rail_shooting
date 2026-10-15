#include "GalaxyParticle.h"
#include "../../Engine/3d/CPUParticle/CPUParticleManager.h"
#include "../../Engine/base/Math.h"

void GalaxyParticle::Initialize()
{
    // 事前に読み込ませるため多分?
    CPUParticleManager::getInstance()->CreateParticleGroup("firstClassStar", "resources/firstClassStar.png", ParticleMeshType::kPlane);

    // ランダムエンジン初期化
    std::random_device seedGenerator;
    randomEngine = std::mt19937(seedGenerator());

    StartColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    EndColor = Vector4(1.0f, 1.0f, 1.0f, 0.0f);

    randomPos.max = { 10.0f, 10.0f, 200.0f };
    randomPos.min = { -10.0f, -10.0f, 100.0f };
}

void GalaxyParticle::NewParticle()
{
    EmitterParam laserfireParam;
    Transform ParticleTransform;

    ParticleTransform.scale = { 1.0f, 1.0f, 1.0 };
    ParticleTransform.rotate = { 0.0f, 0.0f, 0.0f };

    auto randomFloat = [&](float min, float max) {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(randomEngine);
    };

    laserfireParam.SetStartColor({ StartColor });
    laserfireParam.SetEndColor({ EndColor });
    laserfireParam.SetScale({ 1.0f, 1.0f, 1.0f });
    laserfireParam.SetEndScale({ 0.1f, 0.1f, 0.1f });

    laserfireParam.SetVelocity({ 0.0f, 0.0f, 0.0f }); // 残像なのでその場に固定

    for (int i = 0; i < 10; i++) {
        laserfireParam.SetLifeTime(randomFloat(1.0f, 3.0f));
        ParticleTransform.translate.x = randomFloat(randomPos.min.x, randomPos.max.x);
        ParticleTransform.translate.y = randomFloat(randomPos.min.y, randomPos.max.y);
        ParticleTransform.translate.z = randomFloat(randomPos.min.z, randomPos.max.z);

        CPUParticleManager::getInstance()->Emit("firstClassStar", ParticleTransform, 1, laserfireParam);
    }
}
