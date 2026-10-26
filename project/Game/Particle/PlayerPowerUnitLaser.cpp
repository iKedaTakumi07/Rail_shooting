#include "PlayerPowerUnitLaser.h"
#include "../../Engine/3d/CPUParticle/CPUParticleManager.h"
#include "../../Engine/base/Math.h"

void PlayerPowerUnitLaser::Initialize()
{
    // 事前に読み込ませるため多分?
    CPUParticleManager::getInstance()->CreateParticleGroup("PowerUintLaser", "resources/powerUnitLaser.png", ParticleMeshType::kHalfCone);
    CPUParticleManager::getInstance()->CreateParticleGroup("PowerUintLasercircle", "resources/circle2.png", ParticleMeshType::kPlane);

    // ランダムエンジン初期化
    std::random_device seedGenerator;
    randomEngine = std::mt19937(seedGenerator());

    StartColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    EndColor = Vector4(1.0f, 1.0f, 1.0f, 0.0f);
}

void PlayerPowerUnitLaser::NewParticle(const Vector3& prePos, const Transform& emitterTransform, const Vector3& localPos)
{
    EmitterParam laserfireParam;

    auto randomFloat = [&](float min, float max) {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(randomEngine);
    };

    Matrix4x4 rotX = MakeRotateXMatrix(emitterTransform.rotate.x);
    Matrix4x4 rotY = MakeRotateYMatrix(emitterTransform.rotate.y);
    Matrix4x4 rotZ = MakeRotateZMatrix(emitterTransform.rotate.z);
    Matrix4x4 rotMat = Multiply(rotX, Multiply(rotY, rotZ));

    Vector3 worldAxisX = Normalize({ rotMat.m[0][0], rotMat.m[0][1], rotMat.m[0][2] });
    Vector3 worldAxisY = Normalize({ rotMat.m[1][0], rotMat.m[1][1], rotMat.m[1][2] });
    Vector3 worldAxisZ = Normalize({ rotMat.m[2][0], rotMat.m[2][1], rotMat.m[2][2] });

    Vector3 localOffset = {
        localPos.x * emitterTransform.scale.x,
        localPos.y * emitterTransform.scale.y,
        localPos.z * emitterTransform.scale.z
    };

    Vector3 worldOffset = {
        localOffset.x * worldAxisX.x + localOffset.y * worldAxisY.x + localOffset.z * worldAxisZ.x,
        localOffset.x * worldAxisX.y + localOffset.y * worldAxisY.y + localOffset.z * worldAxisZ.y,
        localOffset.x * worldAxisX.z + localOffset.y * worldAxisY.z + localOffset.z * worldAxisZ.z
    };

    Transform emitTransform = emitterTransform;
    emitTransform.translate += worldOffset;

    Vector3 moveDir = {
        emitterTransform.translate.x - prePos.x,
        emitterTransform.translate.y - prePos.y,
        emitterTransform.translate.z - prePos.z,
    };

    float rawLength = std::sqrt(moveDir.x * moveDir.x + moveDir.y * moveDir.y + moveDir.z * moveDir.z);
    Vector3 exhaustDir;

    if (rawLength > 0.001f) {
        exhaustDir = { -moveDir.x / rawLength, -moveDir.y / rawLength, -moveDir.z / rawLength };
    } else {

        exhaustDir = { -worldAxisZ.x, -worldAxisZ.y, -worldAxisZ.z };
    }

    float length = std::clamp(rawLength, 0.1f, 1.2f);

    float targetRotateY = std::atan2(exhaustDir.x, exhaustDir.z);
    float targetRotateX = std::atan2(-exhaustDir.y, std::sqrt(exhaustDir.x * exhaustDir.x + exhaustDir.z * exhaustDir.z));
    float targetRotateZ = 0.0f;

    Transform targetTransform = emitTransform;
    targetTransform.rotate = { targetRotateX, targetRotateY, targetRotateZ };

    float outerRadius = 0.24f;
    targetTransform.scale = { outerRadius, outerRadius, length * 0.4f };

    laserfireParam.SetRotate(targetTransform.rotate);
    laserfireParam.SetScale(targetTransform.scale);
    laserfireParam.SetEndScale({ outerRadius * 0.4f, outerRadius * 0.4f, length * 0.4f });

    laserfireParam.SetStartColor(Vector4(1.0f, 1.0f, 1.0f, 0.3f));
    laserfireParam.SetEndColor(Vector4(1.0f, 1.0f, 1.0f, 0.0f));
    laserfireParam.SetVelocity({ 0.0f, 0.0f, 0.0f });
    laserfireParam.SetLifeTime(0.08f);

    CPUParticleManager::getInstance()->Emit("PowerUintLaser", targetTransform, 1, laserfireParam);

    float innerRadius = 0.12f;
    targetTransform.scale = { innerRadius, innerRadius, length * 0.45f };

    laserfireParam.SetRotate(targetTransform.rotate);
    laserfireParam.SetScale(targetTransform.scale);
    laserfireParam.SetEndScale({ innerRadius * 0.2f, innerRadius * 0.2f, length * 0.45f });
    laserfireParam.SetStartColor(Vector4(1.0f, 0.5f, 0.0f, 0.9f));
    laserfireParam.SetEndColor(Vector4(1.0f, 0.1f, 0.0f, 0.0f));
    laserfireParam.SetVelocity({ 0.0f, 0.0f, 0.0f });
    laserfireParam.SetLifeTime(0.08f);

    CPUParticleManager::getInstance()->Emit("PowerUintLaser", targetTransform, 1, laserfireParam);

    laserfireParam.SetRotate(emitterTransform.rotate);
    laserfireParam.SetScale({ 0.03f, 0.03f, 0.03f });
    laserfireParam.SetEndScale({ 0.005f, 0.005f, 0.005f });
    laserfireParam.SetLifeTime(0.3f);
    laserfireParam.SetVelocity({ -(worldAxisX.x + randomFloat(-0.2f, 0.2f)), -(worldAxisY.y + randomFloat(-0.2f, 0.2f)), -(worldAxisZ.z + randomFloat(-0.2f, 0.2f)) });

    Vector3 circleEmitPos = emitTransform.translate;
    circleEmitPos.x += randomFloat(-0.05f, 0.05f);
    circleEmitPos.y += randomFloat(-0.05f, 0.05f);
    circleEmitPos.z += randomFloat(-0.05f, 0.05f);

    Transform circleTransform = emitterTransform;
    circleTransform.translate = circleEmitPos;

    CPUParticleManager::getInstance()->Emit("PowerUintLasercircle", circleTransform, 5, laserfireParam);
}
