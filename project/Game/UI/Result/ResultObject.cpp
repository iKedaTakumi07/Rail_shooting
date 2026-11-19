#include "ResultObject.h"
#include "../../../Engine/3d/Model.h"
#include "../../../Engine/3d/ModelManager.h"
#include "../../../Engine/3d/Object3d.h"
#include "../../../Engine/base/TextureManager.h"
#include "../../../Engine/scene/SceneManager.h"
#include <numbers>

void ResultObject::Initialize()
{
    TextureManager::getInstance()->LoadTexture("resources/player/1x1white.png");
    TextureManager::getInstance()->FlushUploads();

    ModelManager::GetInstance()->LoadModel("player/Player.obj");

    playerObject3d_ = std::make_unique<Object3d>();
    playerObject3d_->Initialize();

    playerModel_ = std::make_unique<Model>();
    playerModel_->Initialize("resources/player", "Player.obj");
    playerObject3d_->SetModel(playerModel_.get());
    playerObject3d_->SetScale(Vector3(1.0f, 1.0f, 1.0f));

    camera_ = CameraManager::GetInstance()->GetActiveCamera();

    playerPowerUnitLaser_ = std::make_unique<PlayerPowerUnitLaser>();
    playerPowerUnitLaser_->Initialize();
}

void ResultObject::Update()
{
    float delataTime = SceneManager::GetInstance()->GetDeltaTime();
    if (isEscaping_) {
        EscapeUpdate(delataTime);
    } else {
        ResultUpdate(delataTime);
    }

    playerObject3d_->SetTranslate(transform_.translate);
    playerObject3d_->SetRotate(transform_.rotate);
    playerObject3d_->Update();
}

void ResultObject::Draw()
{
    playerObject3d_->Draw();
}

void ResultObject::StartEscape()
{
    if (isEscaping_) {
        return;
    }
    isEscaping_ = true;
    isEscapeFinished_ = false;
    escapeTimer_ = 0.0f;
    escapeBasePlayerPos_ = transform_.translate;
    if (camera_) {
        escapeStartCameraPos_ = camera_->GetTranslate();
        escapeStartCameraOffset_ = escapeStartCameraPos_ - transform_.translate;
    }
}

void ResultObject::ResultUpdate(float deltaTime)
{
    // プレイヤーの更新
    Vector3 prePos = transform_.translate;

    transform_.translate.z += speed_.z * deltaTime;
    playerPowerUnitLaser_->NewParticle(prePos, transform_, kPowerUnitPos_);

    // カメラの更新
    if (camera_) {
        angle_ += angleSpeed_ * deltaTime;
        constexpr float kTwoPi = std::numbers::pi_v<float> * 2.0f;
        if (angle_ >= kTwoPi) {
            angle_ -= kTwoPi;
        }

        // プレイヤーを中心に斜め上から映す
        Vector3 cameraPos;
        cameraPos.x = transform_.translate.x + radius_ * std::cos(angle_);
        cameraPos.y = transform_.translate.y + cameraOffsetY_;
        cameraPos.z = transform_.translate.z + radius_ * std::sin(angle_);
        camera_->SetTranslate(cameraPos);

        // 回転
        Vector3 toPlayer = transform_.translate - cameraPos;
        float yaw = std::atan2(toPlayer.x, toPlayer.z);
        float hypotXZ = std::hypot(toPlayer.x, toPlayer.z);
        float pitch = std::atan2(-toPlayer.y, hypotXZ);
        camera_->SetRotate({ pitch, yaw, 0.0f });
    }
}

void ResultObject::EscapeUpdate(float deltaTime)
{
    float prevTimer = escapeTimer_;
    escapeTimer_ += deltaTime;
    Vector3 PrePos = transform_.translate;
    constexpr float kRotateDuration = 0.6f;
    constexpr float kBaseClimbAngle = 0.35f;

    // 後方にカメラを映す
    if (prevTimer < kRotateDuration) {
        float t = std::clamp(escapeTimer_ / kRotateDuration, 0.0f, 1.0f);
        float easeT = EaseInQuad(t);

        // 機体の更新
        transform_.translate.z += (speed_.z * 0.5f) * deltaTime;
        transform_.rotate.x = -kBaseClimbAngle * easeT;

        if (camera_) {
            // 機体後方へ移動
            Vector3 targetCameraPos = {
                transform_.translate.x,
                transform_.translate.y - 1.0f,
                transform_.translate.z - 15.0f
            };

            Vector3 CameraPos = Lerp(escapeStartCameraPos_, targetCameraPos, easeT);
            camera_->SetTranslate(CameraPos);

            Vector3 toPlayer = transform_.translate - CameraPos;

            float yaw = std::atan2(toPlayer.x, toPlayer.z);
            float hypotXZ = std::hypot(toPlayer.x, toPlayer.z);
            float pitch = std::atan2(-toPlayer.y, hypotXZ);
            camera_->SetRotate({ pitch, yaw, 0.0f });
        }
    } else { // 高速移動
        float boostTime = escapeTimer_ - kRotateDuration;
        float accel = 300.0f * boostTime;
        float moveSpeed = speed_.z + accel;

        // 角度を上げてY/Z急上昇離脱
        float climbRate = std::clamp(boostTime / 0.8f, 0.0f, 1.0f);
        float climbAngle = kBaseClimbAngle + 0.30f * climbRate;
        transform_.rotate.x = -climbAngle;

        transform_.translate.y += std::sin(climbAngle) * moveSpeed * deltaTime;
        transform_.translate.z += std::cos(climbAngle) * moveSpeed * deltaTime;

        // カメラは追尾停止
        if (camera_) {
            Vector3 cameraPos = camera_->GetTranslate();
            Vector3 toPlayer = transform_.translate - cameraPos;
            float yaw = std::atan2(toPlayer.x, toPlayer.z);
            float hypotXZ = std::hypot(toPlayer.x, toPlayer.z);
            float targetPitch = std::atan2(-toPlayer.y, hypotXZ);

            Vector3 currentRot = camera_->GetRotate();
            float dampedPitch = currentRot.x + (targetPitch - currentRot.x) * std::clamp(2.0f * deltaTime, 0.0f, 1.0f);
            camera_->SetRotate({ dampedPitch, yaw, 0.0f });
        }
    }

    playerPowerUnitLaser_->NewParticle(PrePos, transform_, kPowerUnitPos_);

    // 移動完了
    if (escapeTimer_ >= 2.0f) {
        isEscapeFinished_ = true;
    }
}
