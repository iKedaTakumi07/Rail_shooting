#include "pauseUI.h"
#include "../Engine/base/PostProcess.h"
#include "../Engine/base/TextureManager.h"
#include "../Engine/io/Input.h"
#include "../Engine/scene/SceneManager.h"

void pauseUI::Initialize()
{
    TextureManager::getInstance()->LoadTexture("resources/UI/PauseUI1.png");
    TextureManager::getInstance()->LoadTexture("resources/UI/PauseUI2.png");
    TextureManager::getInstance()->LoadTexture("resources/UI/PauseUI3.png");
    TextureManager::getInstance()->LoadTexture("resources/UI/PauseUI4.png");

    PauseUI1 = std::make_unique<Sprite>();
    PauseUI1->Initialize("resources/UI/PauseUI1.png");
    PauseUI1->SetPosition(Vector2(WinApp::KClientWidth / 2.0f, WinApp::KClientHeight / 8.0f * 1.0f));
    PauseUI1->SetAnchorPoint(Vector2(0.5f, 0.5f));

    GameReturn = std::make_unique<Sprite>();
    GameReturn->Initialize("resources/UI/PauseUI2.png");
    GameReturn->SetPosition(Vector2(WinApp::KClientWidth / 2.0f, WinApp::KClientHeight / 8.0f * 3.0f));
    GameReturn->SetAnchorPoint(Vector2(0.5f, 0.5f));

    Reset = std::make_unique<Sprite>();
    Reset->Initialize("resources/UI/PauseUI3.png");
    Reset->SetPosition(Vector2(WinApp::KClientWidth / 2.0f, WinApp::KClientHeight / 8.0f * 5.0f));
    Reset->SetAnchorPoint(Vector2(0.5f, 0.5f));

    ReturnSelect = std::make_unique<Sprite>();
    ReturnSelect->Initialize("resources/UI/PauseUI4.png");
    ReturnSelect->SetPosition(Vector2(WinApp::KClientWidth / 2.0f, WinApp::KClientHeight / 8.0f * 7.0f));
    ReturnSelect->SetAnchorPoint(Vector2(0.5f, 0.5f));

    gameReturnBaseSize_ = GameReturn->GetSize();
    resetBaseSize_ = Reset->GetSize();
    returnSelectBaseSize_ = ReturnSelect->GetSize();

    Size = 1.0f;
    MaxSize = 1.2f;
    MinSize = 0.8f;
    Speed = 0.5f;
}

void pauseUI::Update()
{
    auto* input = Input::GetInstance();

    if (input->TriggerKey(DIK_ESCAPE) || input->TriggerButton(XINPUT_GAMEPAD_START)) {
        // ポーズ画面ON,OFF
        isPause = !isPause;
        nowChoice = isPause ? State::kGameReturn : State::kNull;
    }

    // ポーズなら
    if (isPause) {
        // ブラーを強制解除
        PostProcess::GetInstance()->SetRadialBlur(false);

        PauseUpdate();
    }

    PauseUI1->Update();
    GameReturn->Update();
    Reset->Update();
    ReturnSelect->Update();
}

void pauseUI::Draw()
{
}

void pauseUI::SpritDraw()
{
    if (isPause) {
        PauseUI1->Draw();
        GameReturn->Draw();
        Reset->Draw();
        ReturnSelect->Draw();
    }
}

void pauseUI::PauseUpdate()
{
    auto* input = Input::GetInstance();
    float deltaTime = SceneManager::GetInstance()->GetDeltaTime();

    Size += Speed * deltaTime;

    if (Size >= MaxSize || Size < MinSize) {
        Speed = Speed * -1.0f;
    }

    switch (nowChoice) {
    case pauseUI::State::kGameReturn:

        GameReturn->SetSize(Vector2(gameReturnBaseSize_.x * Size, gameReturnBaseSize_.y * Size));
        Reset->SetSize(resetBaseSize_);
        ReturnSelect->SetSize(returnSelectBaseSize_);

        // ポーズ解除又は移動キー
        if (input->IsControllerConnected()) {
            Vector2 leftStick = input->GetLeftStick();

            bool isStickDown = (leftStick.y < -0.5f) && (prevStickY_ >= -0.5f);
            prevStickY_ = leftStick.y;
            if (isStickDown) {
                nowChoice = State::kReset;
            }
        }

        if (input->TriggerKey(DIK_S) || input->TriggerButton(XINPUT_GAMEPAD_DPAD_DOWN)) {
            nowChoice = State::kReset;
        } else if (input->TriggerKey(DIK_RETURN) || input->TriggerButton(XINPUT_GAMEPAD_A)) {
            isPause = false;
            nowChoice = State::kNull;
        }
        break;
    case pauseUI::State::kReset:
        GameReturn->SetSize(gameReturnBaseSize_);
        Reset->SetSize(Vector2(resetBaseSize_.x * Size, resetBaseSize_.y * Size));
        ReturnSelect->SetSize(returnSelectBaseSize_);

        // 再トライ
        if (input->IsControllerConnected()) {
            Vector2 leftStick = input->GetLeftStick();

            bool isStickDown = (leftStick.y < -0.5f) && (prevStickY_ >= -0.5f);
            bool isStickUp = (leftStick.y > 0.5f) && (prevStickY_ <= 0.5f);

            prevStickY_ = leftStick.y;
            if (isStickDown) {
                nowChoice = State::kReturnSelect;
            } else if (isStickUp) {
                nowChoice = State::kGameReturn;
            }
        }

        if (input->TriggerKey(DIK_W) || input->TriggerButton(XINPUT_GAMEPAD_DPAD_UP)) {
            nowChoice = State::kGameReturn;
        } else if (input->TriggerKey(DIK_RETURN) || input->TriggerButton(XINPUT_GAMEPAD_A)) {
            isResetOrder = true;
        } else if (input->TriggerKey(DIK_S) || input->TriggerButton(XINPUT_GAMEPAD_DPAD_DOWN)) {
            nowChoice = State::kReturnSelect;
        }
        break;
    case pauseUI::State::kReturnSelect:
        GameReturn->SetSize(gameReturnBaseSize_);
        Reset->SetSize(resetBaseSize_);
        ReturnSelect->SetSize(Vector2(returnSelectBaseSize_.x * Size, returnSelectBaseSize_.y * Size));

        // ステージ選択
        if (input->IsControllerConnected()) {
            Vector2 leftStick = input->GetLeftStick();

            bool isStickUp = (leftStick.y > 0.5f) && (prevStickY_ <= 0.5f);

            prevStickY_ = leftStick.y;
            if (isStickUp) {
                nowChoice = State::kReset;
            }
        }

        if (input->TriggerKey(DIK_W) || input->TriggerButton(XINPUT_GAMEPAD_DPAD_UP)) {
            nowChoice = State::kReset;
        } else if (input->TriggerKey(DIK_RETURN) || input->TriggerButton(XINPUT_GAMEPAD_A)) {
            isSelectOrder = true;
        }
        break;
    }
}
