#include "EnemyHomingBullet.h"

#include "../../../Engine/3d/Camera.h"
#include "../../../Engine/3d/CameraManager.h"
#include "../../../Engine/3d/ModelManager.h"
#include "../../../Engine/3d/Object3d.h"
#include "../../../Engine/base/Math.h"
#include "../../../Engine/base/TextureManager.h"
#include "../../../Engine/io/Input.h"

#include "../../Particle/LaserParticle.h"
#include "../../Player/Player.h"

void EnemyHomingBullet::Initialize(Vector3 pos, const Vector3& rotation)
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
    transform_.translate = pos;
    transform_.rotate = rotation;

    object3d->SetTranslate(transform_.translate);
    object3d->SetRotate(transform_.rotate);

    laserParticle_ = std::make_unique<LaserParticle>();
    laserParticle_->Initialize();
    laserParticle_->SetStartColor(Vector4(1.0f, 0.2f, 0.2f, 1.0f));
    laserParticle_->SetEndColor(Vector4(1.0f, 0.2f, 0.2f, 0.0f));
}

void EnemyHomingBullet::Update(float deltaTime)
{
    MoveUpdate();
    CheckCameraCulling();

    deathTimer_ -= deltaTime;
    if (deathTimer_ <= 0.0f) {
        isDead_ = true;
    }

    object3d->SetTranslate(transform_.translate);
    object3d->SetRotate(transform_.rotate);
    object3d->Update();

    particleTimer_ += deltaTime;
}

void EnemyHomingBullet::Draw()
{
    object3d->Draw();

    if (particleTimer_ >= kParticleInterval_) {
        // リセット
        particleTimer_ = 0.0f;
        laserParticle_->NewParticle(transform_);
    }
}

AllAABB EnemyHomingBullet::GetAllAABB() const
{
    AABB aabb;
    aabb.min = { transform_.translate.x - size, transform_.translate.y - size, transform_.translate.z - size };
    aabb.max = { transform_.translate.x + size, transform_.translate.y + size, transform_.translate.z + size };

    AllAABB compound;
    compound.wholeBox = aabb;
    compound.dividBoxes.push_back(aabb); // 単一コライダーでも配列に1つ入れることで共通化
    return compound;
}

AllOBB EnemyHomingBullet::GetAllOBB() const
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

void EnemyHomingBullet::OnCollision(Collider* other)
{
    // 当たったもの次第で分岐
    if (other->GetCollisionGroup() == CollisionGroup::kPlayerBullet) {
        // お互い抹消
        isDead_ = true;
    } else if (other->GetCollisionGroup() == CollisionGroup::kPlayer) {
        // 弾削除
        isDead_ = true;
    } else if (other->GetCollisionGroup() == CollisionGroup::kStageObject) {
        // 弾削除
        isDead_ = true;
    }
}

void EnemyHomingBullet::SetTargetPosition(Vector3 Pos)
{
    // 狙う場所を設定
    targetPos_ = Pos;

    // ターゲットへのベクトル ＝ 目的地の座標 － 現在の座標
    Vector3 direction;
    direction.x = targetPos_.x - transform_.translate.x;
    direction.y = targetPos_.y - transform_.translate.y;
    direction.z = targetPos_.z - transform_.translate.z;

    // directionを「長さが1のベクトル（正規化ベクトル）」にする
    direction = Normalize(direction);

    velocity_ = { 0.0f, 0.0f, 0.0f };

    // 加速度の「大きさ」を決め、それにターゲットの方向を掛け合わせる
    acceleration_ = direction * accelerationScalar;
}

void EnemyHomingBullet::MoveUpdate()
{
    // 弾の移動

    // 方向を検知
    Vector3 direction;
    direction.x = targetPos_.x - transform_.translate.x;
    direction.y = targetPos_.y - transform_.translate.y;
    direction.z = targetPos_.z - transform_.translate.z;

    // 正規化/方向転換
    direction = Normalize(direction);
    if (direction.z > 0.0f) {
        direction.z = 0.0f; // 後ろの攻撃を除外
    }
    acceleration_ = direction * homingPower * accelerationScalar;

    velocity_ += acceleration_;

    float currentSpeed = sqrtf(velocity_.x * velocity_.x + velocity_.y * velocity_.y + velocity_.z * velocity_.z);

    // 弾の速さが最高速度を超えていたら、最高速度に制限する
    float totalMaxSpeed = maxSpeed; // 合計の限界値を出す
    if (currentSpeed >= totalMaxSpeed) {
        // 現在の進行方向（長さ1）を計算し、それに最高速度を掛ける
        Vector3 currentDir = Normalize(velocity_);

        velocity_ = currentDir * totalMaxSpeed;
    }

    transform_.translate += velocity_;

    RoateUpdate();
}

void EnemyHomingBullet::RoateUpdate()
{
    Vector3 rotate;
    rotate.y = atan2(velocity_.x, velocity_.z);
    // 横軸方向の長さを求める
    float hypotXZ = std::hypot(velocity_.x, velocity_.z);
    rotate.x = atan2(-velocity_.y, hypotXZ);
    rotate.z = 0.0f;
    transform_.rotate = rotate;
}

void EnemyHomingBullet::CheckCameraCulling()
{
    Camera* ActiveCamera = CameraManager::GetInstance()->GetCamera("PlayMain");

    Vector3 cameraPos = ActiveCamera->GetTranslate();

    const Matrix4x4& worldMat = ActiveCamera->GetWorldMatrix();

    Vector3 cameraForward = { worldMat.m[2][0], worldMat.m[2][1], worldMat.m[2][2] };

    // 正規化
    cameraForward = Normalize(cameraForward);

    // べ黒る
    Vector3 toBullet;
    toBullet.x = transform_.translate.x - cameraPos.x;
    toBullet.y = transform_.translate.y - cameraPos.y;
    toBullet.z = transform_.translate.z - cameraPos.z;

    // 内積
    float dotProduct = cameraForward.x * toBullet.x + cameraForward.y * toBullet.y + cameraForward.z * toBullet.z;

    // カメラから離れているなら(後で検証)
    if (dotProduct < 6.0f) {
        isDead_ = true;
    }
}
