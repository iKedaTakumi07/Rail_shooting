#pragma once
#include "../../../Engine/base/Math.h"
#include <cstddef>

class FourEyesBoss;

class IBossSpecialState {
public:
    virtual ~IBossSpecialState() = default;

    // 状態開始時に1度だけ呼ばれる初期化処理
    virtual void Enter(FourEyesBoss& boss) = 0;

    // 毎フレーム呼ばれる更新処理
    virtual void Update(FourEyesBoss& boss, float deltaTime) = 0;

    // 状態終了時に1度だけ呼ばれる終了処理
    virtual void Exit(FourEyesBoss& boss) { (void)boss; }
};

// 画面中央(そのうち指定した座標にするかも)
class BossReturnToCenterState : public IBossSpecialState {
public:
    void Enter(FourEyesBoss& boss) override;
    void Update(FourEyesBoss& boss, float deltaTime) override;

private:
    static constexpr float kReturnDuration = 1.0f;
    float timer_ = 0.0f;
    Vector3 startPos_ = { 0.0f, 0.0f, 0.0f };
};

// 必殺技
class BossMinigunFireState : public IBossSpecialState {
public:
    void Enter(FourEyesBoss& boss) override;
    void Update(FourEyesBoss& boss, float deltaTime) override;

private:
    static constexpr float kDuration = 3.0f;
    static constexpr float kFireInterval = 0.08f;
    float timer_ = 0.0f;
    float intervalTimer_ = 0.0f;
    size_t currentPartIndex_ = 0;
};

// 隙
class BossStunnedState : public IBossSpecialState {
public:
    void Enter(FourEyesBoss& boss) override;
    void Update(FourEyesBoss& boss, float deltaTime) override;

private:
    static constexpr float kStunDuration = 3.0f;
    float timer_ = 0.0f;
};