#pragma once
#include "../../../Game/SceneTransition.h"
#include "../base/BaseScene.h"
#include <memory>

class Player;
class skydome;
class ResultUI;

class resultScene : public BaseScene {
public:
    resultScene();
    ~resultScene();

public:
    // 初期化
    void Initialize() override;

    // 終了
    void Finalize() override;

    // 毎フレーム更新
    void Update() override;

    // 描画
    void Draw() override;

private:
    std::unique_ptr<Player> player_;
    std::unique_ptr<ResultUI> ResultUI_;
    std::unique_ptr<skydome> skydome_;

    std::unique_ptr<SceneTransition> Transition_;
    float Timer = 0.0f;
    bool isChange = false;
};
