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

class TitleScene : public BaseScene {
public:
    TitleScene();
    ~TitleScene();

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
    std::unique_ptr<skydome> skydome_;
};
