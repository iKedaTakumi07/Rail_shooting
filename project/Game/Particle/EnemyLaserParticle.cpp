#include "EnemyLaserParticle.h"

#include "../../Engine/3d/CPUParticle/CPUParticleManager.h"
#include "../../Engine/base/Math.h"
#include <cmath>

void EnemyLaserParticle::Initialize()
{
    // 事前に読み込ませるため多分?
    CPUParticleManager::getInstance()->CreateParticleGroup("laser", "resources/playerLaser.png", ParticleMeshType::kPlane);

    StartColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    EndColor = Vector4(1.0f, 1.0f, 1.0f, 0.0f);
}

void EnemyLaserParticle::NewParticle(const Vector3& emitterTransformA, const Vector3& emitterTransformB, const Transform& BulletTransform) const
{
    EmitterParam laserfireParam;
    Transform TagetTransfrom;

    Vector3 dir = {
        emitterTransformB.x - emitterTransformA.x,
        emitterTransformB.y - emitterTransformA.y,
        emitterTransformB.z - emitterTransformA.z,
    };
    float length = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);

    if (length == 0.0f) {
        return;
    }

    dir.x /= length;
    dir.y /= length;
    dir.z /= length;

    Vector3 xAxis = dir;

    Vector3 up = { 0.0f, 1.0f, 0.0f };

    if (std::abs(Dot(xAxis, up)) > 0.999f) {
        up = { 0.0f, 0.0f, 1.0f };
    }

    Vector3 yAxis = Normalize(
        up - xAxis * Dot(up, xAxis));

    Vector3 zAxis = Cross(xAxis, yAxis);

    for (int i = 0; i < 3; ++i) {

        float roll = (std::numbers::pi_v<float> / 3.0f) * i;

        Vector3 y = yAxis * std::cos(roll) + zAxis * std::sin(roll);
        Vector3 z = { -yAxis.x * std::sin(roll) + zAxis.x * std::cos(roll), -yAxis.y * std::sin(roll) + zAxis.y * std::cos(roll), -yAxis.z * std::sin(roll) + zAxis.z * std::cos(roll) };

        float rotateY = std::asin(-xAxis.z);
        float cosY = std::cos(rotateY);

        float rotateZ;
        float rotateX;

        if (std::abs(cosY) > 0.0001f) {
            rotateZ = std::atan2(xAxis.y, xAxis.x);
            rotateX = std::atan2(y.z, z.z);
        } else {
            rotateZ = 0.0f;
            rotateX = std::atan2(-y.x, y.y);
        }

        TagetTransfrom = BulletTransform;

        TagetTransfrom.rotate.x = rotateX;
        TagetTransfrom.rotate.y = rotateY;
        TagetTransfrom.rotate.z = rotateZ;

        float laserRadius = 0.5f;
        TagetTransfrom.scale = { length * 0.5f, laserRadius, laserRadius };

        laserfireParam.SetRotate(TagetTransfrom.rotate);
        laserfireParam.SetScale({ TagetTransfrom.scale });
        laserfireParam.SetStartColor({ StartColor });
        laserfireParam.SetEndColor({ EndColor });
        laserfireParam.SetVelocity({ 0.0f, 0.0f, 0.0f }); // 一応レーザ痕扱いになるはず?
        laserfireParam.SetLifeTime(0.1f);

        // えせトレイル
        CPUParticleManager::getInstance()->Emit("laser", TagetTransfrom, 1, laserfireParam);
    }
}

void EnemyLaserParticle::Update()
{
}

void EnemyLaserParticle::Draw()
{
    // パーティクルの全体描画実行あるため空。
}
