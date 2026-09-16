#include "NormalMoveEnemy.h"

#include "../../../Engine/3d/CameraManager.h"
#include "../../../Engine/3d/Model.h"
#include "../../../Engine/3d/ModelManager.h"

#include "../../../Engine/base/TextureManager.h"
#include "../../Player/Player.h"

#include "../../../Engine/scene/SceneManager.h"
#include "../Bullet/EnemyHomingBullet.h"
#include "../Bullet/TargetBullet.h"

void NormalMoveEnemy::Initialize(Vector3 pos)
{
    TextureManager::getInstance()->LoadTexture("resources/test/uvChecker.png");
    ModelManager::GetInstance()->LoadModel("test/test.obj");

    object3d = std::make_unique<Object3d>();
    object3d->Initialize();

    isAvile_ = true;
    isDead_ = false;

    model = std::make_unique<Model>();
    model->Initialize("resources/test", "test.obj");
    // model->SetEvnTexturefilePath(skydox->GetTextureFilePath()); // 反射が必要なら
    object3d->SetModel(model.get());

    transform_.scale = { 1.0f, 1.0f, 1.0f };
    transform_.rotate = { 0.0f, 0.0f, 0.0f };
    transform_.translate = pos;
}

void NormalMoveEnemy::Update()
{

    float deltaTime = SceneManager::GetInstance()->GetDeltaTime();
    camera_ = CameraManager::GetInstance()->GetActiveCamera();
    Vector3 velocity = { move.x * deltaTime, move.y * deltaTime, move.z * deltaTime };

    if (transform_.translate.z - RanAwayOffset_ <= camera_->GetTranslate().z) {
        // カメラに近いので逃走を開始する
        isRanAway_ = true;
    }

    if (isRanAway_) {
        withdrawalUpdate();
        return;
    }

    transform_.translate.x += velocity.x;
    transform_.translate.y += velocity.y;
    transform_.translate.z += velocity.z;

    if (transform_.translate.x + velocity.x >= basePos.x + maxBaseMove || transform_.translate.x + velocity.x <= basePos.x - maxBaseMove) {
        move.x = -move.x;
    }
    if (transform_.translate.y + velocity.y >= basePos.y + maxBaseMove || transform_.translate.y + velocity.y <= basePos.y - maxBaseMove) {
        move.y = -move.y;
    }
    if (transform_.translate.z + velocity.z >= basePos.z + maxBaseMove || transform_.translate.z + velocity.z <= basePos.z - maxBaseMove) {
        move.z = -move.z;
    }

    BulletUpdate();

    object3d->SetTranslate(transform_.translate);
    object3d->SetRotate(transform_.rotate);
    object3d->Update();
}

void NormalMoveEnemy::Draw()
{
    object3d->Draw();

    for (auto& bullet : enemyBullet_) {
        bullet->Draw();
    }
}

AllAABB NormalMoveEnemy::GetAllAABB() const
{
    AABB aabb;
    aabb.min = { transform_.translate.x - size, transform_.translate.y - size, transform_.translate.z - size };
    aabb.max = { transform_.translate.x + size, transform_.translate.y + size, transform_.translate.z + size };

    AllAABB compound;
    compound.wholeBox = aabb;
    compound.dividBoxes.push_back(aabb); // 単一コライダーでも配列に1つ入れることで共通化
    return compound;
}

AllOBB NormalMoveEnemy::GetAllOBB() const
{
    OBB obb;
    obb.center = transform_.translate;

    Matrix4x4 rotX = MakeRotateXMatrix(transform_.rotate.x);
    Matrix4x4 rotY = MakeRotateYMatrix(transform_.rotate.y);
    Matrix4x4 rotZ = MakeRotateZMatrix(transform_.rotate.z);
    Matrix4x4 rotYX = Multiply(rotY, rotX);
    Matrix4x4 rotMat = Multiply(rotYX, rotZ);

    obb.orientations[0] = Normalize({ rotMat.m[0][0], rotMat.m[0][1], rotMat.m[0][2] });
    obb.orientations[1] = Normalize({ rotMat.m[1][0], rotMat.m[1][1], rotMat.m[1][2] });
    obb.orientations[2] = Normalize({ rotMat.m[2][0], rotMat.m[2][1], rotMat.m[2][2] });

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

void NormalMoveEnemy::OnCollision(Collider* other)
{
    // 当たったもの次第で分岐
    if (other->GetCollisionGroup() == CollisionGroup::kPlayerBullet) {
        // ダメージ処理

        health_ -= other->GetDamage();

        if (health_ <= 0) {
            isAvile_ = false; // 死亡演出作ったならそっちに移行
            isDead_ = true; // 死亡演出トリガー用
        }
    } else if (other->GetCollisionGroup() == CollisionGroup::kPlayer) {
        // お互いダメージ処理
        health_ -= 1;

        if (health_ <= 0) {
            isAvile_ = false; // 死亡演出作ったならそっちに移行
            isDead_ = true; // 死亡演出トリガー用
        }
    }
}

void NormalMoveEnemy::BulletUpdate()
{
    interval -= SceneManager::GetInstance()->GetDeltaTime();

    if (interval <= 0.0f) {
        // 弾の生成
        if (useBullet == 0) {
            std::unique_ptr<TargetBullet> newBulletEnemy = std::make_unique<TargetBullet>();
            newBulletEnemy->Initialize(transform_.translate, transform_.rotate);
            newBulletEnemy->SetTargetPosition(player_->GetTranslate());

            enemyBullet_.push_back(std::move(newBulletEnemy));
            interval = maxInterval;
        } else if (useBullet == 1) {
            std::unique_ptr<EnemyHomingBullet> newBulletEnemy = std::make_unique<EnemyHomingBullet>();
            newBulletEnemy->Initialize(transform_.translate, transform_.rotate);
            newBulletEnemy->SetTargetPosition(player_->GetTranslate());

            enemyBullet_.push_back(std::move(newBulletEnemy));
            interval = maxInterval;
        }
    }

    float currentDeltaTime = SceneManager::GetInstance()->GetDeltaTime();
    // 更新処理
    for (auto& bullet : enemyBullet_) {
        bullet->SetPlayerPos(player_->GetTranslate());
        bullet->Update(currentDeltaTime);
    }

    // 弾の削除
    std::erase_if(enemyBullet_, [](const std::unique_ptr<baseEnemyBullet>& bullet) {
        return bullet->GetIsDead(); // GetIsDead が true なら削除
    });
}

void NormalMoveEnemy::withdrawalUpdate()
{
    float DeltaTime = SceneManager::GetInstance()->GetDeltaTime();
    // 逃げる
    const float AwaySpeedX = 15.0f;
    const float AwaySpeedY = 30.0f;

    if (transform_.translate.x <= 0.0f) {
        transform_.translate.x -= AwaySpeedX * DeltaTime;
    } else {
        transform_.translate.x += AwaySpeedX * DeltaTime;
    }
    transform_.translate.y += AwaySpeedY * DeltaTime;

      if (transform_.translate.y >= 40.0f) {
        isAvile_ = false;
    }

    object3d->SetTranslate(transform_.translate);
    object3d->SetRotate(transform_.rotate);
    object3d->Update();
}
