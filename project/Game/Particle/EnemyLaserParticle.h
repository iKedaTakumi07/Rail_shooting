#pragma once
#include "../../Engine/base/Math.h"
#include <array>
#include <vector>

class EnemyLaserParticle {
public:
    // 初期化
    void Initialize();

    // 生成
    void NewParticle(const Vector3& emitterTransformA, const Vector3& emitterTransformB, const Transform& BulletTransform) const;

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
