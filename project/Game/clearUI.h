#pragma once
#include "../Engine/2d/Sprite.h"
#include "../Engine/base/PostProcess.h"
#include <memory>

class clearUI {
public:
    void Initialize();

    void Update(float clearTimer);

    void Draw();

    void SpritDraw();

private:
    float clearTimer_;

    // UI(スプライト)
    std::unique_ptr<Sprite> MissionCompletedSprite1;
    std::unique_ptr<Sprite> MissionCompletedSprite2;
    std::unique_ptr<Sprite> MissionCompletedSprite3;
    std::unique_ptr<Sprite> MissionCompletedSprite4;
};
