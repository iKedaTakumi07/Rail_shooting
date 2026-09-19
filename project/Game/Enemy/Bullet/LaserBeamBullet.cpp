#include "LaserBeamBullet.h"
#include <cmath>

#include "../../../Engine/3d/Camera.h"
#include "../../../Engine/3d/CameraManager.h"
#include "../../../Engine/3d/ModelManager.h"
#include "../../../Engine/3d/Object3d.h"
#include "../../../Engine/base/Math.h"
#include "../../../Engine/base/TextureManager.h"
#include "../../../Engine/io/Input.h"

#include "../../Particle/LaserParticle.h"
#include "../../Player/Player.h"

void LaserBeamBullet::Initialize(Vector3 pos, const Vector3& rotation)
{
    TextureManager::getInstance()->LoadTexture("resources/test/uvChecker.png");
    ModelManager::GetInstance()->LoadModel("test/enemytest.obj");

    object3d = std::make_unique<Object3d>();
    object3d->Initialize();

    model = std::make_unique<Model>();
    model->Initialize("resources/test", "enemytest.obj");
    // model->SetEvnTexturefilePath(skydox->GetTextureFilePath()); // 反射が必要なら
    object3d->SetModel(model.get());

    // 座標セット
    transform_.translate = pos;
    transform_.rotate = rotation;
    transform_.scale = { laserRadius_, laserRadius_, 1.0f };

    object3d->SetTranslate(transform_.translate);
    object3d->SetRotate(transform_.rotate);

    // パーティクルは一時敵未使用
    /* laserParticle_ = std::make_unique<LaserParticle>();
     laserParticle_->Initialize();
     laserParticle_->SetStartColor(Vector4(1.0f, 0.2f, 0.2f, 1.0f));
     laserParticle_->SetEndColor(Vector4(1.0f, 0.2f, 0.2f, 0.0f));*/

    isDead_ = false;
}

void LaserBeamBullet::Update(float deltaTime)
{
    if (isDead_) {
        return;
    }

    transform_.translate = {
        (startPos_.x + endPos_.x) * 0.5f,
        (startPos_.y + endPos_.y) * 0.5f,
        (startPos_.z + endPos_.z) * 0.5f
    };

    Vector3 dir = { endPos_.x - startPos_.x, endPos_.y - startPos_.y, endPos_.z - startPos_.z };
    float length = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);

    if (length != 0.0f) {
        dir.x /= length;
        dir.y /= length;
        dir.z /= length;

        transform_.rotate.y = std::atan2(dir.x, dir.z);
        transform_.rotate.x = std::atan2(-dir.y, std::sqrt(dir.x * dir.x + dir.z * dir.z));
    }

    /* if (laserParticle_) {
         Transform particleTransform = transform_;
         particleTransform.scale.z = length * 0.5f;
         laserParticle_->NewParticle(particleTransform);
     }*/

    transform_.scale = {
        laserRadius_,
        laserRadius_,
        length * 0.5f
    };

    object3d->SetTranslate(transform_.translate);
    object3d->SetRotate(transform_.rotate);
    object3d->SetScale(transform_.scale);
    object3d->Update();
}

void LaserBeamBullet::Draw()
{
    object3d->Draw();
}

AllAABB LaserBeamBullet::GetAllAABB() const
{
    return AllAABB();
}

AllOBB LaserBeamBullet::GetAllOBB() const
{
    AllOBB allObb;
    OBB obb;

    obb.center = transform_.translate;

    Vector3 dir = { std::sin(transform_.rotate.y) * std::cos(transform_.rotate.x),
        -std::sin(transform_.rotate.x),
        std::cos(transform_.rotate.y) * std::cos(transform_.rotate.x) };

    Vector3 up = { 0.0f, 1.0f, 0.0f };
    if (std::abs(dir.y) == 1.0f) {
        up = { 1.0f, 0.0f, 0.0f };
    }

    Vector3 right = { up.y * dir.z - up.z * dir.y, up.z * dir.x - up.x * dir.z, up.x * dir.y - up.y * dir.x };
    float rLen = std::sqrt(right.x * right.x + right.y * right.y + right.z * right.z);

    if (rLen != 0.0f) {
        right.x /= rLen;
        right.y /= rLen;
        right.z /= rLen;
    }

    Vector3 trueUp = { dir.y * right.z - dir.z * right.y, dir.z * right.x - dir.x * right.z, dir.x * right.y - dir.y * right.x };

    obb.orientations[0] = right;
    obb.orientations[1] = trueUp;
    obb.orientations[2] = dir;

    float length = std::sqrtf(std::powf(endPos_.x - startPos_.x, 2) + std::powf(endPos_.y - startPos_.y, 2) + std::powf(endPos_.z - startPos_.z, 2));
    obb.size = { laserRadius_, laserRadius_, length * 0.5f };

    allObb.wholeBox = obb;
    allObb.dividBoxes.push_back(obb);

    return allObb;
}

void LaserBeamBullet::OnCollision(Collider* other)
{ // 当たったもの次第で分岐
    // 抹消予定なし
    if (other->GetCollisionGroup() == CollisionGroup::kPlayerBullet) {
    } else if (other->GetCollisionGroup() == CollisionGroup::kPlayer) {
    } else if (other->GetCollisionGroup() == CollisionGroup::kStageObject) {
    }
}

void LaserBeamBullet::SetPositions(const Vector3& start, const Vector3& end)
{
    startPos_ = start;
    endPos_ = end;
}
