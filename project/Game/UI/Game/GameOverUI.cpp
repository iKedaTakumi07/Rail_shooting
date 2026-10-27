#include "GameOverUI.h"
#include "../../../Game/Loder/stageDataLoad.h"
#include "../Engine/base/PostProcess.h"
#include "../Engine/base/TextureManager.h"
#include "../Engine/io/Input.h"
#include "../Engine/scene/SceneManager.h"

void GameOverUI::Initialize()
{
    TextureManager::getInstance()->LoadTexture("resources/UI/Game/GameOverUI.png");
    TextureManager::getInstance()->LoadTexture("resources/UI/Game/RetirementUI.png");
    TextureManager::getInstance()->LoadTexture("resources/UI/Game/RetryUI.png");

    GameOver = std::make_unique<Sprite>();
    GameOver->Initialize("resources/UI/Game/GameOverUI.png");
    GameOver->SetPosition(Vector2(WinApp::KClientWidth / 2.0f, WinApp::KClientHeight /8.0f * 3.0f));
    GameOver->SetAnchorPoint(Vector2(0.5f, 0.5f));

    RetirementUI = std::make_unique<Sprite>();
    RetirementUI->Initialize("resources/UI/Game/RetirementUI.png");
    RetirementUI->SetPosition(Vector2(WinApp::KClientWidth / 4.0f, WinApp::KClientHeight / 8.0f * 5.0f));
    RetirementUI->SetAnchorPoint(Vector2(0.5f, 0.5f));

    RetryUI = std::make_unique<Sprite>();
    RetryUI->Initialize("resources/UI/Game/RetryUI.png");
    RetryUI->SetPosition(Vector2(WinApp::KClientWidth / 4.0f * 3.0f, WinApp::KClientHeight / 8.0f * 5.0f));
    RetryUI->SetAnchorPoint(Vector2(0.5f, 0.5f));

    phase_ = Phase::kNone;
    timer_ = 0.0f;

    baseGameOverPos = Vector2(WinApp::KClientWidth / 2.0f, WinApp::KClientHeight / 8.0f * 3.0f);
    endGameOverPos = Vector2(WinApp::KClientWidth / 2.0f, WinApp::KClientHeight / 16.0f * 3.0f);
}

void GameOverUI::Start(bool isInstantExplosion)
{
    phase_ = Phase::kLetterIn;
    timer_ = 0.0f;
    isInstantExplosion_ = isInstantExplosion;
    shouldTriggerExplosion_ = false;
    isSelected_ = false;
    currentSelect_ = SelectOption::kRetry;

    // レターボックス有効化
    PostProcess::GetInstance()->SetLetterbox(true);
    PostProcess::GetInstance()->SetLetterboxHeight(0.0f);

    GameOver->SetColor(Vector4(1.0f, 1.0f, 1.0f, 0.0f));
    RetirementUI->SetColor(Vector4(1.0f, 1.0f, 1.0f, 0.0f));
    RetryUI->SetColor(Vector4(1.0f, 1.0f, 1.0f, 0.0f));

    // 障害物激突即死の場合、レターイン開始と同時に爆破フラグを発火
    if (isInstantExplosion_) {
        shouldTriggerExplosion_ = true;
    }
}

void GameOverUI::Update(float deltaTime)
{
    if (phase_ == Phase::kNone)
        return;

    timer_ += deltaTime;

    switch (phase_) {
    case Phase::kLetterIn: {
        float progress = std::clamp(timer_ / kLetterInTime, 0.0f, 1.0f);
        float eased = EaseOutCubic(progress);

        PostProcess::GetInstance()->SetLetterbox(true);
        PostProcess::GetInstance()->SetLetterboxHeight(kTargetLetterboxHeight * eased);

        if (progress >= 1.0f) {
            timer_ = 0.0f;
            if (isInstantExplosion_) {

                phase_ = Phase::kGameOverShow;
            } else {

                phase_ = Phase::kExplosion;
                shouldTriggerExplosion_ = true;
            }
        }
        break;
    }
    case Phase::kExplosion: {

        phase_ = Phase::kGameOverShow;
        timer_ = 0.0f;
        break;
    }
    case Phase::kGameOverShow: {
        float progress = std::clamp(timer_ / kGameOverShowTime, 0.0f, 1.0f);
        float eased = EaseOutCubic(progress);

        GameOver->SetColor(Vector4(1.0f, 1.0f, 1.0f, eased));

        if (progress >= 1.0f) {
            phase_ = Phase::kLetterOut;

            timer_ = 0.0f;
        }
        break;
    }
    case Phase::kLetterOut: {
        float progress = std::clamp(timer_ / kLetterOutTime, 0.0f, 1.0f);
        float eased = EaseOutCubic(progress);

        PostProcess::GetInstance()->SetLetterboxHeight(kTargetLetterboxHeight * (1.0f - eased));

        if (progress >= 1.0f) {
            PostProcess::GetInstance()->SetLetterbox(false);
            phase_ = Phase::kRetryUIShow;
            timer_ = 0.0f;
        }
        break;
    }
    case Phase::kRetryUIShow: {
        float progress = std::clamp(timer_ / kRetryUIShowTime, 0.0f, 1.0f);
        float eased = EaseOutCubic(progress);

        RetirementUI->SetColor(Vector4(1.0f, 1.0f, 1.0f, eased));
        RetryUI->SetColor(Vector4(1.0f, 1.0f, 1.0f, eased));

        if (progress >= 1.0f) {
            phase_ = Phase::kInteractive;
        }
        break;
    }
    case Phase::kInteractive: {
        UpdateSelectionInput();
        UpdateSelectAnimation(deltaTime);
        break;
    }
    default:
        break;
    }

    UIUpdaate(deltaTime);

    GameOver->Update();
    RetirementUI->Update();
    RetryUI->Update();
}

