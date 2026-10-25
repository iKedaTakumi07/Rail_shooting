#pragma once
#include "../../Loder/stageDataLoad.h"
#include "../Engine/2d/Sprite.h"
#include <memory>
#include <vector>

class IntroUI {
public:
    void Initialize();

    void Update(float progress);

    void Draw();

    void SpritDraw();

public:
    void SetIntroEnd(bool num) { isIntroEnd = num; };
    void SetiskPause(bool num) { iskPause = num; }

private:
    void IntroUpdate();

private:
    // UI(スプライト)
    std::unique_ptr<Sprite> IntroStage;
    std::unique_ptr<Sprite> IntroMission;

    std::vector<std::string> textNumber;

    Vector2 gameReturnBaseSize_ { 1.0f, 1.0f };
    Vector2 resetBaseSize_ { 1.0f, 1.0f };
    Vector2 returnSelectBaseSize_ { 1.0f, 1.0f };

    bool isIntroEnd = false;
    bool isSprite = false;
    bool iskPause = false;

    float Size = 1.0f; // 可変式スケール倍率
    float MaxSize = 1.2f;
    float MinSize = 0.8f;
    float Speed = 0.5f;
};