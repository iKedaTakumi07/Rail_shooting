#pragma once
#include "../Engine/base/PostProcess.h"
#include <string>

class SceneTransition {
public:
    enum class State {
        None, // 遷移なし
        In, // フェードイン（画面が現れる）
        Out // フェードアウト（画面が溶けて消える）
    };

    void Initialize(const std::string& maskTexturePath);
    void Start(State state, float duration);
    void Update(float deltaTime);

    bool IsFinished() const { return isFinished_; }
    State GetState() const { return currentState_; }

private:
    State currentState_ = State::None;
    float duration_ = 1.0f;
    float timer_ = 0.0f;
    bool isFinished_ = false;

    PostProcess::dissolveData maskData_ { };
};
