#include "blackSmokeParticle.h"
#include "../../Engine/3d/CPUParticle/CPUParticleManager.h"
#include "../../Engine/base/Math.h"

void blackSmokeParticle::Initialize()
{
    // 事前に読み込ませるため多分?
    CPUParticleManager::getInstance()->CreateParticleGroup("blackSmoke", "resources/platicle/blackSmoke.png", ParticleMeshType::kPlane);

    StartColor = Vector4(0.1f, 0.1f, 0.1f, 0.8f);
    EndColor = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
}

void blackSmokeParticle::NewParticle(const Transform& emitterTransform)
{
    EmitterParam laserfireParam;

    laserfireParam.SetRotate({ 0.0f, 0.0f, 0.0f });
    laserfireParam.minScale = { 0.5f, 0.5f, 0.5f };
    laserfireParam.maxScale = { 0.5f, 0.5f, 0.5f };
    laserfireParam.SetStartColor({ StartColor });
    laserfireParam.SetEndColor({ EndColor });
    laserfireParam.SetVelocity(Velocity); // その場に固定
    laserfireParam.SetLifeTime(0.5f);

    CPUParticleManager::getInstance()->Emit("blackSmoke", emitterTransform, 1, laserfireParam);
}
