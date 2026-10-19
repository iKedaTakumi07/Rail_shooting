#pragma once
#include "../../../Engine/2d/Sprite.h"
#include "../../../Engine/base/Math.h"
#include <array>
#include <memory>

class SceneTransition;
class SelectUIObject;

class stageSelectUI {
public:
    enum class State {
        kSelect,
        kSortie,
        kTitleBack,
    };

    // 【修正】SelectObject のポインターを受け取る初期化
    void Initialize(SelectUIObject* selectUIObject);

    void Update(float deltaTime);
    void SpriteDraw();

    void ChangeStage(int stageIndex);
    void StartSortie(int stageIndex);

    bool IsGameChangeFinished() const { return isGameChangeFinished_; }
    bool IsTitleBackFinished() const { return isTitleBackFinished_; }
    State GetState() const { return state_; }

private:
    void SelectUpdate(float deltaTime);
    void SortieUpdate(float deltaTime);
    void TitleBackUpdate(float deltaTime);
    void UIAnimationUpdate(float deltaTime);

private:
    static constexpr int maxStage = 2;
    const int minStageNumber_ = 1;
    int stageNumber_ = 1;

    float prevStickX_ = 0.0f;
    State state_ = State::kSelect;

    float gameChangeTimer_ = 0.0f;
    const float kGameChangeTimer_ = 1.2f;
    float titleChangeTimer_ = 1.0f;
    bool isGameChangeFinished_ = false;
    bool isTitleBackFinished_ = false;

    // 外部から与えられる 3D オブジェクトの参照ポインター
    SelectUIObject* selectUIObject_ = nullptr;

    std::unique_ptr<SceneTransition> transition_;
    std::array<std::unique_ptr<Sprite>, maxStage> stageSprite;
    std::array<Vector2, maxStage> stageSpriteBaseSize_;
    std::unique_ptr<Sprite> UI;

    float uiAnimTimer_ = 0.0f;
    const float kUIAnimDuration_ = 0.3f;
};
