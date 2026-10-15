#pragma once
#include "../../Engine/base/Math.h"
#include <random>
#include <vector>

class GalaxyParticle {
public:
    // 初期化
    void Initialize();

    // 生成
    void NewParticle();

    // 毎フレーム更新
    void Update();

    // 描画
    void Draw();

public:
    void SetStartColor(const Vector4 color) { StartColor = color; }
    void SetEndColor(const Vector4 color) { EndColor = color; }

    void SetMaxPos(const Vector3 pos) { randomPos.max = pos; };
    void SetMinPos(const Vector3 pos) { randomPos.min = pos; };

private:
    // ステータス設定。
    Vector4 StartColor;
    Vector4 EndColor;
    AABB randomPos;
    Vector3 EndScale = { 0.1f, 0.1f, 1.0f };

    std::mt19937 randomEngine;
};
