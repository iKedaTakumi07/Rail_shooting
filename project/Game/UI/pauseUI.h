#pragma once
#include "../Engine/2d/Sprite.h"
#include <memory>

class pauseUI {
public:
    enum class State {
        kNull = -1,
        kGameReturn, // ゲームへ戻る
        kReset, //  再トライ
        kReturnSelect, // ステージ選択移行
    };

    void Initialize();

    void Update();

    void Draw();

    void SpritDraw();

public:
    void PauseUpdate();

    // GeT関数
    bool GetPause() const { return isPause; }
    bool GetResetOrder() const { return isResetOrder; }
    bool GetSelectOrder() const { return isSelectOrder; }

private:
    bool isPause = false; // ポーズ中かどうか
    bool isResetOrder = false; // リトライするか
    bool isSelectOrder = false; // セレクトシーンに戻るか

    // UI(スプライト)
    std::unique_ptr<Sprite> PauseUI1;
    std::unique_ptr<Sprite> GameReturn;
    std::unique_ptr<Sprite> Reset;
    std::unique_ptr<Sprite> ReturnSelect;

    Vector2 gameReturnBaseSize_ { 1.0f, 1.0f };
    Vector2 resetBaseSize_ { 1.0f, 1.0f };
    Vector2 returnSelectBaseSize_ { 1.0f, 1.0f };

    float Size = 1.0f; // 可変式スケール倍率
    float MaxSize = 1.2f;
    float MinSize = 0.8f;
    float Speed = 0.5f;

    State nowChoice = State::kNull; // 選択状態
};
