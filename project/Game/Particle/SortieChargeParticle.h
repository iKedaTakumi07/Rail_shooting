#pragma once
#include "../../Engine/base/Math.h"
#include <random>
#include <vector>

class SortieChargeParticle {
public:
    // 初期化
    void Initialize();

    // 生成
    void NewParticle(const Vector3& targetPos) ;

public:
    void SetStartColor(const Vector4 color) { StartColor = color; }
    void SetEndColor(const Vector4 color) { EndColor = color; }

private:
    // ステータス設定。
    Vector4 StartColor = { 1.0f, 0.7f, 0.3f, 0.8f }; // 高輝度オレンジ
    Vector4 EndColor = { 1.0f, 0.7f, 0.3f, 0.0f }; // フェードアウト赤

   std::mt19937 randomEngine_;
    uint32_t MaxCount = 5;
};
