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

    startingPointObject3d = std::make_unique<Object3d>();
    startingPointObject3d->Initialize();
    lastStopObject3d = std::make_unique<Object3d>();
    lastStopObject3d->Initialize();

    model = std::make_unique<Model>();
    model->Initialize("resources/test", "test.obj");
    // model->SetEvnTexturefilePath(skydox->GetTextureFilePath()); // 反射が必要なら
    startingPointObject3d->SetModel(model.get());
    lastStopObject3d->SetModel(model.get());

    startingPointtransform_.scale = { 1.0f, 1.0f, 1.0f };
    startingPointtransform_.rotate = { 0.0f, 0.0f, 0.0f };
    startingPointtransform_.translate = pos;

    lastStoptransform_.scale = { 1.0f, 1.0f, 1.0f };
    lastStoptransform_.rotate = { 0.0f, 0.0f, 0.0f };
    lastStoptransform_.translate = pos;

    startisAvile_ = true; // 存在しているか
    startisDead_ = false; // 死んでいるか

    lastisAvile_ = true; // 存在しているか
    lastisDead_ = false; // 死んでいるか

    AllisAvile_ = true; // 両方存在しているか
    AllisDead_ = false; // 両方死んでいるか
}

void LaserEnemy::Update()
{
    camera_ = CameraManager::GetInstance()->GetActiveCamera();

    if (startingPointtransform_.translate.z <= camera_->GetTranslate().z && lastStoptransform_.translate.z <= camera_->GetTranslate().z) {
        AllisAvile_ = false;
    }

    BulletUpdate();

    startingPointObject3d->SetTranslate(startingPointtransform_.translate);
    startingPointObject3d->SetRotate(startingPointtransform_.rotate);
    startingPointObject3d->Update();

    lastStopObject3d->SetTranslate(lastStoptransform_.translate);
    lastStopObject3d->SetRotate(lastStoptransform_.rotate);
    lastStopObject3d->Update();
}

void LaserEnemy::Draw()
{
    if (startisAvile_) {
        startingPointObject3d->Draw();
    }
    if (lastisAvile_) {
        lastStopObject3d->Draw();
    }

    for (auto& bullet : enemyBullet_) {
        bullet->Draw();
    }
}

AllAABB LaserEnemy::GetAllAABB() const
{
    // OBBからAABBに移行する場合この敵自体を削除推奨。(又は直線のみのレーザービームにする)

    AllAABB compound;

    AABB whole;
    if (startingPointtransform_.translate.x <= lastStoptransform_.translate.x) {
        whole.min.x = startingPointtransform_.translate.x;
        whole.max.x = lastStoptransform_.translate.x;
    } else {
        whole.max.x = startingPointtransform_.translate.x;
        whole.min.x = lastStoptransform_.translate.x;
    }
    if (startingPointtransform_.translate.y <= lastStoptransform_.translate.y) {
        whole.min.y = startingPointtransform_.translate.y;
        whole.max.y = lastStoptransform_.translate.y;
    } else {
        whole.max.y = startingPointtransform_.translate.y;
        whole.min.y = lastStoptransform_.translate.y;
    }
    if (startingPointtransform_.translate.z <= lastStoptransform_.translate.z) {
        whole.min.z = startingPointtransform_.translate.z;
        whole.max.z = lastStoptransform_.translate.z;
    } else {
        whole.max.z = startingPointtransform_.translate.z;
        whole.min.z = lastStoptransform_.translate.z;
    }
    compound.wholeBox = whole;
    AABB box;
    box.min.x = startingPointtransform_.translate.x - size;
    box.min.x = startingPointtransform_.translate.x - size;
    box.min.y = startingPointtransform_.translate.y - size;
    box.min.y = startingPointtransform_.translate.y - size;
    box.min.z = startingPointtransform_.translate.z - size;
    box.min.z = startingPointtransform_.translate.z - size;

    AABB box2;
    box2.min.x = lastStoptransform_.translate.x - size;
    box2.min.x = lastStoptransform_.translate.x - size;
    box2.min.y = lastStoptransform_.translate.y - size;
    box2.min.y = lastStoptransform_.translate.y - size;
    box2.min.z = lastStoptransform_.translate.z - size;
    box2.min.z = lastStoptransform_.translate.z - size;

    compound.dividBoxes.push_back(box);
    compound.dividBoxes.push_back(box2);

    return compound;
}

