#include "TitleUI.h"
#include "../Engine/base/PostProcess.h"
#include "../Engine/base/TextureManager.h"
#include "../Engine/io/Input.h"
#include "../Engine/scene/SceneManager.h"

void TitleUI::Initialize()
{
    TextureManager::getInstance()->LoadTexture("resources/UI/Title/titleSelect.png");
    TextureManager::getInstance()->LoadTexture("resources/UI/Title/titleNotSelect.png");
    TextureManager::getInstance()->LoadTexture("resources/UI/Title/titleSelectEnd.png");
    TextureManager::getInstance()->LoadTexture("resources/UI/Title/titleSelectStart.png");
    TextureManager::getInstance()->LoadTexture("resources/UI/Title/Title.png");

    TitleScene_ = std::make_unique<Sprite>();
    TitleScene_->Initialize("resources/UI/Title/Title.png");
    TitleScene_->SetPosition(Vector2(WinApp::KClientWidth / 2.0f, WinApp::KClientHeight / 8.0f * 1.0f));
    TitleScene_->SetAnchorPoint(Vector2(0.5f, 0.5f));

    titleSelectStart_ = std::make_unique<Sprite>();
    titleSelectStart_->Initialize("resources/UI/Title/titleSelectStart.png");
    titleSelectStart_->SetPosition(Vector2(WinApp::KClientWidth / 16.0f * 12.0f, WinApp::KClientHeight / 8.0f * 6.0f));
    titleSelectStart_->SetAnchorPoint(Vector2(0.0f, 0.0f));

    titleSelectEnd_ = std::make_unique<Sprite>();
    titleSelectEnd_->Initialize("resources/UI/Title/titleSelectEnd.png");
    titleSelectEnd_->SetPosition(Vector2(WinApp::KClientWidth / 16.0f * 12.0f, WinApp::KClientHeight / 8.0f * 7.0f));
    titleSelectEnd_->SetAnchorPoint(Vector2(0.0f, 0.0f));

    titleSelect_ = std::make_unique<Sprite>();
    titleSelect_->Initialize("resources/UI/Title/titleSelect.png");
    titleSelect_->SetPosition(Vector2(WinApp::KClientWidth / 16.0f * 12.0f, WinApp::KClientHeight / 8.0f * 6.0f));
    titleSelect_->SetAnchorPoint(Vector2(0.0f, 0.0f));

    titleNotSelect_ = std::make_unique<Sprite>();
    titleNotSelect_->Initialize("resources/UI/Title/titleNotSelect.png");
    titleNotSelect_->SetPosition(Vector2(WinApp::KClientWidth / 16.0f * 12.0f, WinApp::KClientHeight / 8.0f * 7.0f));
    titleNotSelect_->SetAnchorPoint(Vector2(0.0f, 0.0f));

    SelectState_ = State::kStartGame;
}

void TitleUI::Update()
{
    switch (SelectState_) {
    case TitleUI::State::kStartGame:
        StartGameUpdate();
        break;
    case TitleUI::State::kEndGame:
        EndGameUpdate();
        break;
    }

    TitleScene_->Update();
    titleSelectStart_->Update();
    titleSelectEnd_->Update();
    titleSelect_->Update();
    titleNotSelect_->Update();
}

void TitleUI::Draw()
{
}

void TitleUI::SpriteDraw()
{
    TitleScene_->Draw();

    titleSelect_->Draw();
    titleNotSelect_->Draw();

    titleSelectEnd_->Draw();
    titleSelectStart_->Draw();
}

void TitleUI::StartGameUpdate()
{
    if (isLock) {
        return;
    }

    auto* input = Input::GetInstance();
    // コントローラー
    if (input->IsControllerConnected()) {
        Vector2 leftStick = input->GetLeftStick();

        bool isStickDown = (leftStick.y < -0.5f) && (prevStickY_ >= -0.5f);
        prevStickY_ = leftStick.y;

        if (input->TriggerButton(XINPUT_GAMEPAD_A)) {
            isLock = true;
            isStartGame_ = true;
        }
        if (input->TriggerButton(XINPUT_GAMEPAD_DPAD_DOWN) || isStickDown) {
            SelectState_ = State::kEndGame;
        }
    }

    //// キーボード入力
    if (input->TriggerKey(DIK_RETURN)) {
        isLock = true;
        isStartGame_ = true;
    }
    if (input->TriggerKey(DIK_DOWNARROW) || input->TriggerKey(DIK_S)) {
        SelectState_ = State::kEndGame;
    }

    titleSelect_->SetPosition(Vector2(WinApp::KClientWidth / 16.0f * 12.0f, WinApp::KClientHeight / 8.0f * 6.0f));
    titleNotSelect_->SetPosition(Vector2(WinApp::KClientWidth / 16.0f * 12.0f, WinApp::KClientHeight / 8.0f * 7.0f));
}

void TitleUI::EndGameUpdate()
{
    if (isLock) {
        return;
    }

    auto* input = Input::GetInstance();
    // コントローラー
    if (input->IsControllerConnected()) {
        Vector2 leftStick = input->GetLeftStick();

        bool isStickUp = (leftStick.y > 0.5f) && (prevStickY_ <= 0.5f);
        prevStickY_ = leftStick.y;

        if (input->TriggerButton(XINPUT_GAMEPAD_A)) {
            isLock = true;
            isEndGame_ = true;
        }

        if (input->TriggerButton(XINPUT_GAMEPAD_DPAD_UP) || isStickUp) {
            SelectState_ = State::kStartGame;
        }
    }

    //// キーボード入力
    if (input->TriggerKey(DIK_RETURN)) {
        isLock = true;
        isEndGame_ = true;
    }
    if (input->TriggerKey(DIK_UPARROW) || input->TriggerKey(DIK_W)) {
        SelectState_ = State::kStartGame;
    }

    titleSelect_->SetPosition(Vector2(WinApp::KClientWidth / 16.0f * 12.0f, WinApp::KClientHeight / 8.0f * 7.0f));
    titleNotSelect_->SetPosition(Vector2(WinApp::KClientWidth / 16.0f * 12.0f, WinApp::KClientHeight / 8.0f * 6.0f));
}
