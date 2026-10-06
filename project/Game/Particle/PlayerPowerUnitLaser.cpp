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

void PlayerPowerUnitLaser::NewParticle(const Vector3& prePos, const Transform& emitterTransform, const Vector3& localPos)
{
    EmitterParam laserfireParam;
    Transform TagetTransfrom;

    auto randomFloat = [&](float min, float max) {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(randomEngine);
    };

    Vector3 dir = {
        emitterTransform.translate.x - prePos.x,
        emitterTransform.translate.y - prePos.y,
        emitterTransform.translate.z - prePos.z,
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

        TagetTransfrom = emitterTransform;
        // 推進位置に配置
        TagetTransfrom.translate.x += localPos.x;
        TagetTransfrom.translate.y += localPos.y;
        TagetTransfrom.translate.z += localPos.z;

        TagetTransfrom.rotate.x = rotateX;
        TagetTransfrom.rotate.y = rotateY;
        TagetTransfrom.rotate.z = rotateZ;

        float laserRadius = 0.3f;
        TagetTransfrom.scale = { length * 0.5f, laserRadius, laserRadius };
        EndScale.x = TagetTransfrom.scale.x;

        laserfireParam.SetRotate(TagetTransfrom.rotate);
        laserfireParam.SetEndScale(EndScale);
        laserfireParam.SetScale({ TagetTransfrom.scale });
        laserfireParam.SetStartColor({ StartColor });
        laserfireParam.SetEndColor({ EndColor });
        laserfireParam.SetVelocity({ 0.0f, 0.0f, 0.0f }); // 一応レーザ痕扱いになるはず?
        laserfireParam.SetLifeTime(0.1f);

        // えせトレイル
        CPUParticleManager::getInstance()->Emit("laser", TagetTransfrom, 1, laserfireParam);
    }

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

    laserfireParam.SetRotate(emitterTransform.rotate);
    laserfireParam.SetScale({ 0.05f, 0.05f, 0.05f });
    laserfireParam.SetEndScale({ 0.01f, 0.01f, 0.01f });
    laserfireParam.SetLifeTime(0.5f);
    laserfireParam.SetVelocity({ -(worldAxisX.x + randomFloat(0.0f, 1.0f)), -(worldAxisY.y + randomFloat(0.0f, 1.0f)), -(worldAxisZ.z + randomFloat(0.0f, 1.0f)) }); // 逆ベクトル
    Transform EmitTransform = emitterTransform;
    EmitTransform.translate += worldOffset;

    EmitTransform.translate.x += randomFloat(-0.1f, 0.1f);
    EmitTransform.translate.y += randomFloat(-0.1f, 0.1f);
    EmitTransform.translate.z += randomFloat(-0.1f, 0.1f);

    // 光的な何か
    CPUParticleManager::getInstance()->Emit("lasercircle", EmitTransform, 10, laserfireParam);
}
