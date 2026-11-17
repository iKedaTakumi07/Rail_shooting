#pragma once
#include "../../../Engine/3d/CameraManager.h"
#include "../../../Engine/base/Math.h"
#include "../../Particle/PlayerPowerUnitLaser.h"

class Model;
class Object3d;
class Camera;

class ResultObject {
public:
    void Initialize();

    void Update();

    void Draw();

    // 脱出
    void StartEscape();
    bool IsEscapeFinished() const { return isEscapeFinished_; }

private:
    void ResultUpdate(float deltaTime);
    void EscapeUpdate(float deltaTime);

private:
    bool isEscaping_ = false;
    bool isEscapeFinished_ = false;
    float escapeTimer_ = 0.0f;

    Transform transform_ = { { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } }; // モデル座標
    static constexpr Vector3 kPowerUnitPos_ = { 0.0f, 0.05f, -0.75f }; // 動力射出位置

    Vector3 speed_ = { 1.0f, 1.0f, 5.0f };
    float angleSpeed_ = 0.2f;
    float angle_ = 0.0f;
    float cameraOffsetY_ = 7.5f; // 高さ
    float radius_ = 40.0f; // 回転半径

    // 脱出演出用パラメータ
    Vector3 escapeStartCameraPos_ { 0.0f, 0.0f, 0.0f };
    Vector3 escapeBasePlayerPos_ { 0.0f, 0.0f, 0.0f };

    // カメラ
    Camera* camera_ = nullptr;

    // 3dモデル
    std::unique_ptr<Model> playerModel_;
    std::unique_ptr<Object3d> playerObject3d_;

    std::unique_ptr<PlayerPowerUnitLaser> playerPowerUnitLaser_;
};
