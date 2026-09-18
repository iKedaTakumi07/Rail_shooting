#include "chargeParticle.h"
#include "../../Engine/3d/CPUParticle/CPUParticleManager.h"
#include "../../Engine/base/Math.h"

void chargeParticle::Initialize()
{
    // 事前に読み込ませるため多分?
    CPUParticleManager::getInstance()->CreateParticleGroup("gradationLine", "resources/gradationLine.png", ParticleMeshType::kRing);

    // ランダムエンジン初期化
    std::random_device seedGenerator;
    randomEngine = std::mt19937(seedGenerator());

    StartColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    EndColor = Vector4(1.0f, 1.0f, 1.0f, 0.0f);
}

void chargeParticle::NewParticle(const Transform& emitterTransform)
{
    EmitterParam laserfireParam;

    auto randomFloat = [&](float min, float max) {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(randomEngine);
    };

    for (int i = 0; i < 3; ++i) {

        laserfireParam.SetRotate({ randomFloat(-std::numbers::pi_v<float>, std::numbers::pi_v<float>), randomFloat(-std::numbers::pi_v<float>, std::numbers::pi_v<float>), 0.0f });
        laserfireParam.minScale = { 0.5f, 0.5f, 0.5f };
        laserfireParam.maxScale = { 5.0f, 5.0f, 5.0f };
        laserfireParam.SetStartColor({ StartColor });
        laserfireParam.SetEndColor({ EndColor });
        laserfireParam.SetVelocity({ 0.0f, 0.0f, 0.0f }); // その場に固定
        laserfireParam.SetLifeTime(0.8f);

       
        CPUParticleManager::getInstance()->Emit("gradationLine", emitterTransform, 1, laserfireParam);
    }
}

void chargeParticle::Update()
{
}

void chargeParticle::Draw()
{
    // パーティクルの全体描画実行あるため空。
}
