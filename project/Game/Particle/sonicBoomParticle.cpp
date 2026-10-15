#include "sonicBoomParticle.h"
#include "../../Engine/3d/CPUParticle/CPUParticleManager.h"
#include "../../Engine/base/Math.h"
#include <cmath>

void sonicBoomParticle::Initialize()
{ // 事前に読み込ませるため多分?
    CPUParticleManager::getInstance()->CreateParticleGroup("sonicBoom", "resources/sonicBoom.png", ParticleMeshType::kRing);

    StartColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    EndColor = Vector4(1.0f, 1.0f, 1.0f, 0.0f);
}

void sonicBoomParticle::NewParticle(const Vector3& emitterTransformA, const Vector3& emitterTransformB) const
{
    EmitterParam laserfireParam;
    Transform TargetTransform;

    Vector3 dir = {
        emitterTransformB.x - emitterTransformA.x,
        emitterTransformB.y - emitterTransformA.y,
        emitterTransformB.z - emitterTransformA.z,
    };
    float length = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);

    if (length == 0.0f) {
        return;
    }
    float rotateY = std::atan2(dir.x, dir.z);

    // X軸回転: Y方向への傾き角
    float xzLen = std::sqrt(dir.x * dir.x + dir.z * dir.z);
    float rotateX = std::atan2(-dir.y, xzLen);

    // Z軸回転（進行方向軸周りのロール）は0に設定
    float rotateZ = 0.0f;

    TargetTransform.translate = emitterTransformA;
    TargetTransform.rotate.x = rotateX;
    TargetTransform.rotate.y = rotateY;
    TargetTransform.rotate.z = rotateZ;

    laserfireParam.SetRotate(TargetTransform.rotate);
    laserfireParam.SetScale({ 1.0f, 1.0f, 1.0f });
    laserfireParam.SetEndScale(EndScale);
    laserfireParam.SetStartColor(StartColor);
    laserfireParam.SetEndColor(EndColor);
    laserfireParam.SetVelocity({ 0.0f, 0.0f, 0.0f });
    laserfireParam.SetLifeTime(0.5f);

    CPUParticleManager::getInstance()->Emit("sonicBoom", TargetTransform, 1, laserfireParam);
}
