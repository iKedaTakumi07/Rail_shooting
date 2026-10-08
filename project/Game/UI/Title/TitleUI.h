#pragma once
#include "../Engine/2d/Sprite.h"
#include <memory>

class TitleUI {
public:
    // 初期化
    void Initialize();

    // 毎フレーム更新
    void Update();

    // 描画
    void Draw();
    void SpriteDraw();

public:
    bool GetIsEndGame() const { return isEndGame_; }
    bool GetIsStartGame() const { return isStartGame_; }
    bool GetIsLock() const { return isLock; }

private:
    void StartGameUpdate();
    void EndGameUpdate();

private:
    enum class State {
        kNull = -1,
        kStartGame,
        kEndGame,
    };

    // 選択したもの
    State SelectState_ = State::kNull;
    bool isLock = false;
    bool isEndGame_ = false;
    bool isStartGame_ = false;

    float prevStickY_ = 0.0f;

    // UI(スプライト)
    std::unique_ptr<Sprite> titleSelect_;
    std::unique_ptr<Sprite> titleNotSelect_;
    std::unique_ptr<Sprite> titleSelectEnd_;
    std::unique_ptr<Sprite> titleSelectStart_;

    std::unique_ptr<Sprite> TitleScene_;
    std::unique_ptr<Sprite> TitleScenestateUI_;
};
