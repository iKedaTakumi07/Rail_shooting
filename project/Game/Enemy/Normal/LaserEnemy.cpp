#define NOMINMAX

#include "LaserEnemy.h"

#include "../../../Engine/3d/CameraManager.h"
#include "../../../Engine/3d/Model.h"
#include "../../../Engine/3d/ModelManager.h"

#include "../../../Engine/base/TextureManager.h"
#include "../../OnCollison/CollisionManager.h"

#include "../../../Engine/scene/SceneManager.h"

#include "../Bullet/EnemyHomingBullet.h"
#include "../Bullet/LaserBeamBullet.h"
#include "../Bullet/TargetBullet.h"
#include <algorithm>

void LaserEnemy::Initialize(Vector3 pos)
{
    TextureManager::getInstance()->LoadTexture("resources/test/uvChecker.png");
    ModelManager::GetInstance()->LoadModel("test/test.obj");

    fromPointObject3d = std::make_unique<Object3d>();
    fromPointObject3d->Initialize();
    toStopObject3d = std::make_unique<Object3d>();
    toStopObject3d->Initialize();

    laserModel = std::make_unique<Model>();
    laserModel->Initialize("resources/test", "test.obj");
    fromPointObject3d->SetModel(laserModel.get());
    toStopObject3d->SetModel(laserModel.get());

    FromPointTransform_.scale = { 1.0f, 1.0f, 1.0f };
    FromPointTransform_.rotate = { 0.0f, 0.0f, 0.0f };
    FromPointTransform_.translate = pos;

    ToStopTransform_.scale = { 1.0f, 1.0f, 1.0f };
    ToStopTransform_.rotate = { 0.0f, 0.0f, 0.0f };
    ToStopTransform_.translate = pos;

    fromIsAvile_ = true; // 存在しているか
    fromIsDead_ = false; // 死んでいるか

    toIsAvile_ = true; // 存在しているか
    toIsDead_ = false; // 死んでいるか

    allIsAvile_ = true; // 両方存在しているか
    allIsDead_ = false; // 両方死んでいるか

    ResetPrevPosition(FromPointTransform_.translate);
}

void LaserEnemy::Update()
{
    camera_ = CameraManager::GetInstance()->GetActiveCamera();
    UpdatePrevPosition(FromPointTransform_.translate);

    if (FromPointTransform_.translate.z <= camera_->GetTranslate().z && ToStopTransform_.translate.z <= camera_->GetTranslate().z) {
        allIsAvile_ = false;
    }

    BulletUpdate();

    fromPointObject3d->SetTranslate(FromPointTransform_.translate);
    fromPointObject3d->SetRotate(FromPointTransform_.rotate);
    fromPointObject3d->Update();

    toStopObject3d->SetTranslate(ToStopTransform_.translate);
    toStopObject3d->SetRotate(ToStopTransform_.rotate);
    toStopObject3d->Update();
}

void LaserEnemy::Draw()
{
    if (fromIsAvile_) {
        fromPointObject3d->Draw();
    }
    if (toIsAvile_) {
        toStopObject3d->Draw();
    }

    DrawBullets();
}

AllAABB LaserEnemy::GetAllAABB() const
{
    // OBBからAABBに移行する場合この敵自体を削除推奨。(又は直線のみのレーザービームにする)

    AllAABB compound;

    AABB whole;
    if (FromPointTransform_.translate.x <= ToStopTransform_.translate.x) {
        whole.min.x = FromPointTransform_.translate.x;
        whole.max.x = ToStopTransform_.translate.x;
    } else {
        whole.max.x = FromPointTransform_.translate.x;
        whole.min.x = ToStopTransform_.translate.x;
    }
    if (FromPointTransform_.translate.y <= ToStopTransform_.translate.y) {
        whole.min.y = FromPointTransform_.translate.y;
        whole.max.y = ToStopTransform_.translate.y;
    } else {
        whole.max.y = FromPointTransform_.translate.y;
        whole.min.y = ToStopTransform_.translate.y;
    }
    if (FromPointTransform_.translate.z <= ToStopTransform_.translate.z) {
        whole.min.z = FromPointTransform_.translate.z;
        whole.max.z = ToStopTransform_.translate.z;
    } else {
        whole.max.z = FromPointTransform_.translate.z;
        whole.min.z = ToStopTransform_.translate.z;
    }
    compound.wholeBox = whole;
    AABB box;
    box.min.x = FromPointTransform_.translate.x - size;
    box.min.x = FromPointTransform_.translate.x - size;
    box.min.y = FromPointTransform_.translate.y - size;
    box.min.y = FromPointTransform_.translate.y - size;
    box.min.z = FromPointTransform_.translate.z - size;
    box.min.z = FromPointTransform_.translate.z - size;

    AABB box2;
    box2.min.x = ToStopTransform_.translate.x - size;
    box2.min.x = ToStopTransform_.translate.x - size;
    box2.min.y = ToStopTransform_.translate.y - size;
    box2.min.y = ToStopTransform_.translate.y - size;
    box2.min.z = ToStopTransform_.translate.z - size;
    box2.min.z = ToStopTransform_.translate.z - size;

    compound.dividBoxes.push_back(box);
    compound.dividBoxes.push_back(box2);

    return compound;
}

