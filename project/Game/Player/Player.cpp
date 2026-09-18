#define NOMINMAX

#include "Player.h"
#include <algorithm>
#include <cmath>
#include <numbers>

#include "../../Engine/3d/CameraManager.h"
#include "../../Engine/3d/ModelManager.h"
#include "../../Engine/3d/Object3d.h"
#include "../../Engine/base/Math.h"
#include "../../Engine/base/TextureManager.h"
#include "../../Engine/io/Input.h"
#include "../../Engine/scene/SceneManager.h"

#include "../Enemy/EnemyManager.h"
#include "../Enemy/base/baseEnemy.h"
#include "../Enemy/base/baseEnemyBullet.h"
#include "PlayerBullet.h"

#include "../../Engine/base/WinApp.h"
#include <cstdlib>
#include <utility>

void Player::Initialize()
{
    TextureManager::getInstance()->LoadTexture("resources/player/1x1white.png");
    ModelManager::GetInstance()->LoadModel("player/Player.obj");
    TextureManager::getInstance()->LoadTexture("resources/player/playerReticle.png");
    ModelManager::GetInstance()->LoadModel("player/playerReticle.obj");
    TextureManager::getInstance()->LoadTexture("resources/player/ChargeReticle.png");
    TextureManager::getInstance()->LoadTexture("resources/player/playerHpUI2.png");
    TextureManager::getInstance()->LoadTexture("resources/player/playerHpUI3.png");

    camera_ = CameraManager::GetInstance()->GetActiveCamera();

    playerObject3d = std::make_unique<Object3d>();
    playerObject3d->Initialize();

    playerModel = std::make_unique<Model>();
    playerModel->Initialize("resources/player", "Player.obj");
    playerObject3d->SetModel(playerModel.get());
    playerObject3d->SetScale(basetransform_.scale);
    // model->SetEvnTexturefilePath(skydox->GetTextureFilePath()); // 反射が必要なら

    ShortReticleObject3d = std::make_unique<Object3d>();
    ShortReticleObject3d->Initialize();

    ShortReticleModel = std::make_unique<Model>();
    ShortReticleModel->Initialize("resources/player", "playerReticle.obj");
    ShortReticleObject3d->SetModel(ShortReticleModel.get());

    LongReticleObject3d = std::make_unique<Object3d>();
    LongReticleObject3d->Initialize();

    LongReticleModel = std::make_unique<Model>();
    LongReticleModel->Initialize("resources/player", "playerReticle.obj");
    LongReticleObject3d->SetModel(LongReticleModel.get());

    ChargeReticleSprite = std::make_unique<Sprite>();
    ChargeReticleSprite->Initialize("resources/player/ChargeReticle.png");

    PlayerMaxHpUI = std::make_unique<Sprite>();
    PlayerMaxHpUI->Initialize("resources/player/playerHpUI2.png");
    PlayerMaxHpUI->SetPosition(Vector2(0.0f, 0.0f));

    PlayerHpUI = std::make_unique<Sprite>();
    PlayerHpUI->Initialize("resources/player/playerHpUI3.png");
    PlayerHpUI->SetPosition(Vector2(8.0f, 0.0f));
}

