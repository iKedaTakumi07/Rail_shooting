#pragma once
#include "../Engine/2d/Sprite.h"
#include "../Engine/base/Math.h"
#include <memory>
#include <random>

class ResultUI {
public:
    void Initialize();

    void Update();

    void SpritDraw();

    void StartAnimation();
    void SkipAnimation();
    void SetVisible(bool visible) { isUIDraw_ = visible; }

    // GeT関数
    bool IsAnimationEnd() const { return isAllAnimationEnd_; }

private:
    void SetNumber();
    void maxEnemies();
    void ResultUIUpdate(float deltaTime);
    void NumberUpdate(float deltaTime);

private:
    std::mt19937 randomEngine;

    std::string PopEnemyFilePath_; // 現在読み込んでいるファイル
    int maxNumberOfEnemies = 0; // ステージごとの敵の数
    int NumberOfDefeats = 0; // 撃破スコア（または撃破数）
    int maxHitCount_ = 0; // 最大hit数記録

    // UI(スプライト)
    std::unique_ptr<Sprite> ResultUI_;
    std::unique_ptr<Sprite> Defeats_;
    std::unique_ptr<Sprite> SelectUI_;
    Vector2 ResultUIBaseSize_ { 1.0f, 1.0f };
    Vector2 DefeatsBaseSize_ { 1.0f, 1.0f };

    float animatonUITimer_ = 0.5f;
    static constexpr float kAnimatonUIDuration_ = 0.5f;
    bool isUIDraw_ = false;
    bool isUIAnimationEnd_ = false;

    std::array<std::unique_ptr<Sprite>, 3> maxNumber_; // トータル
    std::array<std::unique_ptr<Sprite>, 3> DefeatsNumber_; // 撃破数
    std::array<std::unique_ptr<Sprite>, 3> animationDefeatsNumber_; // アニメーション(ドラムロールもどき)
    std::array<std::unique_ptr<Sprite>, 3> maxHitNumber_; // 最大hit数表示スプライト
    std::array<std::unique_ptr<Sprite>, 3> animationMaxHitNumber_; // 最大hit数ドラムロール用
    std::array<std::string, 10> numberList_; // マクロもどき
    std::array<int, 3> digits;
    std::array<int, 3> maxHitDigits { 0, 0, 0 };

    float numberAnimationTimer_ = 2.0f; // 数字アニメーション時間
    bool isNumberAnimationEnd_ = false; // 表示し終わりかどうか
    bool isAllAnimationEnd_ = false;
};
