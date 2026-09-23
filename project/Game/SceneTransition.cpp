#include "SceneTransition.h"

void SceneTransition::Initialize(const std::string& maskTexturePath)
{
    PostProcess::GetInstance()->AddMaskTexture(maskTexturePath);
    PostProcess::GetInstance()->SetDissolveMaskTexture(maskTexturePath);

    maskData_ = { { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.3f, 0.3f, 0.3f }, 0.0f, 0.03f, { 0.0f, 0.0f } };
}

void SceneTransition::Start(State state, float duration)
{
    currentState_ = state;
    duration_ = (duration > 0.0f) ? duration : 1.0f;
    timer_ = 0.0f;
    isFinished_ = false;

    if (currentState_ != State::None) {
        PostProcess::GetInstance()->SetDissolve(true);
    }
}

void SceneTransition::Update(float deltaTime)
{
    if (currentState_ == State::None || isFinished_) {
        return;
    }

    timer_ += deltaTime;
    float progress = timer_ / duration_;

    if (progress >= 1.0f) {
        progress = 1.0f;
        isFinished_ = true;
    }

    // フェードアウト(画面消滅)の場合は 0.0 -> 1.0、フェードインの場合は 1.0 -> 0.0
    float threshold = (currentState_ == State::Out) ? progress : (1.0f - progress);
    maskData_.gthreshold = threshold;

    PostProcess::GetInstance()->SetDissolveParam(
        maskData_.thresholdcolor,
        maskData_.Edegcolor,
        maskData_.gthreshold,
        maskData_.edgeWidth);

    if (isFinished_ && currentState_ == State::In) {
        PostProcess::GetInstance()->SetDissolve(false);
    }
}