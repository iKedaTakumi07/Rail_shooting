#pragma once
#include "../Engine/2d/Sprite.h"
#include <memory>
#include <vector>
class GameOverUI {
public:
    // 演出フェーズの定義
    enum class Phase {
        kNone,
        kLetterIn, // 1. レターイン
        kExplosion, // 2. 爆発演出ウェイト
        kGameOverShow, // 3. ゲームオーバー表示
        kLetterOut, // 4. レターアウト
        kRetryUIShow, // 5. リトライ系列UI表示
        kInteractive // 演出完了・入力受付中
    };

    enum class SelectOption {
        kRetry,
        kRetire
    };

    void Initialize();
    void Start(bool isInstantExplosion);

    void Update(float deltaTime);

    void Draw();

    void SpritDraw();

public:
    Phase GetPhase() const { return phase_; }
    bool IsInteractive() const { return phase_ == Phase::kInteractive; }

    bool IsSelected() const { return isSelected_; }
    SelectOption GetSelectedOption() const { return currentSelect_; }

    bool ShouldTriggerExplosion() const { return shouldTriggerExplosion_; }
    void ClearExplosionTrigger() { shouldTriggerExplosion_ = false; }

private:
    void UpdateSelectionInput();
    void UpdateSelectAnimation(float deltaTime);
    void UIUpdaate(float deltaTime);

private:
    // UI(スプライト)
    std::unique_ptr<Sprite> GameOver;
    std::unique_ptr<Sprite> RetirementUI;
    std::unique_ptr<Sprite> RetryUI;

    Phase phase_ = Phase::kNone;
    float timer_ = 0.0f;
    bool isInstantExplosion_ = false;
    bool shouldTriggerExplosion_ = false;

    // UI選択パラメータ
    SelectOption currentSelect_ = SelectOption::kRetry;
    bool isSelected_ = false;

    // スケーリングアニメーション用
    float retryScale_ = 1.0f;
    float retireScale_ = 1.0f;
    Vector2 baseSpriteSize_ = { 200.0f, 100.0f };

    Vector2 baseGameOverPos = { 0.0f, 0.0f };
    Vector2 endGameOverPos = { 0.0f, 0.0f };

    // 各演出の規定時間（秒）
    static constexpr float kLetterInTime = 0.2f;
    static constexpr float kGameOverShowTime = 1.0f;
    static constexpr float kLetterOutTime = 1.0f;
    static constexpr float kRetryUIShowTime = 1.0f;

    // レターボックスの目標高さ割合 (画面全体の約12%)
    static constexpr float kTargetLetterboxHeight = 0.12f;
};
