#include "PlayerPowerUnitLaser.h"
#include "../../Engine/3d/CPUParticle/CPUParticleManager.h"
#include "../../Engine/base/Math.h"

void PlayerPowerUnitLaser::Initialize()
{
    // 事前に読み込ませるため多分?
    CPUParticleManager::getInstance()->CreateParticleGroup("laser", "resources/powerUnitLaser.png", ParticleMeshType::kPlane);
    CPUParticleManager::getInstance()->CreateParticleGroup("lasercircle", "resources/circle2.png", ParticleMeshType::kPlane);

    // ランダムエンジン初期化
    std::random_device seedGenerator;
    randomEngine = std::mt19937(seedGenerator());

    StartColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    EndColor = Vector4(1.0f, 1.0f, 1.0f, 0.0f);
}

void PlayerPowerUnitLaser::NewParticle(const Transform& emitterTransform, const Vector3& localPos)
{
    auto randomFloat = [&](float min, float max) {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(randomEngine);
    };

    Vector3 localOffset = {
        localPos.x * emitterTransform.scale.x,
        localPos.y * emitterTransform.scale.y,
        localPos.z * emitterTransform.scale.z
    };

    Matrix4x4 rotX = MakeRotateXMatrix(emitterTransform.rotate.x);
    Matrix4x4 rotY = MakeRotateYMatrix(emitterTransform.rotate.y);
    Matrix4x4 rotZ = MakeRotateZMatrix(emitterTransform.rotate.z);
    Matrix4x4 rotMat = Multiply(rotX, Multiply(rotY, rotZ));

    Vector3 worldAxisX = Normalize({ rotMat.m[0][0], rotMat.m[0][1], rotMat.m[0][2] });
    Vector3 worldAxisY = Normalize({ rotMat.m[1][0], rotMat.m[1][1], rotMat.m[1][2] });
    Vector3 worldAxisZ = Normalize({ rotMat.m[2][0], rotMat.m[2][1], rotMat.m[2][2] });

    Vector3 worldOffset = {
        localOffset.x * worldAxisX.x + localOffset.y * worldAxisY.x + localOffset.z * worldAxisZ.x,
        localOffset.x * worldAxisX.y + localOffset.y * worldAxisY.y + localOffset.z * worldAxisZ.y,
        localOffset.x * worldAxisX.z + localOffset.y * worldAxisY.z + localOffset.z * worldAxisZ.z
    };

    Vector3 TragetPos = {
        emitterTransform.translate.x + worldOffset.x,
        emitterTransform.translate.y + worldOffset.y,
        emitterTransform.translate.z + worldOffset.z
    };

    EmitterParam laserfireParam;
    Transform EmitTransform = emitterTransform;
    EmitTransform.translate.z -= worldAxisZ.z;

    for (int i = 0; i < 3; ++i) {
        float rotY = std::numbers::pi_v<float> / 2.0f;
        float rotZ = (std::numbers::pi_v<float> / 3.0f) * (float)i;

        Vector3 finalRotate = {
            emitterTransform.rotate.x,
            emitterTransform.rotate.y + rotY,
            emitterTransform.rotate.z + rotZ,
        };

        laserfireParam.SetRotate(finalRotate);
        laserfireParam.SetScale({ 0.25f, 0.25f, 0.5f });
        laserfireParam.SetStartColor({ StartColor });
        laserfireParam.SetEndColor({ EndColor });
        laserfireParam.SetVelocity({ 0.0f, 0.0f, 0.0f }); // 残像なのでその場に固定
        laserfireParam.SetLifeTime(0.10f);

        // えせトレイル
        CPUParticleManager::getInstance()->Emit("laser", EmitTransform, 1, laserfireParam);
    }

    laserfireParam.SetRotate(emitterTransform.rotate);
    laserfireParam.SetScale({ 0.1f, 0.1f, 0.1f });
    laserfireParam.SetVelocity({ -(worldAxisX.x + randomFloat(0.0f, 1.0f)), -(worldAxisY.y + randomFloat(0.0f, 1.0f)), -(worldAxisZ.z + randomFloat(0.0f, 1.0f)) }); // 逆ベクトル

    EmitTransform.translate.x += randomFloat(-0.1f, 0.1f);
    EmitTransform.translate.y += randomFloat(-0.1f, 0.1f);
    EmitTransform.translate.z += randomFloat(-0.1f, 0.1f);

    // 光的な何か
    CPUParticleManager::getInstance()->Emit("lasercircle", EmitTransform, 10, laserfireParam);
}
