#include "PlayerBullet.h"

#include "../../Engine/3d/Camera.h"
#include "../../Engine/3d/ModelManager.h"
#include "../../Engine/3d/Object3d.h"
#include "../../Engine/base/Math.h"
#include "../../Engine/base/TextureManager.h"
#include "../../Engine/io/Input.h"
#include "../Enemy/EnemyManager.h"
#include "../Enemy/base/baseEnemy.h"

#include "../Particle/LaserParticle.h"

void PlayerBullet::Initialize(Camera* camera, const Vector3& position, const Vector3& rotation)
{
    TextureManager::getInstance()->LoadTexture("resources/test/uvChecker.png");
    ModelManager::GetInstance()->LoadModel("test/test.obj");

    object3d = std::make_unique<Object3d>();
    object3d->Initialize();

    model = std::make_unique<Model>();
    model->Initialize("resources/test", "test.obj");
    // model->SetEvnTexturefilePath(skydox->GetTextureFilePath()); // 反射が必要なら
    object3d->SetModel(model.get());

    // 座標セット
    transform_.translate = position;
    transform_.rotate = rotation;

    velocity_.x = -std::sin(transform_.rotate.z) * speed_;
    velocity_.y = -std::sin(transform_.rotate.x) * speed_;
    velocity_.z = std::cos(transform_.rotate.z) * speed_;

    // 進行方向ベクトルからモデルの向きを計算
    transform_.rotate.y = std::atan2(velocity_.x, velocity_.z);
    float hypotXZ = std::hypot(velocity_.x, velocity_.z);
    transform_.rotate.x = std::atan2(-velocity_.y, hypotXZ);
    transform_.rotate.z = rotation.z;

    Vector3 renderRotate = transform_.rotate;
    renderRotate.x *= -1.0f;

    object3d->SetTranslate(transform_.translate);
    object3d->SetRotate(renderRotate);

    laserParticle_ = std::make_unique<LaserParticle>();
    laserParticle_->Initialize();
    laserParticle_->SetStartColor(Vector4(0.5f, 1.0f, 0.5f, 1.0f));
    laserParticle_->SetEndColor(Vector4(0.5f, 1.0f, 0.5f, 0.0f));
}

void PlayerBullet::Update(float deltaTime)
{
    baseEnemy* target = nullptr;
    if (targetId_ != 0 && enemyManager_) {
        target = enemyManager_->GetEnemyById(targetId_);
        float HomingUp = 0.005f; // 時間経過で追尾強化
        if (homingStrength_ <= 1.0f) {
            homingStrength_ += HomingUp;
        }
    }

    if (target && target->GetIsAvile_()) { // ターゲットが存在し、生きている場合
        std::vector<Vector3> targetPositions = target->GetTargetPositions();

        if (!targetPositions.empty()) {
            Vector3 targetPos = targetPositions[0];

            // 指定された番号の座標を取得
            if (targetIndex_ >= 0 && targetIndex_ < targetPositions.size()) {
                targetPos = targetPositions[targetIndex_];
            }

            // ベクトル計算
            Vector3 toTarget = {
                targetPos.x - transform_.translate.x,
                targetPos.y - transform_.translate.y,
                targetPos.z - transform_.translate.z
            };

            // 正規化
            float dist = std::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y + toTarget.z * toTarget.z);
            if (dist > 0.0f) {
                toTarget.x /= dist;
                toTarget.y /= dist;
                toTarget.z /= dist;
            }

            // 速度ベクトルを正規化
            Vector3 currentDir = velocity_;
            float currentSpeed = std::sqrt(currentDir.x * currentDir.x + currentDir.y * currentDir.y + currentDir.z * currentDir.z);
            if (currentSpeed > 0.0f) {
                currentDir.x /= currentSpeed;
                currentDir.y /= currentSpeed;
                currentDir.z /= currentSpeed;
            }

            // 現在の進行方向とターゲット方向を線形補間(Lerp)して曲げる
            currentDir.x = std::lerp(currentDir.x, toTarget.x, homingStrength_);
            currentDir.y = std::lerp(currentDir.y, toTarget.y, homingStrength_);
            currentDir.z = std::lerp(currentDir.z, toTarget.z, homingStrength_);

            // 再度正規化して速度を掛け直す
            float newDirLen = std::sqrt(currentDir.x * currentDir.x + currentDir.y * currentDir.y + currentDir.z * currentDir.z);
            if (newDirLen > 0.0f) {
                velocity_.x = (currentDir.x / newDirLen) * speed_;
                velocity_.y = (currentDir.y / newDirLen) * speed_;
                velocity_.z = (currentDir.z / newDirLen) * speed_;
            }
        }
    }

    // 弾の移動
    transform_.translate.x += velocity_.x * deltaTime;
    transform_.translate.y += velocity_.y * deltaTime;
    transform_.translate.z += velocity_.z * deltaTime;

    deathTimer_ -= deltaTime;
    if (deathTimer_ <= 0.0f) {
        isDead_ = true;
    }

    // 弾の向きをベクトルにする
    Vector3 rotate;
    rotate.y = atan2(velocity_.x, velocity_.z);
    // 横軸方向の長さを求める
    float hypotXZ = std::hypot(velocity_.x, velocity_.z);
    rotate.x = atan2(-velocity_.y, hypotXZ);
    rotate.z = 0.0f;
    transform_.rotate = rotate;

    object3d->SetTranslate(transform_.translate);
    object3d->SetRotate(transform_.rotate);
    object3d->Update();

    particleTimer_ += deltaTime;
}

