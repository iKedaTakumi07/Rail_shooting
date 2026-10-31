#pragma once
#include "../base/BaseScene.h"
#include <memory>

#include "../../2d/Sprite.h"
#include "../../3d/CPUParticle/ParticleEmitter.h"
#include "../../audio/Sound.h"
class Player;

class LaserParticle;
class HitParticle;

class Model;
class Object3d;
class Skybox;
class skydome;

class TestScene : public BaseScene {
public:
    TestScene();
    ~TestScene();

public:
    // 初期化
    void Initialize() override;

    // 終了
    void Finalize() override;

    // 毎フレーム更新
    void Update() override;

    // 描画
    void Draw() override;

private:
    // 3dモデル
    std::unique_ptr<Model> model;
    std::unique_ptr<Object3d> object3d;

    std::unique_ptr<Model> model_2;
    std::unique_ptr<Object3d> object3d_2;

    std::unique_ptr<Skeleton> skeleton_2;

    std::unique_ptr<Skybox> skydox;

    std::unique_ptr<ParticleEmitter> particleEmitter2;

    std::unique_ptr<ParticleEmitter> particleEmitter4;

    std::unique_ptr<Player> player_;

    std::unique_ptr<LaserParticle> laserTest;
    std::unique_ptr<HitParticle> hitParticle;
    float testTimer = 0.0f;

    size_t gpuEmitterHandle1_ = 0;
    size_t gpuEmitterHandle2_ = 0;

    Vector3 emitterPos_ = { 0.0f, 2.0f, 0.0f };
    Vector3 gravity_ = { 0.0f, -9.8f, 0.0f };
    Vector3 acceleration_ = { 0.0f, 2.0f, 0.0f };
    float airResistance_ = 0.1f;
    float emitTimer_ = 2.0f;
    bool autoEmit_ = true;
    int emitCount_ = 32;
};
