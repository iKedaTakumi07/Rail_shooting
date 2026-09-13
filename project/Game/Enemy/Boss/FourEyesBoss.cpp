#define NOMINMAX

#include "FourEyesBoss.h"
#include <algorithm>
#include <numbers>

#include "../../../Engine/3d/CameraManager.h"
#include "../../../Engine/3d/Model.h"
#include "../../../Engine/3d/ModelManager.h"

#include "../../../Engine/base/TextureManager.h"
#include "../../Player/Player.h"

#include "../../../Engine/3d/Object3d.h"
#include "../../../Engine/scene/SceneManager.h"
#include "../Bullet/EnemyHomingBullet.h"
#include "../Bullet/TargetBullet.h"
#include <memory>

void FourEyesBoss::Initialize(Vector3 pos)
{
    // スポーン位置を中心座標としてセット
    centerPos_ = pos;
    deadPos_ = pos;

    TextureManager::getInstance()->LoadTexture("resources/baseEnemy/uvChecker.png");
    TextureManager::getInstance()->LoadTexture("resources/EnemyUI/bossHpBar.png");
    TextureManager::getInstance()->LoadTexture("resources/EnemyUI/bossHpTank.png");
    TextureManager::getInstance()->LoadTexture("resources/EnemyUI/WARNING.png");
    ModelManager::GetInstance()->LoadModel("baseEnemy/bossEnemy.obj");

    object3d = std::make_unique<Object3d>();
    object3d->Initialize();

    isAvile_ = true;
    isDead_ = false;

    model = std::make_unique<Model>();
    model->Initialize("resources/baseEnemy", "bossEnemy.obj");
    // model->SetEvnTexturefilePath(skydox->GetTextureFilePath()); // 反射が必要なら
    object3d->SetModel(model.get());

    transform_.scale = { 1.0f, 1.0f, 1.0f };
    transform_.rotate = { 0.0f, 0.0f, 0.0f };
    transform_.translate = centerPos_;

    BossMaxHpUI = std::make_unique<Sprite>();
    BossMaxHpUI->Initialize("resources/EnemyUI/bossHpTank.png");
    BossMaxHpUI->SetPosition(Vector2(5.0f, 200.0f));

    BossHpUI = std::make_unique<Sprite>();
    BossHpUI->Initialize("resources/EnemyUI/bossHpBar.png");
    BossHpUI->SetPosition(Vector2(5.0f, 200.0f));

    BossWarning = std::make_unique<Sprite>();
    BossWarning->Initialize("resources/EnemyUI/WARNING.png");
    BossWarning->SetPosition(Vector2(0.0f, 0.0f));

    parts_[0] = { { 0.0f, -6.0f, 3.0f }, { -1.5f, -7.5f, -3.0f }, { +1.5f, -4.5f, +1.5f }, 200 }; // 下 (Bottom)
    parts_[1] = { { 0.0f, 6.0f, 3.0f }, { -1.5f, +4.5f, -3.0f }, { +1.5f, +7.5f, +1.5f }, 200 }; // 上 (Top)
    parts_[2] = { { -6.0f, 0.0f, -3.0f }, { -7.5f, -1.5f, -3.0f }, { -4.5f, +1.5f, +1.5f }, 200 }; // 右 (Right)
    parts_[3] = { { 6.0f, 0.0f, -3.0f }, { +4.5f, -1.5f, -3.0f }, { +7.5f, +1.5f, +1.5f }, 200 }; // 左 (Left)
}

void FourEyesBoss::Update()
{
    float deltaTime = SceneManager::GetInstance()->GetDeltaTime();
    camera_ = CameraManager::GetInstance()->GetActiveCamera();

    if (isAppearing_) {
        // 出現演出中は演出ロジックのみを更新（攻撃は行わない）
        UpdateAppearance(deltaTime);
        return;
    }
    if (isDeathProdiction_) {
        UpdateDeathProduction(deltaTime);
        return;
    }

    // 発射処理
    MoveUpdate();
    FireFourWayBullets();
    UIUpdate();

    object3d->Update();
}

void FourEyesBoss::Draw()
{
    object3d->Draw();

    for (const auto& bullet : enemyBullet_) {
        if (bullet) {
            bullet->Draw();
        }
    }
}

void FourEyesBoss::SpriteDraw()
{
    BossMaxHpUI->Draw();
    BossHpUI->Draw();

    if (isAppearing_) {
        const float kBlinkInterval = 0.1f; // 点滅周期
        if (std::fmod(appearanceTimer_, kBlinkInterval * 2.0f) > kBlinkInterval) {
            BossWarning->Draw();
        }
    }
}

