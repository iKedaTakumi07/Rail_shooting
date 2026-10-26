#include "stageSelectUI.h"
#include "../../../Engine/2d/Sprite.h"
#include "../../../Engine/base/PostProcess.h"
#include "../../../Engine/base/TextureManager.h"
#include "../../../Engine/base/WinApp.h"
#include "../../../Engine/io/Input.h"
#include "../../../Game/Loder/stageDataLoad.h"
#include "../../../Game/SceneTransition.h"
#include "SelectUIObject.h"
#include <algorithm>

void stageSelectUI::Initialize(SelectUIObject* selectObject)
{
    // ポインター保持
    selectUIObject_ = selectObject;

    TextureManager::getInstance()->LoadTexture("resources/UI/SelectUIStage1.png");
    TextureManager::getInstance()->LoadTexture("resources/UI/SelectUIStage2.png");
    TextureManager::getInstance()->LoadTexture("resources/UI/SelectUI1.png");

    transition_ = std::make_unique<SceneTransition>();
    transition_->Initialize("resources/noise5.png");
    transition_->Start(SceneTransition::State::In, 1.0f);

    std::array<std::string, maxStage> stageTexturePaths = {
        "resources/UI/SelectUIStage1.png",
        "resources/UI/SelectUIStage2.png"
    };

    for (int i = 0; i < maxStage; i++) {
        stageSprite[i] = std::make_unique<Sprite>();
        stageSprite[i]->Initialize(stageTexturePaths[i]);
        stageSprite[i]->SetPosition(Vector2(WinApp::KClientWidth / 2.0f, WinApp::KClientHeight / 8.0f));
        stageSprite[i]->SetAnchorPoint(Vector2(0.5f, 0.5f));
        stageSpriteBaseSize_[i] = stageSprite[i]->GetSize();
    }

    UI = std::make_unique<Sprite>();
    UI->Initialize("resources/UI/SelectUI1.png");
    UI->SetPosition(Vector2(0.0f, WinApp::KClientHeight / 8.0f * 6.0f));
    UI->SetAnchorPoint(Vector2(0.0f, 0.0f));

    stageNumber_ = 1;
    uiAnimTimer_ = 0.0f;
    state_ = State::kSelect;
}

void stageSelectUI::Update(float deltaTime)
{
    switch (state_) {
    case State::kSelect:
        SelectUpdate(deltaTime);
        break;
    case State::kSortie:
        SortieUpdate(deltaTime);
        break;
    case State::kTitleBack:
        TitleBackUpdate(deltaTime);
        break;
    }

    UIAnimationUpdate(deltaTime);
    transition_->Update(deltaTime);

    UI->Update();
    for (int i = 0; i < maxStage; i++) {
        stageSprite[i]->Update();
    }
}

void stageSelectUI::SpriteDraw()
{
    if (state_ != State::kSelect) {
        return;
    }
    int idx = std::clamp(stageNumber_ - 1, 0, maxStage - 1);
    stageSprite[idx]->Draw();
    UI->Draw();
}

void stageSelectUI::ChangeStage(int stageIndex)
{
    stageNumber_ = stageIndex;
    if (selectUIObject_) {
        selectUIObject_->ChangeStage(stageIndex);
    }
    uiAnimTimer_ = 0.0f;
}

void stageSelectUI::StartSortie(int stageIndex)
{
    stageNumber_ = stageIndex;
    if (selectUIObject_) {
        selectUIObject_->StartSortie(stageIndex);
    }
}

