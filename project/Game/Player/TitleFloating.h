#pragma once
#include "../../../Engine/base/Math.h"
#include "PlayerBullet.h"
#include <numbers>

class TitleFloating {
public:
    void Initialize();

    void Update();

    void Draw();

public:
    // Set関数
    void SetPostiton(Vector3 pos) { transform_.translate = pos; }
    void SetStartPos(Vector3 pos) { StartPos = pos; }
    void SetEndPos(Vector3 pos) { EndPos = pos; }
    void SetSortie(bool num) { isSortie = num; }
    // Get関数
    Vector3 GetPosition() const { return transform_.translate; }

private:
    // その他
    void IntroUpdate(float deltaTime);
    void SortieUpdate(float deltaTime);
    void MoveUpdate(float deltaTime);
    void RoateUpdate(float deltaTime, float currentAccel, bool isShift);
    void BulletUpdate(float deltaTime);

private:
    enum class State {
        knull = -1, // なんもしない
        kIntro, // 　イントロ演出
        kStay, // 入力町
        kSortie, // Enterによるシーンチェンジ演出
    };

    enum class MoveState {
        knull = -1, // なんもしない
        kLeftRoll, // 左旋回
        kRightRoll, // 右旋回
        kLeftShiftRoll, // 左旋回
        kRightShiftRoll, // 右旋回
    };

    std::list<std::unique_ptr<PlayerBullet>> playerBullets_; // 弾
    State pattern_ = State::knull; // 行動パターン
    MoveState MovePattern_ = MoveState::knull; // 行動パターン
    std::mt19937 randomEngine;

    float AttackTimer = 10.0f; // オート射撃間隔
    const float kAttackTimer = 10.0f; // オート射撃間隔
    bool isAttack = false; // 発射フラグ
    bool isSortie = false; // シーンチェンジ演出

    float IntroTimer = 1.0f;
    float kIntroTimer = 1.0f;
    float SortieTimer = 1.0f;
    float kSortieTimer = 1.0f;
    float patternInterval = 2.0f; // stat切り替え時間
    const float kpatternInterval = 2.0f;

    // 移動系パラメータ
    const float kCharacterSpeed = 0.05f; // 最高速度
    const float kAcceleration = 0.005f; // 加速度
    const float shiftUpSpeed = 1.1f; // シフト(高速旋回)乗算倍率
    const float kFriction = 0.87f; // 摩擦抵抗

    float UpDownSwitchTimer = 2.0f; // 上昇下降切り替え時間
    float dirY = 1.0f;
    float MoveSwitchTimer = 2.0f;
    float dirX = 1.0f;

    // 行動パターン系
    const float kMaxRollShift = std::numbers::pi_v<float> * 0.5f; // AD+shift時90°Z軸回転
    const float kMaxRollNormal = 0.35f; // 非shift時、横移動時の回転
    const float kMaxPitchAngle = 0.45f; // 上下移動時の回転
    const float kMaxYawAngle = 0.35f; // 横移動時の回転
    const float kShiftYawFactor = 0.05f; // shift時にy回転を抑える減衰係数

    Transform transform_ = { { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } }; // モデル座標
    Vector3 localPos_ = { 0.0f, 0.0f, 0.0f };

    Vector3 velocity_ = { 0.0f, 0.0f, 0.0f }; // 移動速度
    Vector3 StartPos = { 0.0f, 0.0f, 0.0f }; // イージング開始座標
    Vector3 EndPos = { 0.0f, 0.0f, 0.0f }; // 終了座標
    Vector3 minPos = { -8.0f, -4.0f, -10.0f }; // 動ける範囲(x:進行方向 y:上下 z:左右)
    Vector3 maxPos = { 2.0f, 4.0f, 5.0f }; // 動ける範囲(x:進行方向 y:上下 z:左右)

    // 3dモデル
    std::unique_ptr<Model> playerModel;
    std::unique_ptr<Object3d> playerObject3d;
};