AllOBB LaserEnemy::GetAllOBB() const
{
    AllOBB compound;

    // 始点側のOBB構築
    OBB startOBB;
    startOBB.center = startingPointtransform_.translate;
    startOBB.orientations[0] = { 1.0f, 0.0f, 0.0f };
    startOBB.orientations[1] = { 0.0f, 1.0f, 0.0f };
    startOBB.orientations[2] = { 0.0f, 0.0f, 1.0f };
    startOBB.size = { size, size, size };

    // 終点側のOBB構築
    OBB endOBB;
    endOBB.center = lastStoptransform_.translate;
    endOBB.orientations[0] = { 1.0f, 0.0f, 0.0f };
    endOBB.orientations[1] = { 0.0f, 1.0f, 0.0f };
    endOBB.orientations[2] = { 0.0f, 0.0f, 1.0f };
    endOBB.size = { size, size, size };

    compound.dividBoxes.push_back(startOBB);
    compound.dividBoxes.push_back(endOBB);

    // 全体範囲(AABBもどき)
    Vector3 center = {
        (startingPointtransform_.translate.x + lastStoptransform_.translate.x) * 0.5f,
        (startingPointtransform_.translate.y + lastStoptransform_.translate.y) * 0.5f,
        (startingPointtransform_.translate.z + lastStoptransform_.translate.z) * 0.5f
    };

    compound.wholeBox.center = center;
    compound.wholeBox.orientations[0] = { 1.0f, 0.0f, 0.0f };
    compound.wholeBox.orientations[1] = { 0.0f, 1.0f, 0.0f };
    compound.wholeBox.orientations[2] = { 0.0f, 0.0f, 1.0f };
    compound.wholeBox.size = {
        std::abs(startingPointtransform_.translate.x - center.x) + size,
        std::abs(startingPointtransform_.translate.y - center.y) + size,
        std::abs(startingPointtransform_.translate.z - center.z) + size
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
    if (!AllisAvile_) {
        // 万が一のエラー対策
        positions.push_back(startingPointtransform_.translate);
    }

    if (startisAvile_) {
        positions.push_back(startingPointtransform_.translate);
    }
    if (lastisAvile_) {
        positions.push_back(lastStoptransform_.translate);
    }

    return positions;
}

void LaserEnemy::BulletUpdate()
{
    if (startisAvile_ && lastisAvile_) {
        // 両方生きているならレーザビーム発射
        if (enemyBullet_.empty()) {
            auto laser = std::make_unique<LaserBeamBullet>();
            laser->Initialize(startingPointtransform_.translate, { 0, 0, 0 });
            laser->SetPositions(startingPointtransform_.translate, lastStoptransform_.translate);
            enemyBullet_.push_back(std::move(laser));
        }
    } else {
        // 片方死んだらレーザビームを消してただの弾にする
        std::erase_if(enemyBullet_, [](const std::unique_ptr<baseEnemyBullet>& bullet) {
            return dynamic_cast<LaserBeamBullet*>(bullet.get()) != nullptr; // レーザーなら消す
        });

        // 既存の弾発射ロジック
        interval -= SceneManager::GetInstance()->GetDeltaTime();
        if (interval <= 0.0f) {
            Transform BulletTransform;
            if (startisAvile_) {
                BulletTransform = startingPointtransform_;
            } else if (lastisAvile_) {
                BulletTransform = lastStoptransform_;
            } else {
                return;
            }
            std::unique_ptr<TargetBullet> newBulletEnemy = std::make_unique<TargetBullet>();
            newBulletEnemy->Initialize(BulletTransform.translate, BulletTransform.rotate);
            newBulletEnemy->SetTargetPosition(player_->GetTranslate());

            enemyBullet_.push_back(std::move(newBulletEnemy));
            interval = maxInterval;
        }
    }

    float currentDeltaTime = SceneManager::GetInstance()->GetDeltaTime();
    for (auto& bullet : enemyBullet_) {
        bullet->SetPlayerPos(player_->GetTranslate());
        bullet->Update(currentDeltaTime);
    }

    // 弾の削除
    std::erase_if(enemyBullet_, [](const std::unique_ptr<baseEnemyBullet>& bullet) {
        return bullet->GetIsDead(); // GetIsDead が true なら削除
    });
}

void LaserEnemy::partsDamage(Collider* other)
{
    AllOBB otherAllOBB = other->GetAllOBB();
    AllOBB myAllOBB = GetAllOBB(); // ボス自身の現在の回転が反映されたOBB群を取得
    int damage = other->GetDamage();

    size_t obbIndex = 0;
    for (auto i = 0; i < myAllOBB.dividBoxes.size(); i++) {
        const OBB& partOBB = myAllOBB.dividBoxes[i];

        for (const auto& otherBox : otherAllOBB.dividBoxes) {

            if (i == 0 && startisAvile_) {
                for (const auto& otherBox : otherAllOBB.dividBoxes) {
                    if (CollisionManager::CheckOBB(partOBB, otherBox)) {
                        starthealth_ = std::max(0, starthealth_ - damage);
                        break; // 多重ヒット防止
                    }
                }
            } else if (i == 1 && lastisAvile_) {
                for (const auto& otherBox : otherAllOBB.dividBoxes) {
                    if (CollisionManager::CheckOBB(partOBB, otherBox)) {
                        lasthealth_ = std::max(0, lasthealth_ - damage);
                        break; // 多重ヒット防止
                    }
                }
            }
        }
    }

    // 死亡演出作るかどうかは未定
    if (starthealth_ <= 0) {
        startisDead_ = true;
        startisAvile_ = false;
    }
    if (lasthealth_ <= 0) {
        lastisDead_ = true;
        lastisAvile_ = false;
    }
    if (lastisDead_ && startisDead_) {
        AllisDead_ = true;
        AllisAvile_ = false;
    }
}