void FourEyesBoss::SetHp(int num)
{
    currentHp_ = num;
    maxHp_ = currentHp_;
    int partsHp = currentHp_ / 4;
    for (auto& part : parts_) {
        part.hp = partsHp;
    }
}

AllAABB FourEyesBoss::GetAllAABB() const
{
    AllAABB compound;

    AABB whole;
    whole.min = { transform_.translate.x - 7.5f, transform_.translate.y - 7.5f, transform_.translate.z - 3.0f };
    whole.max = { transform_.translate.x + 7.5f, transform_.translate.y + 7.5f, transform_.translate.z + 1.5f };
    compound.wholeBox = whole;

    // ===== 変更: 生存している部位(HP > 0)のAABBのみ登録 =====
    for (const auto& part : parts_) {
        if (part.hp > 0) {
            AABB box;
            box.min = { transform_.translate.x + part.aabbMinOffset.x,
                transform_.translate.y + part.aabbMinOffset.y,
                transform_.translate.z + part.aabbMinOffset.z };
            box.max = { transform_.translate.x + part.aabbMaxOffset.x,
                transform_.translate.y + part.aabbMaxOffset.y,
                transform_.translate.z + part.aabbMaxOffset.z };
            compound.dividBoxes.push_back(box);
        }
    }

    return compound;
}

std::vector<Vector3> FourEyesBoss::GetTargetPositions()
{
    std::vector<Vector3> positions;

    // 生きている発射位置のみロック対象
    for (const auto& part : parts_) {
        if (part.hp > 0) {
            positions.push_back({ transform_.translate.x + part.muzzleOffset.x,
                transform_.translate.y + part.muzzleOffset.y,
                transform_.translate.z + part.muzzleOffset.z });
        }
    }
    return positions;
}

void FourEyesBoss::OnCollision(Collider* other)
{
    // 当たったもの次第で分岐
    if (other->GetCollisionGroup() == CollisionGroup::kPlayerBullet) {
        // ダメージ処理
        currentHp_ -= other->GetDamage();

        partsDamage(other);

        if (currentHp_ <= 0) {
            StartDeathProduction();
            isDead_ = true; // 死亡演出トリガー用
        }
    } else if (other->GetCollisionGroup() == CollisionGroup::kPlayer) {
        // お互いダメージ処理
        currentHp_ -= 1;

        if (currentHp_ <= 0) {
            StartDeathProduction();
            isDead_ = true; // 死亡演出トリガー用
        }
    }
}

void FourEyesBoss::StartAppearance()
{
    isAppearing_ = true;
    appearanceTimer_ = 0.0f;

    transform_.translate = centerPos_;
    transform_.translate.y = centerPos_.y + kStartOffsetY;
    transform_.rotate.y = kStartRotateY;

    object3d->SetTranslate(transform_.translate);
    object3d->SetRotate(transform_.rotate);
}

void FourEyesBoss::UpdateAppearance(float deltaTime)
{
    appearanceTimer_ += deltaTime;

    float t = appearanceTimer_ / kAppearanceDuration;
    t = std::clamp(t, 0.0f, 1.0f);

    float easeT = 1 - (1 - t) * (1 - t);

    float currentOffsetY = kStartOffsetY * (1.0f - easeT);
    transform_.translate.y = centerPos_.y + currentOffsetY;

    transform_.rotate.y = kStartRotateY * (1.0f - easeT);

    Vector3 pos = CameraManager::GetInstance()->GetActiveCamera()->GetTranslate();
    pos.z = pos.z + offsetPosZ;
    transform_.translate.z = pos.z;

    UIUpdate();
    object3d->SetTranslate(transform_.translate);
    object3d->SetRotate(transform_.rotate);
    object3d->Update();

    if (appearanceTimer_ >= kAppearanceDuration) {
        isAppearing_ = false; // 演出終了、通常戦闘状態へ遷移
    }
}

void FourEyesBoss::StartDeathProduction()
{
    isDeathProdiction_ = true;
    isDeadMoveCompletion_ = false;
    deadPos_ = transform_.translate;
    deathTimer = 0.0f;
}

