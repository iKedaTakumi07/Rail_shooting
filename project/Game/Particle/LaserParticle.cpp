#include "LaserParticle.h"
#include "../../Engine/3d/CPUParticle/CPUParticleManager.h"
#include "../../Engine/base/Math.h"

void LaserParticle::Initialize()
{
    // 事前に読み込ませるため多分?
    CPUParticleManager::getInstance()->CreateParticleGroup("multiLaser", "resources/playerLaser.png", ParticleMeshType::kPlane, BlendMode::kBlendModeAdd);
    CPUParticleManager::getInstance()->CreateParticleGroup("laser_halfcone", "resources/powerUnitLaser.png", ParticleMeshType::kHalfCone, BlendMode::kBlendModeAdd);
    CPUParticleManager::getInstance()->CreateParticleGroup("laser_cone", "resources/powerUnitLaser.png", ParticleMeshType::kCone, BlendMode::kBlendModeAdd);
}

void LaserParticle::NewParticle(const Transform& emitterTransform) const
{
    EmitterParam laserfireParam;

    float baseLength = emitterTransform.scale.z;
    if (baseLength <= 0.0001f) {
        baseLength = 0.5f;
    }

    Transform targetTransform = emitterTransform;

    // 1. 外殻 (HalfCone)
    float outerRadius = 1.0f;
    targetTransform.scale = { outerRadius, outerRadius, baseLength * 0.5f };

    laserfireParam.SetRotate(targetTransform.rotate);
    laserfireParam.SetScale(targetTransform.scale);
    laserfireParam.SetEndScale({ outerRadius * 0.3f, outerRadius * 0.3f, baseLength * 0.5f });
    laserfireParam.SetStartColor(auraStartColor_);
    laserfireParam.SetEndColor(auraEndColor_);
    laserfireParam.SetVelocity({ 0.0f, 0.0f, 0.0f });
    laserfireParam.SetLifeTime(0.1f);

    CPUParticleManager::getInstance()->Emit("laser_halfcone", targetTransform, 1, laserfireParam);

    // 2. 内コア (Cone)
    float innerRadius = 0.4f;
    targetTransform.scale = { innerRadius, innerRadius, baseLength * 0.5f };

    laserfireParam.SetRotate(targetTransform.rotate);
    laserfireParam.SetScale(targetTransform.scale);
    laserfireParam.SetEndScale({ outerRadius * 0.2f, outerRadius * 0.2f, baseLength * 0.5f });
    laserfireParam.SetStartColor(coreStartColor_);
    laserfireParam.SetEndColor(coreEndColor_);
    laserfireParam.SetVelocity({ 0.0f, 0.0f, 0.0f });
    laserfireParam.SetLifeTime(0.1f);

    CPUParticleManager::getInstance()->Emit("laser_cone", targetTransform, 1, laserfireParam);

    for (int i = 0; i < 3; ++i) {
        float rotY = std::numbers::pi_v<float> / 2.0f;
        float rotZ = (std::numbers::pi_v<float> / 3.0f) * (float)i;

        Vector3 finalRotate = {
            emitterTransform.rotate.x,
            emitterTransform.rotate.y + rotY,
            emitterTransform.rotate.z + rotZ,
        };

        laserfireParam.SetRotate(finalRotate);
        laserfireParam.SetEndScale(EndScale);
        laserfireParam.SetScale({ 1.0f, 0.5f, 5.0f });
        laserfireParam.SetStartColor({ coreStartColor_ });
        laserfireParam.SetEndColor({ coreStartColor_ });
        laserfireParam.SetVelocity({ 0.0f, 0.0f, 0.0f }); // 残像なのでその場に固定
        laserfireParam.SetLifeTime(0.10f);

        // えせトレイル
        CPUParticleManager::getInstance()->Emit("multiLaser", emitterTransform, 1, laserfireParam);
    }
}

void LaserParticle::Update()
{
}

void LaserParticle::Draw()
{
    // パーティクルの全体描画実行あるため空。
}
