#pragma once
#include "../../Engine/base/Math.h"
#include <random>
#include <vector>

class sonicBoomParticle {
public:
    // 初期化
    void Initialize();

    // 生成
    void NewParticle(const Vector3& emitterTransformA, const Vector3& emitterTransformB) const;

public:
    void SetStartColor(const Vector4 color) { StartColor = color; }
    void SetEndColor(const Vector4 color) { EndColor = color; }

private:
    // ステータス設定。
    Vector4 StartColor;
    Vector4 EndColor;

    Vector3 EndScale = { 10.0f, 10.0f, 10.0f };
};
