#pragma once
#include "baseEnemy.h"

class baseBossEnemy : public baseEnemy {
public:
    virtual float GetHpRate() const = 0;
    virtual int GetCurrentPhase() const = 0;

    // 出現演出関連のインターフェース
    virtual void StartAppearance() = 0; // 出現フラグ頭の初期化
    virtual void UpdateAppearance(float deltaTime) = 0; // 出現中の更新処理
    virtual bool IsAppearing() const = 0; // 出現中かのフラグ

    // 死亡演出のインターフェース
    virtual void StartDeathProduction() = 0;
    virtual void UpdateDeathProduction(float deltaTime) = 0;

protected:
    int maxHp_ = 1000;
    int currentHp_ = 1000;
    int currentPhase_ = 1;
    bool isAppearing_ = true;
    bool isDeathProdiction_ = false;
};