void GameOverUI::Draw()
{
}

void GameOverUI::SpritDraw()
{
    if (phase_ == Phase::kNone)
        return;

    if (GameOver->GetColor().w > 0.0f) {
        GameOver->Draw();
    }
    if (RetirementUI->GetColor().w > 0.0f) {
        RetirementUI->Draw();
    }
    if (RetryUI->GetColor().w > 0.0f) {
        RetryUI->Draw();
    }
}

void GameOverUI::UpdateSelectionInput()
{

    auto* input = Input::GetInstance();

    bool isLeft = input->TriggerKey(DIK_A) || input->TriggerKey(DIK_LEFT);
    bool isRight = input->TriggerKey(DIK_D) || input->TriggerKey(DIK_RIGHT);

    if (input->IsControllerConnected()) {
        if (input->PushButton(XINPUT_GAMEPAD_DPAD_LEFT) || input->GetLeftStick().x < -0.5f)
            isLeft = true;
        if (input->PushButton(XINPUT_GAMEPAD_DPAD_RIGHT) || input->GetLeftStick().x > 0.5f)
            isRight = true;
    }

    if (isLeft) {
        currentSelect_ = SelectOption::kRetire;
    } else if (isRight) {
        currentSelect_ = SelectOption::kRetry;
    }

    // 決定キー（Space / Return / コントローラーAボタン）
    if (input->TriggerKey(DIK_SPACE) || input->TriggerKey(DIK_RETURN) || (input->IsControllerConnected() && input->PushButton(XINPUT_GAMEPAD_A))) {
        isSelected_ = true;
    }
}

void GameOverUI::UpdateSelectAnimation(float deltaTime)
{

    const float targetRetryScale = (currentSelect_ == SelectOption::kRetry) ? 1.25f : 1.0f;
    const float targetRetireScale = (currentSelect_ == SelectOption::kRetire) ? 1.25f : 1.0f;

    float lerpT = std::clamp(10.0f * deltaTime, 0.0f, 1.0f);
    retryScale_ += (targetRetryScale - retryScale_) * lerpT;
    retireScale_ += (targetRetireScale - retireScale_) * lerpT;

    RetryUI->SetSize({ baseSpriteSize_.x * retryScale_, baseSpriteSize_.y * retryScale_ });
    RetirementUI->SetSize({ baseSpriteSize_.x * retireScale_, baseSpriteSize_.y * retireScale_ });

    Vector4 activeColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    Vector4 inactiveColor = { 0.5f, 0.5f, 0.5f, 0.6f };

    RetryUI->SetColor(currentSelect_ == SelectOption::kRetry ? activeColor : inactiveColor);
    RetirementUI->SetColor(currentSelect_ == SelectOption::kRetire ? activeColor : inactiveColor);
}

void GameOverUI::UIUpdaate(float deltaTime)
{
    if (phase_ != Phase::kLetterOut && phase_ != Phase::kRetryUIShow && phase_ != Phase::kInteractive) {
        return;
    }

    if (phase_ == Phase::kInteractive) {
        GameOver->SetPosition(endGameOverPos);
        return;
    }

    float totalTime = kLetterOutTime + kRetryUIShowTime;
    float currentElapsedTime = (phase_ == Phase::kLetterOut) ? timer_ : (kLetterOutTime + timer_);
    float t = std::clamp(currentElapsedTime / totalTime, 0.0f, 1.0f);

    float easedT = EaseOutCubic(t);

    
    Vector2 pos;
    pos.x = baseGameOverPos.x;
    pos.y = Lerp(baseGameOverPos.y, endGameOverPos.y, easedT);

    GameOver->SetPosition(pos);
}