void PlayerBullet::Draw()
{
    object3d->Draw();

    if (particleTimer_ >= kParticleInterval_) {
        // リセット
        particleTimer_ = 0.0f;
        laserParticle_->NewParticle(transform_);
    }
}

AllAABB PlayerBullet::GetAllAABB() const
{
    AABB aabb;
    aabb.min = { transform_.translate.x - size, transform_.translate.y - size, transform_.translate.z - size };
    aabb.max = { transform_.translate.x + size, transform_.translate.y + size, transform_.translate.z + size };

    AllAABB compound;
    compound.wholeBox = aabb;
    compound.dividBoxes.push_back(aabb); // 単一コライダーでも配列に1つ入れることで共通化
    return compound;
}

AllOBB PlayerBullet::GetAllOBB() const
{
    OBB obb;
    obb.center = transform_.translate;

    Vector3 forward = velocity_;
    float sqLength = Dot(forward, forward);

    if (sqLength == 0.0f) {
        forward = { 0.0f, 0.0f, 1.0f };
    } else {
        forward = Normalize(forward);
    }

    Vector3 up = { 0.0f, 1.0f, 0.0f };
    if (std::abs(forward.y) == 1.0f) {
        up = { 0.0f, 0.0f, 1.0f };
    }

    Vector3 right = Normalize(Cross(up, forward));
    Vector3 localUp = Cross(forward, right);

    obb.orientations[0] = right; // ローカル X 軸
    obb.orientations[1] = localUp; // ローカル Y 軸
    obb.orientations[2] = forward; // ローカル Z 軸

    obb.size = {
        size * transform_.scale.x,
        size * transform_.scale.y,
        size * transform_.scale.z
    };

    AllOBB compound;
    compound.wholeBox = obb;
    compound.dividBoxes.push_back(obb);
    return compound;
}

void PlayerBullet::OnCollision(Collider* other)
{
    // 当たったもの次第で分岐
    if (other->GetCollisionGroup() == CollisionGroup::kEnenmy) {
        isDead_ = true;
    } else if (other->GetCollisionGroup() == CollisionGroup::kStageObject) {
        // 弾削除
        isDead_ = true;
    } else if (other->GetCollisionGroup() == CollisionGroup::kEnemyBullet) {
        // 耐久力を消費
        life--;

        // 0なら消滅
        if (life <= 0) {
            isDead_ = true;
        }
    }
}

int PlayerBullet::GetDamage() const
{
    if (isChargeBullet) {
        return ChageDameg;
    }

    return Dameg;
}
