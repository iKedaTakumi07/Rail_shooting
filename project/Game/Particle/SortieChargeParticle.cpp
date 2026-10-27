#include "SortieChargeParticle.h"
#include "../../Engine/3d/CPUParticle/CPUParticleManager.h"
#include "../../Engine/base/Math.h"
#include <cmath>
#include <cstdint>

void SortieChargeParticle::Initialize()
{
    // 事前に読み込ませるため多分?
    CPUParticleManager::getInstance()->CreateParticleGroup("circle2", "resources/circle2.png", ParticleMeshType::kPlane, BlendMode::kBlendModeAdd);

    std::random_device seedGenerator;
    randomEngine_ = std::mt19937(seedGenerator());
}

void SortieChargeParticle::NewParticle(const Vector3& targetPos)
{
    auto randomFloat = [&](float min, float max) {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(randomEngine_);
    };

    for (uint32_t i = 0; i < MaxCount; ++i) {
        EmitterParam param;

        // 1. 単位球面上にランダムな方向ベクトルを生成
        float theta = randomFloat(0.0f, 2.0f * std::numbers::pi_v<float>);
        float phi = randomFloat(-std::numbers::pi_v<float> * 0.5f, std::numbers::pi_v<float> * 0.5f);
        float radius = randomFloat(1.2f, 2.2f); // 発生半径のバラつき

        Vector3 spawnOffset = {
            radius * std::cos(phi) * std::cos(theta),
            radius * std::sin(phi),
            radius * std::cos(phi) * std::sin(theta)
        };

        // 発生座標（目標地点の周囲）
        Transform spawnTransform;
        spawnTransform.translate = targetPos + spawnOffset;
        spawnTransform.rotate = { 0.0f, 0.0f, 0.0f };

        float lifeTime = randomFloat(0.8f, 1.0f);

        Vector3 velocity = {
            -spawnOffset.x * 1.5f / lifeTime,
            -spawnOffset.y * 1.5f / lifeTime,
            -spawnOffset.z * 1.5f / lifeTime
        };

        // 3. パラメータ設定 (外側で大きく、目標地点で小さく収縮)
        float startSize = randomFloat(0.25f, 0.45f);
        param.SetScale({ startSize, startSize, 1.0f });
        param.SetEndScale({ 0.02f, 0.02f, 1.0f });

        param.SetStartColor(StartColor);
        param.SetEndColor(EndColor);
        param.SetVelocity(velocity);
        param.SetLifeTime(lifeTime);

        CPUParticleManager::getInstance()->Emit("circle2", spawnTransform, 1, param);
    }
}
