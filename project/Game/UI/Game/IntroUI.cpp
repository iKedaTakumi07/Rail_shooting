#include "IntroUI.h"
#include "../../../Game/Loder/stageDataLoad.h"
#include "../Engine/base/PostProcess.h"
#include "../Engine/base/TextureManager.h"
#include "../Engine/io/Input.h"
#include "../Engine/scene/SceneManager.h"

void IntroUI::Initialize()
{
    TextureManager::getInstance()->LoadTexture("resources/UI/intro/IntroStage1.png");
    TextureManager::getInstance()->LoadTexture("resources/UI/intro/IntroStage2.png");
    TextureManager::getInstance()->LoadTexture("resources/UI/intro/IntroMission.png");
    TextureManager::getInstance()->LoadTexture("resources/UI/black1x1.png");

    int i = stageDataLoad::GetInstance()->GetloadNumber();

    textNumber = {
        "resources/UI/intro/IntroStage1.png",
        "resources/UI/intro/IntroStage2.png"
    };

    int rawStageNum = stageDataLoad::GetInstance()->GetloadNumber();
    size_t safeIndex = static_cast<size_t>(std::clamp(rawStageNum - 1, 0, static_cast<int>(textNumber.size()) - 1));

    IntroStage = std::make_unique<Sprite>();
    IntroStage->Initialize(textNumber[safeIndex]);
    IntroStage->SetPosition(Vector2(WinApp::KClientWidth / 2.0f, WinApp::KClientHeight / 8.0f * 1.0f));
    IntroStage->SetAnchorPoint(Vector2(0.5f, 0.5f));

    IntroMission = std::make_unique<Sprite>();
    IntroMission->Initialize("resources/UI/intro/IntroMission.png");
    IntroMission->SetPosition(Vector2(WinApp::KClientWidth / 2.0f, WinApp::KClientHeight / 8.0f * 3.0f));
    IntroMission->SetAnchorPoint(Vector2(0.5f, 0.5f));
}

void IntroUI::Update(float progress)
{
    // -20.0fまでブラーを入れる
    if (progress <= 0.2f) {
        Vector2 Center = { 0.5f, 0.5f }; // 中心位置

        float Blur = 1.0f - (progress * 5.0f);

        PostProcess::GetInstance()->SetRadialBlur(true);
        PostProcess::GetInstance()->SetRadialBlurParam(Center, Blur);
        PostProcess::GetInstance()->SetBlackout(true);
        PostProcess::GetInstance()->SetBlackout(Blur);

    } else {
        PostProcess::GetInstance()->SetRadialBlur(false);
        PostProcess::GetInstance()->SetBlackout(false);
    }

    if (progress <= 0.4f) {
        float t = progress - 0.2f;
        t = std::clamp(t, 0.0f, 0.1f);
        float Height = 0.1f - t;

        PostProcess::GetInstance()->SetLetterbox(true);
        PostProcess::GetInstance()->SetLetterboxHeight(Height);
    } else {
        isSprite = true;
        PostProcess::GetInstance()->SetLetterbox(false);
    }

    if (progress <= 0.5f) {
        float t = (progress - 0.4f) * 10.0f;

        IntroStage->SetColor(Vector4(1.0f, 1.0f, 1.0f, t));
        IntroMission->SetColor(Vector4(1.0f, 1.0f, 1.0f, t));
    }

    if (progress >= 0.95f) {
        float t = (1.0f - progress) * 20.0f;

        IntroStage->SetColor(Vector4(1.0f, 1.0f, 1.0f, t));
        IntroMission->SetColor(Vector4(1.0f, 1.0f, 1.0f, t));
    }

    IntroStage->Update();
    IntroMission->Update();
}

void IntroUI::Draw()
{
}

void IntroUI::SpritDraw()
{
    if (!isIntroEnd && isSprite && !iskPause) {
        IntroStage->Draw();
        IntroMission->Draw();
    }
}

void IntroUI::IntroUpdate()
{
}
