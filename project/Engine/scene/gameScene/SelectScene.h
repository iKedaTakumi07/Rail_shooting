#pragma once
#include "../../2d/Sprite.h"
#include "../base/BaseScene.h"
#include <memory>

class SceneTransition;
class skydome;

class SelectScene : public BaseScene {
public:
    SelectScene();
    ~SelectScene();

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
    int stageNumber = 0;
    int MaxStageNumber = 2;
    int MinStageNumber = 1;

    bool selectStop = false;
    bool GameChange = false;
    float GameChangeTimer = 0.5f;

    // UI(スプライト)
    std::unique_ptr<Sprite> SatgeUI1;
    std::unique_ptr<Sprite> SatgeUI2;
    std::unique_ptr<skydome> skydome_;

    bool isTitile = false; // タイトルバック
    float titleChangeTimer = 1.0f;

    std::unique_ptr<SceneTransition> Transition_;
};
