#include "LaserParticle.h"
#include "../../Engine/3d/CPUParticle/CPUParticleManager.h"
#include "../../Engine/base/Math.h"

void LaserParticle::Initialize()
{
    // 事前に読み込ませるため多分?
    CPUParticleManager::getInstance()->CreateParticleGroup("laser", "resources/playerLaser.png", ParticleMeshType::kPlane);

    StartColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    EndColor = Vector4(1.0f, 1.0f, 1.0f, 0.0f);
}

void LaserParticle::NewParticle(const Transform& emitterTransform) const
{
    EmitterParam laserfireParam;
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
        laserfireParam.SetStartColor({ StartColor });
        laserfireParam.SetEndColor({ EndColor });
        laserfireParam.SetVelocity({ 0.0f, 0.0f, 0.0f }); // 残像なのでその場に固定
        laserfireParam.SetLifeTime(0.10f);

        // えせトレイル
        CPUParticleManager::getInstance()->Emit("laser", emitterTransform, 1, laserfireParam);
    }
}

void LaserParticle::Update()
{
}

void LaserParticle::Draw()
{
    // パーティクルの全体描画実行あるため空。
}
