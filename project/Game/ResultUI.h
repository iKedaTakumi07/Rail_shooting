#pragma once
#include "../Engine/2d/Sprite.h"
#include "../Engine/base/Math.h"
#include <memory>
#include <random>

class ResultUI {
public:
    enum class State {
        kNull = -1,
        kSceneChange, // シーンチェンジ中(スキップ不可)
        kResultUpdateNow, // リザルト発表中(アニメーション,イージング等中)(スキップ可)
        kResult, //  リザルト表記完了
        kReturnSelect, // ステージ選択移行
    };

    void Initialize();

    void Update();

    void Draw();

    void SpritDraw();

public:
    void SetNumber();
    void maxEnemies();
    void ResultUIUpdate();
    void NumberUpdate();
    void skipUpdate();

    // GeT関数
    bool GetSelectOrder() const { return isSelectOrder; }

private:
    std::mt19937 randomEngine;

    std::string PopEnemyFilePath_; // 現在読み込んでいるファイル
    int maxNumberOfEnemies; // ステージごとの敵の数
    int NumberOfDefeats; // 撃破数

    // UI(スプライト)
    std::unique_ptr<Sprite> ResultUI;
    std::unique_ptr<Sprite> Defeats;
    Vector2 ResultUIBaseSize_ { 1.0f, 1.0f };
    Vector2 DefeatsBaseSize_ { 1.0f, 1.0f };

    float SpriteDrawTimer = 1.0f; // シーンチェンジ後UI表示
    float AnimatonUI = 0.5f; // UIアニメーション時間
    const float kAnimatonUI = 0.5f; // UIアニメーション時間
    bool isUIDraw = false;
    bool isUIAnimationEnd = false;

    std::array<std::unique_ptr<Sprite>, 3> maxNumber_; // トータル
    std::array<std::unique_ptr<Sprite>, 3> DefeatsNumber_; // 撃破数
    std::array<std::unique_ptr<Sprite>, 3> animationDefeatsNumber_; // アニメーション(ドラムロールもどき)
    std::array<std::string, 10> numberList_; // マクロもどき
    std::array<int, 3> digits;

    float numberAnimationTimer = 2.0f; // 数字アニメーション時間
    const float knumberAnimationTimer = 2.0f;
    bool isNumberAnimationEnd = false; // 表示し終わりかどうか
    bool isAllAnimationEnd = false;

    std::unique_ptr<Sprite> SelectUI;

    bool isSelectOrder = false; // セレクトシーンに戻るか

    State nowChoice = State::kNull; // 選択状態
};