AllOBB LaserEnemy::GetAllOBB() const
{
    AllOBB compound;

    if (fromIsAvile_) {
        OBB startOBB;
        startOBB.center = FromPointTransform_.translate;
        startOBB.orientations[0] = { 1.0f, 0.0f, 0.0f };
        startOBB.orientations[1] = { 0.0f, 1.0f, 0.0f };
        startOBB.orientations[2] = { 0.0f, 0.0f, 1.0f };
        startOBB.size = { size, size, size };
        compound.dividBoxes.push_back(startOBB);
    }

    if (toIsAvile_) {
        OBB endOBB;
        endOBB.center = ToStopTransform_.translate;
        endOBB.orientations[0] = { 1.0f, 0.0f, 0.0f };
        endOBB.orientations[1] = { 0.0f, 1.0f, 0.0f };
        endOBB.orientations[2] = { 0.0f, 0.0f, 1.0f };
        endOBB.size = { size, size, size };
        compound.dividBoxes.push_back(endOBB);
    }

    // 全体範囲 (wholeBox) の計算
    Vector3 center = {
        (FromPointTransform_.translate.x + ToStopTransform_.translate.x) * 0.5f,
        (FromPointTransform_.translate.y + ToStopTransform_.translate.y) * 0.5f,
        (FromPointTransform_.translate.z + ToStopTransform_.translate.z) * 0.5f
    };

    compound.wholeBox.center = center;
    compound.wholeBox.orientations[0] = { 1.0f, 0.0f, 0.0f };
    compound.wholeBox.orientations[1] = { 0.0f, 1.0f, 0.0f };
    compound.wholeBox.orientations[2] = { 0.0f, 0.0f, 1.0f };
    compound.wholeBox.size = {
        std::abs(FromPointTransform_.translate.x - center.x) + size,
        std::abs(FromPointTransform_.translate.y - center.y) + size,
        std::abs(FromPointTransform_.translate.z - center.z) + size
    };

    return compound;
}

void LaserEnemy::OnCollision(Collider* other)
{ // 当たったもの次第で分岐
    if (other->GetCollisionGroup() == CollisionGroup::kPlayerBullet) {
        // ダメージ処理
        partsDamage(other);

    } else if (other->GetCollisionGroup() == CollisionGroup::kPlayer) {
        // ダメージ処理なし
    }
}

std::vector<Vector3> LaserEnemy::GetTargetPositions()
{
    std::vector<Vector3> positions;
    if (!allIsAvile_) {
        // 万が一のエラー対策
        positions.push_back(FromPointTransform_.translate);
    }

    if (fromIsAvile_) {
        positions.push_back(FromPointTransform_.translate);
    }
    if (toIsAvile_) {
        positions.push_back(ToStopTransform_.translate);
    }

    return positions;
}

void LaserEnemy::BulletUpdate()
{
    if (fromIsAvile_ && toIsAvile_) {
        // 両方生きているならレーザビーム発射
        if (isBulletEmpty()) {
            auto laser = std::make_unique<LaserBeamBullet>();
            laser->Initialize(FromPointTransform_.translate, { 0, 0, 0 });
            laser->SetPositions(FromPointTransform_.translate, ToStopTransform_.translate);
            AddBullet(std::move(laser));
        }
    } else {
        // 片方死んだらレーザビームを消してただの弾にする(1度きりの実行)
        if (!isLaserCleared_) {
            RemoveBulletsIf([](const baseEnemyBullet* bullet) {
                return dynamic_cast<const LaserBeamBullet*>(bullet) != nullptr;
            });
            isLaserCleared_ = true;
        }

        // 既存の弾発射ロジック
        interval -= SceneManager::GetInstance()->GetDeltaTime();
        if (interval <= 0.0f) {
            Transform BulletTransform;
            if (fromIsAvile_) {
                BulletTransform = FromPointTransform_;
            } else if (toIsAvile_) {
                BulletTransform = ToStopTransform_;
            } else {
                return;
            }
            std::unique_ptr<TargetBullet> newBulletEnemy = std::make_unique<TargetBullet>();
            newBulletEnemy->Initialize(BulletTransform.translate, BulletTransform.rotate);
            newBulletEnemy->SetTargetPosition(player_->GetTranslate());

            AddBullet(std::move(newBulletEnemy));
            interval = maxInterval;
        }
    }

    float currentDeltaTime = SceneManager::GetInstance()->GetDeltaTime();

    // 更新、削除
    if (player_) {
        UpdateBullets(currentDeltaTime, player_->GetTranslate());
    }
}

void LaserEnemy::partsDamage(Collider* other)
{
    AllOBB otherAllOBB = other->GetAllOBB();
    int damage = other->GetDamage();

    // 始点パーツの判定
    if (fromIsAvile_) {
        OBB startOBB { FromPointTransform_.translate, { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } }, { size, size, size } };
        for (const auto& otherBox : otherAllOBB.dividBoxes) {
            if (CollisionManager::CheckOBB(startOBB, otherBox)) {
                starthealth_ = std::max(0, starthealth_ - damage);
                break;
            }
        }
    }

    // 終点パーツの判定
    if (toIsAvile_) {
        OBB endOBB { ToStopTransform_.translate, { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } }, { size, size, size } };
        for (const auto& otherBox : otherAllOBB.dividBoxes) {
            if (CollisionManager::CheckOBB(endOBB, otherBox)) {
                lasthealth_ = std::max(0, lasthealth_ - damage);
                break;
            }
        }
    }

    if (starthealth_ <= 0) {
        fromIsDead_ = true;
        fromIsAvile_ = false;
    }
    if (lasthealth_ <= 0) {
        toIsDead_ = true;
        toIsAvile_ = false;
    }
    if (toIsDead_ && fromIsDead_) {
        allIsDead_ = true;
        allIsAvile_ = false;
    }
}