void stageSelectUI::SelectUpdate(float deltaTime)
{
    if (selectUIObject_ && selectUIObject_->IsMoving()) {
        return;
    }

    auto* input = Input::GetInstance();
    int prevStage = stageNumber_;

    bool isRightTriggered = false;
    bool isLeftTriggered = false;

    if (input->IsControllerConnected()) {
        Vector2 leftStick = input->GetLeftStick();

        if (leftStick.x > 0.5f && prevStickX_ <= 0.5f) {
            isRightTriggered = true;
        }
        if (leftStick.x < -0.5f && prevStickX_ >= -0.5f) {
            isLeftTriggered = true;
        }
        prevStickX_ = leftStick.x;

        if (input->TriggerButton(XINPUT_GAMEPAD_DPAD_RIGHT)) {
            isRightTriggered = true;
        }
        if (input->TriggerButton(XINPUT_GAMEPAD_DPAD_LEFT)) {
            isLeftTriggered = true;
        }

        if (input->TriggerButton(XINPUT_GAMEPAD_A)) {
            state_ = State::kSortie;
            gameChangeTimer_ = kGameChangeTimer_;
            StartSortie(stageNumber_);
            stageDataLoad::GetInstance()->SetStage(stageNumber_);
            return;
        }

        if (input->TriggerButton(XINPUT_GAMEPAD_B)) {
            state_ = State::kTitleBack;
            titleChangeTimer_ = 1.0f;
            if (selectUIObject_)
                selectUIObject_->StartTitleBack();
            transition_->Start(SceneTransition::State::Out, 1.0f);
            return;
        }
    }

    if (input->TriggerKey(DIK_D) || input->TriggerKey(DIK_RIGHTARROW)) {
        isRightTriggered = true;
    }
    if (input->TriggerKey(DIK_A) || input->TriggerKey(DIK_LEFTARROW)) {
        isLeftTriggered = true;
    }

    if (isRightTriggered && stageNumber_ < maxStage) {
        stageNumber_++;
    }
    if (isLeftTriggered && stageNumber_ > minStageNumber_) {
        stageNumber_--;
    }

    if (stageNumber_ != prevStage) {
        ChangeStage(stageNumber_);
    }

    if (input->TriggerKey(DIK_SPACE) || input->TriggerKey(DIK_RETURN)) {
        state_ = State::kSortie;
        gameChangeTimer_ = kGameChangeTimer_;
        StartSortie(stageNumber_);
        stageDataLoad::GetInstance()->SetStage(stageNumber_);
        return;
    }

    if (input->TriggerKey(DIK_BACKSPACE) || input->TriggerKey(DIK_ESCAPE)) {
        state_ = State::kTitleBack;
        titleChangeTimer_ = 1.0f;
        if (selectUIObject_)
            selectUIObject_->StartTitleBack();
        transition_->Start(SceneTransition::State::Out, 1.0f);
        return;
    }
}

void stageSelectUI::SortieUpdate(float deltaTime)
{
    // 4 : 出撃状態
    if (static_cast<int>(selectUIObject_->GetSequenceState()) == 4) {
        gameChangeTimer_ -= deltaTime;
    }

    Vector2 center = { 0.5f, 0.5f };
    float blur = 1.0f - (gameChangeTimer_ / kGameChangeTimer_);
    if (gameChangeTimer_ <= 0.0f) {
        gameChangeTimer_ = 0.0f;
        blur = 1.0f;

        if (transition_->IsFinished()) {
            isGameChangeFinished_ = true;
        }
    }

    PostProcess::GetInstance()->SetRadialBlur(true);
    PostProcess::GetInstance()->SetRadialBlurParam(center, blur);
    PostProcess::GetInstance()->SetFade(true);
    PostProcess::GetInstance()->SetBlackout(blur);
}

void stageSelectUI::TitleBackUpdate(float deltaTime)
{
    titleChangeTimer_ -= deltaTime;
    if (titleChangeTimer_ <= 0.0f) {
        isTitleBackFinished_ = true;
    }
}

void stageSelectUI::UIAnimationUpdate(float deltaTime)
{
    if (uiAnimTimer_ < kUIAnimDuration_) {
        uiAnimTimer_ += deltaTime;
        float t = std::clamp(uiAnimTimer_ / kUIAnimDuration_, 0.0f, 1.0f);
        float easedT = EaseOutCubic(t);

        int activeIdx = std::clamp(stageNumber_ - 1, 0, maxStage - 1);
        Vector2 baseSize = stageSpriteBaseSize_[activeIdx];
        stageSprite[activeIdx]->SetSize({ baseSize.x, baseSize.y * easedT });
    }
}