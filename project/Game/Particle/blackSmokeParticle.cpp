#include "blackSmokeParticle.h"
#include "../../Engine/3d/CPUParticle/CPUParticleManager.h"
#include "../../Engine/base/Math.h"

void blackSmokeParticle::Initialize()
{
    // 事前に読み込ませるため多分?
    CPUParticleManager::getInstance()->CreateParticleGroup("blackSmoke", "resources/platicle/blackSmoke.png", ParticleMeshType::kPlane);
    CPUParticleManager::getInstance()->CreateParticleGroup("fireCircle", "resources/circle.png", ParticleMeshType::kPlane, BlendMode::kBlendModeAdd);

    StartColor = Vector4(0.1f, 0.1f, 0.1f, 0.8f);
    EndColor = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
    fireColor = Vector4(1.0f, 0.2f, 0.0f, 1.0f);

    std::random_device seedGenerator;
    randomEngine = std::mt19937(seedGenerator());
}

void blackSmokeParticle::NewParticle(const Transform& emitterTransform)
{
    EmitterParam laserfireParam;

    auto randomFloat = [&](float min, float max) {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(randomEngine);
    };

    Velocity.x = randomFloat(-0.5f, 0.5f);
    Velocity.z = randomFloat(-0.5f, 0.5f);
    float LifeTimer = randomFloat(2.0f, 5.0f);

    laserfireParam.SetRotate({ 0.0f, 0.0f, 0.0f });
    laserfireParam.minScale = { 0.4f, 0.4f, 0.4f };
    laserfireParam.maxScale = { 0.4f, 0.4f, 0.4f };
    laserfireParam.minEndScale = { 0.8f, 0.8f, 0.8f };
    laserfireParam.maxEndScale = { 0.8f, 0.8f, 0.8f };
    laserfireParam.SetStartColor({ StartColor });
    laserfireParam.SetEndColor({ EndColor });
    laserfireParam.SetVelocity(Velocity); // その場に固定
    laserfireParam.SetLifeTime(LifeTimer);

    CPUParticleManager::getInstance()->Emit("blackSmoke", emitterTransform, 1, laserfireParam);

    Velocity.x = randomFloat(-1.0f, 1.0f);
    Velocity.y = randomFloat(1.0f, 2.5f);
    Velocity.z = randomFloat(-1.0f, 1.0f);
    fireColor.x = randomFloat(0.8f, 1.0f);

    laserfireParam.SetRotate({ 0.0f, 0.0f, 0.0f });
    laserfireParam.minScale = { 0.1f, 0.1f, 0.1f };
    laserfireParam.maxScale = { 0.1f, 0.1f, 0.1f };
    laserfireParam.minEndScale = { 0.05f, 0.05f, 0.05f };
    laserfireParam.maxEndScale = { 0.05f, 0.05f, 0.05f };
    laserfireParam.SetStartColor({ fireColor });
    laserfireParam.SetEndColor({ fireColor });
    laserfireParam.SetVelocity(Velocity);
    laserfireParam.SetLifeTime(1.0f);
    CPUParticleManager::getInstance()->Emit("fireCircle", emitterTransform, 1, laserfireParam);
}
