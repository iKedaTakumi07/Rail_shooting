#define NOMINMAX

#include "FourEyesBoss.h"
#include <algorithm>

#include "../../../Engine/3d/CameraManager.h"
#include "../../../Engine/3d/Model.h"
#include "../../../Engine/3d/ModelManager.h"

#include "../../../Engine/base/TextureManager.h"
#include "../../Player/Player.h"

#include "../../../Engine/3d/Object3d.h"
#include "../../../Engine/scene/SceneManager.h"
#include "../../OnCollison/CollisionManager.h"
#include "../Bullet/EnemyHomingBullet.h"
#include "../Bullet/LaserBeamBullet.h"
#include "../Bullet/TargetBullet.h"
#include <memory>

void FourEyesBoss::Initialize(Vector3 pos)
{
    // スポーン位置を中心座標としてセット
    centerPos_ = pos;
    deadPos_ = pos;

    std::random_device seedGenerator;
    randomEngine = std::mt19937(seedGenerator());

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

    ResetPrevPosition(transform_.translate);

    hasTriggeredHalfHpSpecial_ = false;
    isSpecialAttacking_ = false;
    specialState_ = SpecialState::kNone;
    isStaggeredFiring_ = false;
}

void FourEyesBoss::Update()
{
    float deltaTime = SceneManager::GetInstance()->GetDeltaTime();
    camera_ = CameraManager::GetInstance()->GetActiveCamera();
    UpdatePrevPosition(transform_.translate);

    if (isAppearing_) {
        // 出現演出中は演出ロジックのみを更新（攻撃は行わない）
        UpdateAppearance(deltaTime);
        return;
    }
    if (isDeathProdiction_) {
        UpdateDeathProduction(deltaTime);
        return;
    }

    float hpRate = GetHpRate();
    if (!hasTriggeredHalfHpSpecial_ && hpRate <= 0.5f) {
        hasTriggeredHalfHpSpecial_ = true;
        isSpecialAttacking_ = true;
        specialState_ = SpecialState::kReturnToCenter;
        specialTimer_ = 0.0f;
        specialStartPos_ = transform_.translate;
        isStaggeredFiring_ = false; // 通常時差攻撃を中断
    }

    // 2. 特殊フェーズ処理（割り込み）
    if (isSpecialAttacking_) {
        UpdateSpecialAttack(deltaTime);
        UIUpdate();
        object3d->Update();
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

    DrawBullets();
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

AllOBB FourEyesBoss::GetAllOBB() const
{
    AllOBB compound;

    // 方向ベクトル
    Matrix4x4 rotX = MakeRotateXMatrix(transform_.rotate.x);
    Matrix4x4 rotY = MakeRotateYMatrix(transform_.rotate.y);
    Matrix4x4 rotZ = MakeRotateZMatrix(transform_.rotate.z);
    Matrix4x4 rotMat = Multiply(rotX, Multiply(rotY, rotZ));

    // 各軸
    Vector3 orientations[3] = {
        Normalize({ rotMat.m[0][0], rotMat.m[0][1], rotMat.m[0][2] }),
        Normalize({ rotMat.m[1][0], rotMat.m[1][1], rotMat.m[1][2] }),
        Normalize({ rotMat.m[2][0], rotMat.m[2][1], rotMat.m[2][2] })
    };

    // 全体のサイズ
    Vector3 wholeMin = { -7.5f, -7.5f, -3.0f };
    Vector3 wholeMax = { 7.5f, 7.5f, 1.5f };

    // スケールを考慮したローカル中心とサイズ
    Vector3 wholeLocalCenter = {
        (wholeMin.x + wholeMax.x) / 2.0f * transform_.scale.x,
        (wholeMin.y + wholeMax.y) / 2.0f * transform_.scale.y,
        (wholeMin.z + wholeMax.z) / 2.0f * transform_.scale.z
    };
    Vector3 wholeHalfSize = {
        (wholeMax.x - wholeMin.x) / 2.0f * transform_.scale.x,
        (wholeMax.y - wholeMin.y) / 2.0f * transform_.scale.y,
        (wholeMax.z - wholeMin.z) / 2.0f * transform_.scale.z
    };

    // 中心位置
    Vector3 wholeWorldCenter = {
        transform_.translate.x + (wholeLocalCenter.x * rotMat.m[0][0] + wholeLocalCenter.y * rotMat.m[1][0] + wholeLocalCenter.z * rotMat.m[2][0]),
        transform_.translate.y + (wholeLocalCenter.x * rotMat.m[0][1] + wholeLocalCenter.y * rotMat.m[1][1] + wholeLocalCenter.z * rotMat.m[2][1]),
        transform_.translate.z + (wholeLocalCenter.x * rotMat.m[0][2] + wholeLocalCenter.y * rotMat.m[1][2] + wholeLocalCenter.z * rotMat.m[2][2])
    };

    compound.wholeBox.center = wholeWorldCenter;
    compound.wholeBox.orientations[0] = orientations[0];
    compound.wholeBox.orientations[1] = orientations[1];
    compound.wholeBox.orientations[2] = orientations[2];
    compound.wholeBox.size = wholeHalfSize;

    // 部位ごとの当たり判定
    for (const auto& part : parts_) {
        if (part.hp > 0) {
            // ローカルの中心点とサイズを計算
            Vector3 localCenter = {
                (part.aabbMinOffset.x + part.aabbMaxOffset.x) / 2.0f * transform_.scale.x,
                (part.aabbMinOffset.y + part.aabbMaxOffset.y) / 2.0f * transform_.scale.y,
                (part.aabbMinOffset.z + part.aabbMaxOffset.z) / 2.0f * transform_.scale.z
            };
            Vector3 halfSize = {
                (part.aabbMaxOffset.x - part.aabbMinOffset.x) / 2.0f * transform_.scale.x,
                (part.aabbMaxOffset.y - part.aabbMinOffset.y) / 2.0f * transform_.scale.y,
                (part.aabbMaxOffset.z - part.aabbMinOffset.z) / 2.0f * transform_.scale.z
            };

            // ワールド座標の中心位置への変換
            Vector3 worldCenter = {
                transform_.translate.x + (localCenter.x * rotMat.m[0][0] + localCenter.y * rotMat.m[1][0] + localCenter.z * rotMat.m[2][0]),
                transform_.translate.y + (localCenter.x * rotMat.m[0][1] + localCenter.y * rotMat.m[1][1] + localCenter.z * rotMat.m[2][1]),
                transform_.translate.z + (localCenter.x * rotMat.m[0][2] + localCenter.y * rotMat.m[1][2] + localCenter.z * rotMat.m[2][2])
            };

            OBB box;
            box.center = worldCenter;
            box.orientations[0] = orientations[0];
            box.orientations[1] = orientations[1];
            box.orientations[2] = orientations[2];
            box.size = halfSize;

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
        // お互いダメージ処理(接触するはずがない)
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
        isAppearing_ = false; // 演出終了
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
        float t = (deathTimer + 4.0f) / kdeathTimer;
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

    // 時差発射（2発目）のタイマー監視
    if (isStaggeredFiring_) {
        staggeredTimer_ -= currentDeltaTime;
        if (staggeredTimer_ <= 0.0f) {
            for (size_t idx : pendingPartIndices_) {
                if (idx < parts_.size() && parts_[idx].hp > 0) {
                    auto bullet = std::make_unique<EnemyHomingBullet>();
                    Vector3 pos = {
                        transform_.translate.x + parts_[idx].muzzleOffset.x,
                        transform_.translate.y + parts_[idx].muzzleOffset.y,
                        transform_.translate.z + parts_[idx].muzzleOffset.z
                    };
                    bullet->Initialize(pos, transform_.rotate);
                    if (player_) {
                        bullet->SetTargetPosition(player_->GetTranslate());
                    }
                    AddBullet(std::move(bullet));
                }
            }
            isStaggeredFiring_ = false;
            pendingPartIndices_.clear();
        }
    }

    interval -= currentDeltaTime;
    if (interval <= 0.0f) {
        float hpRate = GetHpRate();

        // 選択可能なモードを判定
        // モード0: 4発同時, モード1: 時差発射(2発->0.5s->2発), モード2: HP75%以下レーザー
        int maxMode = (hpRate <= 0.75f) ? 3 : 2;
        std::uniform_int_distribution<int> dist(0, maxMode);

        int selectedMode = dist(randomEngine);

        if (selectedMode == 0) {
            // モード0: 全生存パーツから同時発射
            for (const auto& part : parts_) {
                if (part.hp <= 0)
                    continue;

                auto bullet = std::make_unique<EnemyHomingBullet>();
                Vector3 pos = {
                    transform_.translate.x + part.muzzleOffset.x,
                    transform_.translate.y + part.muzzleOffset.y,
                    transform_.translate.z + part.muzzleOffset.z
                };
                bullet->Initialize(pos, transform_.rotate);
                if (player_) {
                    bullet->SetTargetPosition(player_->GetTranslate());
                }
                AddBullet(std::move(bullet));
            }
        } else if (selectedMode == 1) {
            // モード1: 時差発射（前半2部位発射0.5s後残り発射）
            ExecuteStaggeredFire();
        } else if (selectedMode == 2) {
            // モード2: 固定レーザー発射
            ExecuteLaserFire();
        }

        interval = maxInterval;
    }

    if (player_) {
        UpdateBullets(currentDeltaTime, player_->GetTranslate());
    }
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
    AllOBB otherAllOBB = other->GetAllOBB();
    AllOBB myAllOBB = GetAllOBB();
    int damage = other->GetDamage();

    size_t obbIndex = 0;
    for (auto& part : parts_) {
        if (part.hp <= 0)
            continue;

        if (obbIndex >= myAllOBB.dividBoxes.size())
            break;
        const OBB& partOBB = myAllOBB.dividBoxes[obbIndex++];

        bool hit = false;
        for (const auto& otherBox : otherAllOBB.dividBoxes) {
            if (CollisionManager::CheckOBB(partOBB, otherBox)) {
                part.hp = std::max(0, part.hp - damage);
                hit = true;
                break;
            }
        }
        if (hit) {
            // 貫通弾でない場合は1部位に当たった時点で破断
            break;
        }
    }
}

void FourEyesBoss::UpdateSpecialAttack(float deltaTime)
{
    specialTimer_ += deltaTime;

    Vector3 camPos = camera_->GetTranslate();
    Vector3 targetCenter = { centerPos_.x, centerPos_.y, camPos.z + offsetPosZ };

    switch (specialState_) {
    case SpecialState::kReturnToCenter: {
        // 1.0秒かけて画面中央へ戻る
        constexpr float kReturnDuration = 1.0f;
        float t = std::clamp(specialTimer_ / kReturnDuration, 0.0f, 1.0f);

        transform_.translate.x = std::lerp(specialStartPos_.x, targetCenter.x, t);
        transform_.translate.y = std::lerp(specialStartPos_.y, targetCenter.y, t);
        transform_.translate.z = targetCenter.z;

        object3d->SetTranslate(transform_.translate);

        if (t >= 1.0f) {
            specialState_ = SpecialState::kMinigunFire;
            specialTimer_ = 0.0f;
            minigunIntervalTimer_ = 0.0f;
            currentMinigunPartIndex_ = 0;
        }
        break;
    }
    case SpecialState::kMinigunFire: {
        transform_.translate = targetCenter;
        object3d->SetTranslate(transform_.translate);

        minigunIntervalTimer_ -= deltaTime;
        if (minigunIntervalTimer_ <= 0.0f) {

           
            for (size_t i = 0; i < parts_.size(); ++i) {
                size_t targetIdx = (currentMinigunPartIndex_ + i) % parts_.size();

                if (parts_[targetIdx].hp > 0) {
                    const auto& part = parts_[targetIdx];

                    auto bullet = std::make_unique<TargetBullet>();
                    Vector3 pos = {
                        transform_.translate.x + part.muzzleOffset.x,
                        transform_.translate.y + part.muzzleOffset.y,
                        transform_.translate.z + part.muzzleOffset.z
                    };
                    bullet->Initialize(pos, transform_.rotate);
                    if (player_) {
                        bullet->SetTargetPosition(player_->GetTranslate());
                    }
                    AddBullet(std::move(bullet));

                    
                    currentMinigunPartIndex_ = (targetIdx + 1) % parts_.size();
                    break; // 1発発射したらループを抜ける
                }
            }

            minigunIntervalTimer_ = 0.08f; 
        }

        if (specialTimer_ >= 3.0f) {
            specialState_ = SpecialState::kStunned;
            specialTimer_ = 0.0f;
        }
        break;
    }
    case SpecialState::kStunned: {
        // 3.0秒間、完全停止（盛大な隙）
        transform_.translate = targetCenter;
        object3d->SetTranslate(transform_.translate);

        if (specialTimer_ >= 3.0f) {
            isSpecialAttacking_ = false;
            specialState_ = SpecialState::kNone;
            interval = maxInterval; // タイマーリセット
            moveTimer_ = 0.0f;
            ResetPrevPosition(transform_.translate);
        }
        break;
    }
    default:
        break;
    }

    if (player_) {
        UpdateBullets(deltaTime, player_->GetTranslate());
    }
}

void FourEyesBoss::ExecuteStaggeredFire()
{
    std::vector<size_t> activeIndices;
    for (size_t i = 0; i < parts_.size(); ++i) {
        if (parts_[i].hp > 0) {
            activeIndices.push_back(i);
        }
    }

    if (activeIndices.empty())
        return;

    // 前半（最大2個）を直ちに発射
    size_t firstCount = (std::min)(static_cast<size_t>(2), activeIndices.size());
    for (size_t i = 0; i < firstCount; ++i) {
        size_t idx = activeIndices[i];
        auto bullet = std::make_unique<EnemyHomingBullet>();
        Vector3 pos = {
            transform_.translate.x + parts_[idx].muzzleOffset.x,
            transform_.translate.y + parts_[idx].muzzleOffset.y,
            transform_.translate.z + parts_[idx].muzzleOffset.z
        };
        bullet->Initialize(pos, transform_.rotate);
        if (player_) {
            bullet->SetTargetPosition(player_->GetTranslate());
        }
        AddBullet(std::move(bullet));
    }

    // 残りのインデックスを保持して0.5秒後に発射予約
    pendingPartIndices_.clear();
    for (size_t i = firstCount; i < activeIndices.size(); ++i) {
        pendingPartIndices_.push_back(activeIndices[i]);
    }

    if (!pendingPartIndices_.empty()) {
        isStaggeredFiring_ = true;
        staggeredTimer_ = 0.5f; // 間隔0.5秒
    }
}

void FourEyesBoss::ExecuteLaserFire()
{
    std::vector<Vector3> activePositions;
    for (const auto& part : parts_) {
        if (part.hp > 0) {
            activePositions.push_back({ transform_.translate.x + part.muzzleOffset.x,
                transform_.translate.y + part.muzzleOffset.y,
                transform_.translate.z + part.muzzleOffset.z });
        }
    }

    // レーザーを張るには最低2点が必要
    if (activePositions.size() < 2) {
        // 2点未満の場合は通常の同時発射へフォールバック
        for (const auto& pos : activePositions) {
            auto bullet = std::make_unique<EnemyHomingBullet>();
            bullet->Initialize(pos, transform_.rotate);
            if (player_)
                bullet->SetTargetPosition(player_->GetTranslate());
            AddBullet(std::move(bullet));
        }
        return;
    }

    // 生存位置から2つを選択
    size_t idxA = 0;
    size_t idxB = 1;
    if (activePositions.size() > 2) {
        idxA = std::rand() % activePositions.size();
        do {
            idxB = std::rand() % activePositions.size();
        } while (idxA == idxB);
    }

    auto laser = std::make_unique<LaserBeamBullet>();
    laser->Initialize(activePositions[idxA], { 0.0f, 0.0f, 0.0f });
    laser->SetPositions(activePositions[idxA], activePositions[idxB]);
    AddBullet(std::move(laser));
}
