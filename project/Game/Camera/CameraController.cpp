#include "CameraController.h"
#include "../../Engine/3d/Camera.h"
#include "../../Engine/3d/CameraManager.h"
#include "../../Engine/base/PostProcess.h"
#include "../../Engine/scene/SceneManager.h"
#include "../Player/Player.h"
#include <algorithm>
#include <cmath>

void CameraController::Initialize(const Player* player)
{

    player_ = player;

    camera_ = CameraManager::GetInstance()->GetActiveCamera();
    if (camera_) {
        defaultCameraPos_ = camera_->GetTranslate();
    }
}

void CameraController::Update(const Vector3& railPos)
{
    float deltaTime_ = SceneManager::GetInstance()->GetDeltaTime();

    // カメラ又はプレイヤーがいないなら早期リターン(ボス実装するなら変更するかも)
    if (!camera_ || !player_) {
        return;
    }

    // プレイヤーの座標を入手
    Vector3 playerPos = player_->GetTranslate();
    // レール座標を取り除く
    float localPlayerX = playerPos.x - railPos.x;
    float localPlayerY = playerPos.y - railPos.y;

    float pLimitX = player_->GetLimitX();
    float pLimitY = player_->GetLimitY();

    // カメラが動くか判別
    float deadZoneX = 0.0f;
    float deadZoneY = 0.0f;

    if (localPlayerX > baseDeadZoneX_) {
        deadZoneX = localPlayerX - baseDeadZoneX_;
    } else if (localPlayerX < -baseDeadZoneX_) {
        deadZoneX = localPlayerX + baseDeadZoneX_;
    }
    if (localPlayerY > baseDeadZoneY_) {
        deadZoneY = localPlayerY - baseDeadZoneY_;
    } else if (localPlayerY < -baseDeadZoneY_) {
        deadZoneY = localPlayerY + baseDeadZoneY_;
    }

    float rangeX = deadZoneX / (pLimitX - baseDeadZoneX_);
    float rangeY = deadZoneY / (pLimitY - baseDeadZoneY_);

    rangeX = std::clamp(rangeX, -1.0f, 1.0f);
    rangeY = std::clamp(rangeY, -1.0f, 1.0f);

    // カメラの移動量を計算
    Vector3 targetCameraPos = defaultCameraPos_;
    Vector3 currentPos = camera_->GetTranslate();

    targetCameraPos.x = railPos.x + defaultCameraPos_.x + (rangeX * maxShiftX_);
    targetCameraPos.y = railPos.y + defaultCameraPos_.y + (rangeY * maxShiftY_);
    targetCameraPos.z = railPos.z + defaultOffsetZ_;

    float t = 1.0f - std::exp(-followSpeed_ * deltaTime_); // イージング

    Vector3 newPos;
    newPos.x = currentPos.x + (targetCameraPos.x - currentPos.x) * t;
    newPos.y = currentPos.y + (targetCameraPos.y - currentPos.y) * t;
    newPos.z = targetCameraPos.z; // ブースト実装まで補間抜き

    // カメラ座標の設定と行列計算の更新
    camera_->SetTranslate(newPos);
    camera_->Update();
}

void CameraController::UpdateIntro(const Vector3& railPos, float progress)
{
    if (!camera_ || !player_)
        return;

    // -20.0fまでブラーを入れる
    if (progress <= 0.2f) {
        Vector2 Center = { 0.5f, 0.5f }; // 中心位置

        float Blur = 1.0f - (progress * 5.0f);

        PostProcess::GetInstance()->SetRadialBlur(true);
        PostProcess::GetInstance()->SetRadialBlurParam(Center, Blur);
    } else {
        PostProcess::GetInstance()->SetRadialBlur(false);
    }

    // カメラの移動量を計算
    Vector3 targetCameraPos = defaultCameraPos_;
    Vector3 currentPos = camera_->GetTranslate();

    targetCameraPos.x = railPos.x + defaultCameraPos_.x;
    targetCameraPos.y = railPos.y + defaultCameraPos_.y;
    targetCameraPos.z = railPos.z + defaultOffsetZ_;

    Vector3 newPos;
    newPos.x = targetCameraPos.x;
    newPos.y = targetCameraPos.y;
    newPos.z = targetCameraPos.z;

    // カメラ座標の設定と行列計算の更新
    camera_->SetTranslate(newPos);
    camera_->Update();
}
