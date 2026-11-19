#include "FourEyesBossState.h"
#include "FourEyesBoss.h"
#include <algorithm>
#include <cmath>
#include <memory>

void BossReturnToCenterState::Enter(FourEyesBoss& boss)
{
    timer_ = 0.0f;
    startPos_ = boss.GetTranslate();
}

void BossReturnToCenterState::Update(FourEyesBoss& boss, float deltaTime)
{
    timer_ += deltaTime;
    float t = std::clamp(timer_ / kReturnDuration, 0.0f, 1.0f);

    Vector3 targetCenter = boss.GetSpecialTargetCenter();
    Vector3 nextPos = {
        std::lerp(startPos_.x, targetCenter.x, t),
        std::lerp(startPos_.y, targetCenter.y, t),
        targetCenter.z
    };
    boss.SetSpecialPosition(nextPos);

    if (t >= 1.0f) {
        boss.ChangeSpecialState(std::make_unique<BossMinigunFireState>());
    }
}

void BossMinigunFireState::Enter(FourEyesBoss& boss)
{
    (void)boss;
    timer_ = 0.0f;
    intervalTimer_ = 0.0f;
    currentPartIndex_ = 0;
}

void BossMinigunFireState::Update(FourEyesBoss& boss, float deltaTime)
{
    timer_ += deltaTime;

    boss.SetSpecialPosition(boss.GetSpecialTargetCenter());

    intervalTimer_ -= deltaTime;
    if (intervalTimer_ <= 0.0f) {
        boss.FireMinigunFromNextPart(currentPartIndex_);
        intervalTimer_ = kFireInterval;
    }

    if (timer_ >= kDuration) {
        boss.ChangeSpecialState(std::make_unique<BossStunnedState>());
    }
}

void BossStunnedState::Enter(FourEyesBoss& boss)
{
    (void)boss;
    timer_ = 0.0f;
}

void BossStunnedState::Update(FourEyesBoss& boss, float deltaTime)
{
    timer_ += deltaTime;

    boss.SetSpecialPosition(boss.GetSpecialTargetCenter());

    if (timer_ >= kStunDuration) {
        boss.OnSpecialAttackFinished();
    }
}