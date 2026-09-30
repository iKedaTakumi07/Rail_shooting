#pragma once
#include "../../Engine/base/Math.h"
#include <vector>

class LaserParticle {
public:
    // 初期化
    void Initialize();

    // 生成
    void NewParticle(const Transform& emitterTransform) const;

    // 毎フレーム更新
    void Update();

    // 描画
    void Draw();

public:
    void SetStartColor(Vector4 color) { StartColor = color; }
    void SetEndColor(Vector4 color) { EndColor = color; }

private:
    // ステータス設定。
    Vector4 StartColor;
    Vector4 EndColor;
};
