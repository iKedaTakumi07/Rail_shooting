#pragma once
#include "../../Engine/base/Math.h"

class Camera;
class Player;

class CameraController {
public:
    CameraController() = default;
    ~CameraController() = default;

    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="player">プレイヤーのポインタ</param>
    void Initialize(const Player* player);

    /// <summary>
    /// 毎フレームの更新処理
    /// </summary>
    /// <param name="RailPos">レール位置</param>
    void Update(const Vector3& RailPos);
    void UpdateIntro(const Vector3& railPos, float progress);

    // Set関数
    void SetDefaultPosition(const Vector3& pos) { defaultCameraPos_ = pos; }
    void SetDeadZone(float x, float y)
    {
        baseDeadZoneX_ = x;
        baseDeadZoneY_ = y;
    }
    void SetMaxShift(float x, float y)
    {
        maxShiftX_ = x;
        maxShiftY_ = y;
    }
    void SetFollowSpeed(float speed) { followSpeed_ = speed; }

private:
    Camera* camera_ = nullptr;
    const Player* player_ = nullptr;

    // カメラ座標設定
    Vector3 defaultCameraPos_ = { 0.0f, 0.0f, -10.0f }; // カメラの基準位置
    float defaultOffsetZ_ = -15.0f; // プレイヤーから離す範囲

    // プレイヤーが移動してもカメラが動かない範囲
    float baseDeadZoneX_ = 3.0f;
    float baseDeadZoneY_ = 3.0f;

    // 画面端に寄った際のカメラ最大移動量
    float maxShiftX_ = 3.75f;
    float maxShiftY_ = 2.5f;

    float followSpeed_ = 6.0f; // 移動補間
};
