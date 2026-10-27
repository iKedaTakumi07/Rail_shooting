#pragma once
#include "../../Engine/base/Math.h"
#include <random>
#include <vector>

class ExplosionParticle {
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
    Vector4 StartColor = { 1.0f, 0.5f, 0.1f, 1.0f }; 
    Vector4 EndColor = { 0.8f, 0.1f, 0.0f, 0.0f }; 

    Vector4 CoreStartColor = { 1.0f, 1.0f, 1.0f, 1.0f }; 
    Vector4 CoreEndColor = { 1.0f, 0.4f, 0.0f, 0.0f };

    std::mt19937 randomEngine;
};
