#pragma once
#include "../../../Engine/base/Math.h"

class shockWaveParticle {
public:
    void Initialize();

    void Emit(Vector3 pos) const; // autoEmitの場合いらぬ

private:
    // パーティクル番号
    size_t testSphereEmit_ = 0;
    size_t testRingEmit_ = 0;

    Vector3 ParticlePos = { 0.0f, 0.0f, 0.0f };
    Vector3 gravity_ = { 0.0f, 0.0f, 0.0f };
    Vector3 acceleration_ = { 0.0f, 0.0f, 0.0f };
    float airResistance_ = 0.1f;
    float emitTimer_ = 1.0f;
    bool autoEmit_ = false;
    int emitCount_ = 1;
};