void FourEyesBoss::UpdateDeathProduction(float deltaTime)
{
    deathTimer += deltaTime;
    if (deathTimer >= kdeathTimer) {
        isAvile_ = false;
        return;
    }

    if (!isDeadMoveCompletion_) {
        float t = (deathTimer * 2.0f) / kdeathTimer;
        t = std::clamp(t, 0.0f, 1.0f);
        if (t >= 1.0f) {
            isDeadMoveCompletion_ = true;
        }

        transform_.translate.y = std::lerp(deadPos_.y, centerPos_.y, t);
    
    } else {
        // 死亡時間までパーティクルとsclaeいじいじ
        float scaleTimer = deathTimer - (kdeathTimer * 0.5f);

        const float kFrequency = 19.0f;
        const float kAmplitude = 0.3f;

        // 1.0 を中心に 0.7 ～ 1.3 の範囲で拡大縮小
        float scaleFactor = 1.0f + std::sin(scaleTimer * kFrequency) * kAmplitude;
        transform_.scale = { scaleFactor, scaleFactor, scaleFactor };
    }

    Vector3 pos = CameraManager::GetInstance()->GetActiveCamera()->GetTranslate();
    pos.z = pos.z + offsetPosZ;
    transform_.translate.z = pos.z;

    object3d->SetScale(transform_.scale);
    object3d->SetTranslate(transform_.translate);
    object3d->SetRotate(transform_.rotate);
    object3d->Update();
}

void FourEyesBoss::FireFourWayBullets()
{
    float currentDeltaTime = SceneManager::GetInstance()->GetDeltaTime();
    interval -= currentDeltaTime;

    if (interval <= 0.0f) {
        // 弾の生成
        for (const auto& part : parts_) {
            if (part.hp <= 0) {
                continue;
            }

            std::unique_ptr<EnemyHomingBullet> newBulletEnemy = std::make_unique<EnemyHomingBullet>();
            Vector3 pos = {
                transform_.translate.x + part.muzzleOffset.x,
                transform_.translate.y + part.muzzleOffset.y,
                transform_.translate.z + part.muzzleOffset.z
            };
            newBulletEnemy->Initialize(pos, transform_.rotate);
            newBulletEnemy->SetTargetPosition(player_->GetTranslate());

            enemyBullet_.push_back(std::move(newBulletEnemy));
        }
        interval = maxInterval;
    }

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

void FourEyesBoss::MoveUpdate()
{
    float deltaTime = SceneManager::GetInstance()->GetDeltaTime();

    moveTimer_ += deltaTime * kMoveSpeed;

    transform_.translate.x = centerPos_.x + kAmplitudeX * std::sin(moveTimer_);
    transform_.translate.y = centerPos_.y + kAmplitudeY * std::sin(moveTimer_ * 2.0f);

    Vector3 pos = CameraManager::GetInstance()->GetActiveCamera()->GetTranslate();
    pos.z = pos.z + offsetPosZ;
    transform_.translate.z = pos.z;

    object3d->SetTranslate(transform_.translate);
    object3d->SetRotate(transform_.rotate);
}

void FourEyesBoss::UIUpdate()
{
    float hpRate = static_cast<float>(currentHp_) / static_cast<float>(maxHp_);
    hpRate = std::clamp(hpRate, 0.0f, 1.0f);

    BossHpUI->SetGaugeRateTop(hpRate);

    BossMaxHpUI->Update();
    BossHpUI->Update();
    BossWarning->Update();
}

void FourEyesBoss::partsDamage(Collider* other)
{
    AllAABB otherAllAABB = other->GetAllAABB();
    int damage = other->GetDamage();

    for (auto& part : parts_) {
        if (part.hp <= 0) {
            continue;
        }

        AABB partBox;
        partBox.min = { transform_.translate.x + part.aabbMinOffset.x,
            transform_.translate.y + part.aabbMinOffset.y,
            transform_.translate.z + part.aabbMinOffset.z };
        partBox.max = { transform_.translate.x + part.aabbMaxOffset.x,
            transform_.translate.y + part.aabbMaxOffset.y,
            transform_.translate.z + part.aabbMaxOffset.z };

        for (const auto& otherBox : otherAllAABB.dividBoxes) {
            bool isHit = (partBox.min.x < otherBox.max.x && partBox.max.x > otherBox.min.x) && (partBox.min.y < otherBox.max.y && partBox.max.y > otherBox.min.y) && (partBox.min.z < otherBox.max.z && partBox.max.z > otherBox.min.z);

            if (isHit) {
                part.hp = std::max(0, part.hp - damage);
                break; // 同一フレームでの多重ヒット防止
            }
        }
    }
}
