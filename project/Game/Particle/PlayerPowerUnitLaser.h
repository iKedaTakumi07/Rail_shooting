#pragma once
#include "../../Engine/base/Math.h"
#include <random>
#include <vector>

class PlayerPowerUnitLaser {
public:
    // 初期化
    void Initialize();

    // 生成
    void NewParticle(const Vector3& prePos, const Transform& emitterTransform, const Vector3& localPos);

public:
    void SetStartColor(Vector4 color) { StartColor = color; }
    void SetEndColor(Vector4 color) { EndColor = color; }

private:
    // ステータス設定。
    Vector4 StartColor;
    Vector4 EndColor;

    std::mt19937 randomEngine;
};