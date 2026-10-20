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
    void SetAuraColor(const Vector4& start, const Vector4& end)
    {
        auraStartColor_ = start;
        auraEndColor_ = end;
    }

    void SetCoreColor(const Vector4& start, const Vector4& end)
    {
        coreStartColor_ = start;
        coreEndColor_ = end;
    }

    void SetColors(const Vector4& auraStart, const Vector4& auraEnd, const Vector4& coreStart, const Vector4& coreEnd)
    {
        auraStartColor_ = auraStart;
        auraEndColor_ = auraEnd;
        coreStartColor_ = coreStart;
        coreEndColor_ = coreEnd;
    }

    void SetAuraStartColor(const Vector4& color) { auraStartColor_ = color; }
    void SetAuraEndColor(const Vector4& color) { auraEndColor_ = color; }
    void SetCoreStartColor(const Vector4& color) { coreStartColor_ = color; }
    void SetCoreEndColor(const Vector4& color) { coreEndColor_ = color; }

private:
    Vector4 auraStartColor_ = { 0.2f, 1.0f, 0.2f, 0.35f };
    Vector4 auraEndColor_ = { 0.2f, 1.0f, 0.2f, 0.0f };
    Vector4 coreStartColor_ = { 0.8f, 1.0f, 0.8f, 0.9f };
    Vector4 coreEndColor_ = { 0.2f, 1.0f, 0.2f, 0.0f };

    Vector3 EndScale = { 0.1f, 0.1f, 1.0f };
};
