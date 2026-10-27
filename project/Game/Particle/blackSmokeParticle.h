#pragma once
#include "../../Engine/base/Math.h"
#include <random>
#include <vector>

class blackSmokeParticle {
public:
    // 初期化
    void Initialize();

    // 生成
    void NewParticle(const Transform& emitterTransform);

public:
    void SetStartColor(Vector4 color) { StartColor = color; }
    void SetEndColor(Vector4 color) { EndColor = color; }

private:
    // ステータス設定。
    Vector4 StartColor;
    Vector4 EndColor;
    Vector3 Velocity = { 0.0f,3.0f,0.0f};
};
