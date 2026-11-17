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
        if (angle_ >= std::numbers::pi * 2.0f) {
            angle_ -= 6.283185f;
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
    escapeTimer_ += deltaTime;
    Vector3 PrePos = transform_.translate;

    // 後方にカメラを映す
    if (escapeTimer_ < 0.6f) {
        float t = escapeTimer_ / 0.6f;
        float easeT = EaseInQuad(t);

        // 機体の更新
        transform_.translate.z += (speed_.z * 0.5f) * deltaTime;
        transform_.rotate.x = -0.35f * easeT;

        if (camera_) {
            // 機体後方へ移動
            Vector3 targetCameraPos = {
                transform_.translate.x,
                transform_.translate.y - 1.0f,
                transform_.translate.z - 35.0f
            };

            Vector3 CameraPos = Slerp(escapeStartCameraPos_, targetCameraPos, easeT);
            camera_->SetTranslate(CameraPos);

            Vector3 toPlayer = {
                transform_.translate.x - targetCameraPos.x,
                transform_.translate.y - targetCameraPos.y,
                transform_.translate.z - targetCameraPos.z
            };

            float yaw = std::atan2(toPlayer.x, toPlayer.z);
            float hypotXZ = std::hypot(toPlayer.x, toPlayer.z);
            float pitch = std::atan2(-toPlayer.y, hypotXZ);
            camera_->SetRotate({ pitch, -yaw, 0.0f });
        }
    } else { // 高速移動
        float boostTime = escapeTimer_ - 0.6f;
        float accel = 300.0f * boostTime;

        // 角度を上げてY/Z急上昇離脱
        transform_.translate.y += std::sin(0.35f) * (speed_.z + accel) * deltaTime;
        transform_.translate.z += std::sin(0.35f) * (speed_.z + accel) * deltaTime;

        // カメラは追尾停止
        if (camera_) {
            Vector3 cameraPos = camera_->GetTranslate();
            Vector3 toPlayer = transform_.translate - cameraPos;
            float yaw = std::atan2(toPlayer.x, toPlayer.z);
            float hypotXZ = std::hypot(toPlayer.x, toPlayer.z);
            float pitch = std::atan2(-toPlayer.y, hypotXZ);
            camera_->SetRotate({ pitch, yaw, 0.0f });
        }
    }

    playerPowerUnitLaser_->NewParticle(PrePos, transform_, kPowerUnitPos_);

    // 移動完了
    if (escapeTimer_ >= 2.0f) {
        isEscapeFinished_ = true;
    }
}
