#include "LaserParticle.h"
#include "../../Engine/3d/CPUParticle/CPUParticleManager.h"
#include "../../Engine/base/Math.h"

void LaserParticle::Initialize()
{
    // 事前に読み込ませるため多分?
    CPUParticleManager::getInstance()->CreateParticleGroup("laser_halfcone", "resources/playerLaser.png", ParticleMeshType::kHalfCone);
    CPUParticleManager::getInstance()->CreateParticleGroup("laser_cone", "resources/playerLaser.png", ParticleMeshType::kCone);
}

void LaserParticle::NewParticle(const Transform& emitterTransform) const
{
    EmitterParam laserfireParam;

    // 半径および長さスケールの基準定義
    // (HalfConeMesh はローカル高さ 2.0f のため、scale.z に 長さ * 0.5f を代入してフィットさせます)
    float baseLength = emitterTransform.scale.z;
    if (baseLength <= 0.0001f) {
        baseLength = 1.0f; // 安全デフォルト
    }

    Transform targetTransform = emitterTransform;

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

    float innerRadius = 0.8f;
    targetTransform.scale = { innerRadius, innerRadius, baseLength * 0.5f };

    laserfireParam.SetRotate(targetTransform.rotate);
    laserfireParam.SetScale(targetTransform.scale);
    laserfireParam.SetEndScale({ innerRadius * 0.2f, innerRadius * 0.2f, baseLength * 0.5f });
    laserfireParam.SetStartColor(coreStartColor_);
    laserfireParam.SetEndColor(coreEndColor_);
    laserfireParam.SetVelocity({ 0.0f, 0.0f, 0.0f });
    laserfireParam.SetLifeTime(0.1f);

    CPUParticleManager::getInstance()->Emit("laser_cone", targetTransform, 1, laserfireParam);
}
