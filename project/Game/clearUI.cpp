#include "clearUI.h"
#include "../Engine/base/PostProcess.h"
#include "../Engine/base/TextureManager.h"
#include "../Engine/base/WinApp.h"

void clearUI::Initialize()
{
    TextureManager::getInstance()->LoadTexture("resources/UI/MissionCompleted1.png");
    TextureManager::getInstance()->LoadTexture("resources/UI/MissionCompleted2.png");
    TextureManager::getInstance()->LoadTexture("resources/UI/MissionCompleted3.png");
    TextureManager::getInstance()->LoadTexture("resources/UI/MissionCompleted4.png");

    MissionCompletedSprite1 = std::make_unique<Sprite>();
    MissionCompletedSprite1->Initialize("resources/UI/MissionCompleted1.png");
    MissionCompletedSprite1->SetPosition(Vector2(WinApp::KClientWidth / 2.0f, WinApp::KClientHeight / 2.0f)); // 中心位置
    MissionCompletedSprite1->SetAnchorPoint(Vector2(1.0f, 1.0f));

    MissionCompletedSprite2 = std::make_unique<Sprite>();
    MissionCompletedSprite2->Initialize("resources/UI/MissionCompleted2.png");
    MissionCompletedSprite2->SetPosition(Vector2(WinApp::KClientWidth / 2.0f, WinApp::KClientHeight / 2.0f)); // 中心位置
    MissionCompletedSprite2->SetAnchorPoint(Vector2(0.0f, 1.0f));

    MissionCompletedSprite3 = std::make_unique<Sprite>();
    MissionCompletedSprite3->Initialize("resources/UI/MissionCompleted3.png");
    MissionCompletedSprite3->SetPosition(Vector2(WinApp::KClientWidth / 2.0f, WinApp::KClientHeight / 2.0f)); // 中心位置
    MissionCompletedSprite3->SetAnchorPoint(Vector2(1.0f, 0.0f));

    MissionCompletedSprite4 = std::make_unique<Sprite>();
    MissionCompletedSprite4->Initialize("resources/UI/MissionCompleted4.png");
    MissionCompletedSprite4->SetPosition(Vector2(WinApp::KClientWidth / 2.0f, WinApp::KClientHeight / 2.0f)); // 中心位置
    MissionCompletedSprite4->SetAnchorPoint(Vector2(0.0f, 0.0f));

    clearTimer_ = 0.0f;
}

void clearUI::Update(float clearTimer)
{
    clearTimer_ = clearTimer;

    MissionCompletedSprite1->Update();
    MissionCompletedSprite2->Update();
    MissionCompletedSprite3->Update();
    MissionCompletedSprite4->Update();
}

void clearUI::Draw()
{
}

void clearUI::SpritDraw()
{
    if (clearTimer_ >= 1.0f) {
        MissionCompletedSprite1->Draw();
    }
    if (clearTimer_ >= 1.5f) {
        MissionCompletedSprite2->Draw();
    }
    if (clearTimer_ >= 2.0f) {
        MissionCompletedSprite3->Draw();
    }
    if (clearTimer_ >= 2.5f) {
        MissionCompletedSprite4->Draw();
    }
}