void Player::Update()
{
    float deltaTime = SceneManager::GetInstance()->GetDeltaTime();
    camera_ = CameraManager::GetInstance()->GetActiveCamera();

    // 無敵時間の処理
    if (isinvincible) {
        invincibleTime -= deltaTime;
        const float kBlinkInterval = 0.1f; // 点滅周期

        if (std::fmod(invincibleTime, kBlinkInterval * 2.0f) > kBlinkInterval) {
            playerModel->SetMaterialColor(Vector4(1.0f, 1.0f, 1.0f, 0.5f));
        } else {
            playerModel->SetMaterialColor(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
        }

        if (invincibleTime <= 0.0f) {
            isinvincible = false;
            playerModel->SetMaterialColor(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
        }
    }

    MoveUpdate();
    BulletUpdate();
    ReticleUpdate();

    playerObject3d->SetTranslate(transform_.translate);
    playerObject3d->SetRotate(transform_.rotate);

    playerObject3d->Update();
    playerObject3d->DrawImGui("Player");

    UIUpdate();
}

void Player::UpdateIntro()
{
    float deltaTime = SceneManager::GetInstance()->GetDeltaTime();
    idleTimer_ += deltaTime;

    // 入力を無視し、レール座標にそのまま追従
    basetransform_.translate = railBasePos_;

    HoverUpdate(); // 揺れ処理のみ適用
    ReticleUpdate();

    playerObject3d->SetTranslate(transform_.translate);
    playerObject3d->SetRotate(transform_.rotate);
    playerObject3d->Update();
    UIUpdate();
}

void Player::UpdateClear()
{
    float deltaTime = SceneManager::GetInstance()->GetDeltaTime();
    idleTimer_ += deltaTime;

    // 奥に進みながら上昇 (数値は要調整)
    basetransform_.translate.z += 40.0f * deltaTime;
    basetransform_.translate.y += 15.0f * deltaTime;

    HoverUpdate();

    // 前の角度から戻すため
    transform_.rotate = { 0.0f, 0.0f, 0.0f };

    playerObject3d->SetTranslate(transform_.translate);
    playerObject3d->SetRotate(transform_.rotate);
    playerObject3d->Update();
}

void Player::Draw()
{
    for (auto& bullet_ : playerBullets_) {
        bullet_->Draw();
    }

    playerObject3d->Draw();

    ShortReticleObject3d->Draw();
    LongReticleObject3d->Draw();
}

void Player::SpritDraw()
{
    PlayerMaxHpUI->Draw();
    PlayerHpUI->Draw();

    if (ChageLook_) {
        ChargeReticleSprite->Draw();
    }
}

AllAABB Player::GetAllAABB() const
{
    AABB box;
    box.min = { basetransform_.translate.x - size, basetransform_.translate.y - size, basetransform_.translate.z - size };
    box.max = { basetransform_.translate.x + size, basetransform_.translate.y + size, basetransform_.translate.z + size };

    AllAABB compound;
    compound.wholeBox = box;
    compound.dividBoxes.push_back(box); // 単一コライダーでも配列に1つ入れることで共通化
    return compound;
}

void Player::OnCollision(Collider* other)
{
    // 無敵状態はスルー
    if (isinvincible)
        return;

    // 当たったもの次第で分岐
    if (other->GetCollisionGroup() == CollisionGroup::kEnemyBullet || other->GetCollisionGroup() == CollisionGroup::kEnenmy) {
        int damege = other->GetDamage();
        hp_ -= damege;

        // 無敵時間のフラグ実行
        isinvincible = true;
        invincibleTime = KinvincibleTime;
    } else if (other->GetCollisionGroup() == CollisionGroup::kStageObject) {
        int damege = other->GetDamage();
        hp_ -= damege;

        // 無敵時間のフラグ実行
        isinvincible = true;
        invincibleTime = KinvincibleTime;
    }
}

void Player::MoveUpdate()
{
    float deltaTime = SceneManager::GetInstance()->GetDeltaTime();
    idleTimer_ += deltaTime;

    Vector3 inputDir = { 0, 0, 0 };

    // 押した方向でベクトル変更
    if (Input::getInstance()->PushKey(DIK_A)) {
        inputDir.x -= 1.0f;
    }
    if (Input::getInstance()->PushKey(DIK_D)) {
        inputDir.x += 1.0f;
    }
    if (Input::getInstance()->PushKey(DIK_W)) {
        inputDir.y += 1.0f;
    }
    if (Input::getInstance()->PushKey(DIK_S)) {
        inputDir.y -= 1.0f;
    }

    // 正規化
    float length = std::sqrt(inputDir.x * inputDir.x + inputDir.y * inputDir.y);
    if (length > 0.0f) {
        inputDir.x /= length;
        inputDir.y /= length;
    }

    // 高速旋回
    bool isShift = Input::getInstance()->PushKey(DIK_LSHIFT) || Input::getInstance()->PushKey(DIK_RSHIFT);

    float currentAccel = isShift ? kAcceleration * shiftUpSpeed : kAcceleration; // 加速度
    float currentMaxSpeed = isShift ? kCharacterSpeed * shiftUpSpeed : kCharacterSpeed; // 速度

    // 指定方向に加速
    velocity_.x += inputDir.x * currentAccel;
    velocity_.y += inputDir.y * currentAccel;

    // 摩擦による減速
    velocity_.x *= kFriction;
    velocity_.y *= kFriction;

    // 最高速の制限
    float speed = velocity_.x * velocity_.x + velocity_.y * velocity_.y;
    if (speed > currentMaxSpeed * currentMaxSpeed) {
        float currentSpeed = std::sqrt(speed);
        velocity_.x = (velocity_.x / currentSpeed) * currentMaxSpeed;
        velocity_.y = (velocity_.y / currentSpeed) * currentMaxSpeed;
    }

    // 計算結果を代入
    localPos_.x += velocity_.x;
    localPos_.y += velocity_.y;
    // オーバーしていたら戻す
    localPos_.x = std::clamp(localPos_.x, -kMoveLimitX, kMoveLimitX);
    localPos_.y = std::clamp(localPos_.y, -kMoveLimitY, kMoveLimitY);

    // レール座標を加算
    basetransform_.translate.x = railBasePos_.x + localPos_.x;
    basetransform_.translate.y = railBasePos_.y + localPos_.y;
    basetransform_.translate.z = railBasePos_.z;

    // 揺れを含まない回転角
    float kTargetRoll = (velocity_.x / currentMaxSpeed);
    float kTargetYRoll = -(velocity_.y / currentMaxSpeed);
    float kTargetZRoll = 0.0f;

    // シフトを押しているなら機体を進行方向横に傾ける(キー入力していないなら傾けない)
    if (isShift) {
        if (length != 0.0f) {
            kTargetRoll = -(velocity_.x / currentMaxSpeed);
            kTargetZRoll = -(velocity_.x / currentMaxSpeed) * shiftZRollFactor;
        }
    }

    // 補間の速度
    float lerpSpeed = 8.0f;
    float t = 1.0f - std::exp(-lerpSpeed * deltaTime);

    basetransform_.rotate.y += (kTargetRoll - basetransform_.rotate.y) * t;
    basetransform_.rotate.x += (kTargetYRoll - basetransform_.rotate.x) * t;
    basetransform_.rotate.z += (kTargetZRoll - basetransform_.rotate.z) * t;

    // 揺れの計算
    HoverUpdate();
}

void Player::HoverUpdate()
{
    // 揺れを含む座標系
    float hoverY = std::sin(idleTimer_ * kHoverSpeed) * kHoverAmount;
    transform_.translate = basetransform_.translate;
    transform_.translate.y += hoverY;

    // 揺れ込みの回転角
    float swayZ = std::sin(idleTimer_ * kSwaySpeed) * kSwayAmountZ;
    float swayX = std::cos(idleTimer_ * kSwaySpeed * 0.7f) * kSwayAmountX;

    transform_.rotate.z = basetransform_.rotate.z + swayZ;
    transform_.rotate.x = basetransform_.rotate.x + swayX;

    transform_.rotate.y = basetransform_.rotate.y;
}

void Player::ReticleUpdate()
{
    Vector3 forwardDir;
    forwardDir.x = -std::sin(basetransform_.rotate.z);
    forwardDir.y = -std::sin(basetransform_.rotate.x);
    forwardDir.z = std::cos(basetransform_.rotate.z);

    Vector3 shortPos;
    shortPos.x = basetransform_.translate.x + (forwardDir.x * kShortDistancePlayerTo3DReticle);
    shortPos.y = basetransform_.translate.y + (forwardDir.y * kShortDistancePlayerTo3DReticle);
    shortPos.z = basetransform_.translate.z + (forwardDir.z * kShortDistancePlayerTo3DReticle);

    ShortReticleObject3d->SetTranslate(shortPos);
    ShortReticleObject3d->SetRotate(basetransform_.rotate);
    ShortReticleObject3d->Update();

    Vector3 longPos;
    longPos.x = basetransform_.translate.x + (forwardDir.x * kLongDistancePlayerTo3DReticle);
    longPos.y = basetransform_.translate.y + (forwardDir.y * kLongDistancePlayerTo3DReticle);
    longPos.z = basetransform_.translate.z + (forwardDir.z * kLongDistancePlayerTo3DReticle);

    LongReticleObject3d->SetTranslate(longPos);
    LongReticleObject3d->SetRotate(basetransform_.rotate);
    LongReticleObject3d->Update();
}

void Player::BulletUpdate()
{
    float deltaTime = SceneManager::GetInstance()->GetDeltaTime();

    // べく鳥
    Vector3 forwardDir;
    forwardDir.x = -std::sin(basetransform_.rotate.z);
    forwardDir.y = -std::sin(basetransform_.rotate.x);
    forwardDir.z = std::cos(basetransform_.rotate.z);

    bool isSpacePushed = Input::getInstance()->PushKey(DIK_SPACE);
    if (isSpacePushed) {
        // チャージ
        chargeTimer_ += deltaTime;

        lockonTargetId_ = 0;

        uint32_t bestCandidateId = 0;
        int bestCandidateIndex = 0;
        float maxDot = kLockonAngleThreshold;

        // エラー回避
        if (enemyManager_ != nullptr) {
            for (const auto& enemy : enemyManager_->GetEnemyes()) {
                if (!enemy->GetIsAvile_())
                    continue; // 死んでいるやつはする―

                std::vector<Vector3> targetPositions = enemy->GetTargetPositions();
                for (int i = 0; i < targetPositions.size(); ++i) {
                    // ベクトル
                    Vector3 toEnemy = {
                        targetPositions[i].x - basetransform_.translate.x,
                        targetPositions[i].y - basetransform_.translate.y,
                        targetPositions[i].z - basetransform_.translate.z
                    };

                    // 正規化

                    float dist = std::sqrt(toEnemy.x * toEnemy.x + toEnemy.y * toEnemy.y + toEnemy.z * toEnemy.z);
                    if (dist > kLongDistancePlayerTo3DReticle || dist <= 0.0f) {
                        continue;
                    }

                    toEnemy.x /= dist;
                    toEnemy.y /= dist;
                    toEnemy.z /= dist;

                    // 内積で近い敵を探し
                    float dot = forwardDir.x * toEnemy.x + forwardDir.y * toEnemy.y + forwardDir.z * toEnemy.z;
                    if (dot > maxDot) {
                        maxDot = dot;

                        bestCandidateId = enemy->GetId();
                        bestCandidateIndex = i; // 対象のパーツ番号を保存
                    }
                }
            }
        }

        // ロックオン対象の更新
        if (bestCandidateId != 0) {
            ChageLookId_ = bestCandidateId;
            ChageLookIndex_ = bestCandidateIndex;
            lockonTargetId_ = bestCandidateId;
            lockonTargetIndex_ = bestCandidateIndex;
        } else {
            lockonTargetId_ = ChageLookId_;
            lockonTargetIndex_ = ChageLookIndex_;
        }

        BulletCharge();
    } else {
        // チャージ時間が満たしているならちゃ―初
        if (chargeTimer_ >= kChargeTime) {
            auto playerbullet = std::make_unique<PlayerBullet>();
            playerbullet->Initialize(camera_, basetransform_.translate, basetransform_.rotate);
            playerbullet->SetisChargeBullet(true); // チャージショット扱い
            // ロックオン対象がいればセット
            if (lockonTargetId_ != 0) {
                playerbullet->SetTarget(lockonTargetId_, lockonTargetIndex_, enemyManager_);
            }
            playerBullets_.push_back(std::move(playerbullet));
            coolTime = kCoolTime;
        } else if (chargeTimer_ > 0.0f && coolTime <= 0.0f) {
            // チャージ時間未達なら通常化
            auto playerbullet = std::make_unique<PlayerBullet>();
            playerbullet->Initialize(camera_, basetransform_.translate, basetransform_.rotate);
            playerBullets_.push_back(std::move(playerbullet));
            coolTime = kCoolTime;
        }

        // リセット
        chargeTimer_ = 0.0f;
        lockonTargetId_ = 0;
        ChageLookId_ = 0;
        ChageLook_ = false;
    }

    // クールタイム
    if (coolTime > 0.0f) {
        coolTime -= deltaTime;
    }

    for (auto& bullet_ : playerBullets_) {
        bullet_->Update(deltaTime);
    }

    std::erase_if(playerBullets_, [](const std::unique_ptr<PlayerBullet>& bullet) {
        return bullet->IsDead();
    });
}

void Player::BulletCharge()
{
    // ロックオンをした敵がいるか
    if (ChageLookId_ != 0 && enemyManager_ != nullptr) {
        baseEnemy* target = enemyManager_->GetEnemyById(ChageLookId_);

        // 対象が生きているなら
        if (target != nullptr && target->GetIsAvile_()) {
            std::vector<Vector3> targetPositions = target->GetTargetPositions();
            Vector3 pos = target->GetTranslate();
            if (ChageLookIndex_ >= 0 && ChageLookIndex_ < static_cast<int>(targetPositions.size())) {
                pos = targetPositions[ChageLookIndex_];
            }

            Vector2 screenPos = WorldToScreen(pos, CameraManager::GetInstance()->GetActiveCamera());
            ChargeReticleSprite->SetPosition(screenPos);
            ChageLook_ = true;
        } else {

            ChageLook_ = false;
            ChageLookId_ = 0;
            lockonTargetId_ = 0;
        }
    } else {

        ChageLook_ = false;
    }

    // イージングもどき
    if (ChageLook_) {
        float progress = chargeTimer_ / kChargeTime;

        if (progress > 1.0f)
            progress = 1.0f; // t

        // 0.15f未満なら表示しない
        if (progress > 0.1f) {
            float easeT = progress * progress * progress; // EaseInCubic

            const float kStartScale = 1.5f;
            const float kEndScale = 1.0f;
            float currentScale = kStartScale + (kEndScale - kStartScale) * easeT;

            // 回転
            const float kMaxRotateZ = static_cast<float>(std::numbers::pi) * 2.0f;
            float currentRotateZ = (1.0f - easeT) * kMaxRotateZ;

            Vector2 baseSize = ChargeReticleSprite->GetextureSize();
            ChargeReticleSprite->SetSize({ baseSize.x * currentScale, baseSize.y * currentScale });
            ChargeReticleSprite->SetRotation(currentRotateZ);

            if (progress >= 1.0f) {
                ChargeReticleSprite->SetColor(Vector4(1.0f, 0.2f, 0.2f, 1.0f)); // チャージ完了時赤点滅等
            } else {
                ChargeReticleSprite->SetColor(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
            }
        } else {
            ChageLook_ = false;
        }
    }

    ChargeReticleSprite->SetAnchorPoint(Vector2(0.5f, 0.5f));
    ChargeReticleSprite->Update();
}

void Player::UIUpdate()
{
    float hpRate = static_cast<float>(hp_) / static_cast<float>(Maxhp_);
    hpRate = std::clamp(hpRate, 0.0f, 1.0f);

    PlayerHpUI->SetGaugeRateRight(hpRate);

    PlayerMaxHpUI->Update();
    PlayerHpUI->Update();
}

Vector2 Player::WorldToScreen(const Vector3& worldPos, Camera* camera)
{
    if (!camera) {
        return Vector2(0.0f, 0.0f);
    }

    const Matrix4x4& vp = camera->GetViewProjectionMatrix();

    // ビュー変換
    float x = worldPos.x * vp.m[0][0] + worldPos.y * vp.m[1][0] + worldPos.z * vp.m[2][0] + vp.m[3][0];
    float y = worldPos.x * vp.m[0][1] + worldPos.y * vp.m[1][1] + worldPos.z * vp.m[2][1] + vp.m[3][1];
    float z = worldPos.x * vp.m[0][2] + worldPos.y * vp.m[1][2] + worldPos.z * vp.m[2][2] + vp.m[3][2];
    float w = worldPos.x * vp.m[0][3] + worldPos.y * vp.m[1][3] + worldPos.z * vp.m[2][3] + vp.m[3][3];

    // 背面にあるなら動かさない(動作しない)
    if (w <= 0.0f) {
        return Vector2(0.0f, 0.0f);
    }

    // w除算
    float ndcX = x / w;
    float ndcY = y / w;

    float screenX = (ndcX + 1.0f) * 0.5f * static_cast<float>(WinApp::KClientWidth);
    float screenY = (1.0f - ndcY) * 0.5f * static_cast<float>(WinApp::KClientHeight);

    return Vector2(screenX, screenY);
}

AllOBB Player::GetAllOBB() const
{
    OBB obb;
    obb.center = basetransform_.translate;

    Matrix4x4 rotX = MakeRotateXMatrix(basetransform_.rotate.x);
    Matrix4x4 rotY = MakeRotateYMatrix(basetransform_.rotate.y);
    Matrix4x4 rotZ = MakeRotateZMatrix(basetransform_.rotate.z);
    Matrix4x4 rotMat = Multiply(rotX, Multiply(rotY, rotZ));

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
