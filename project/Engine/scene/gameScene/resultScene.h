#pragma once
#include "../../../Game/SceneTransition.h"
#include "../base/BaseScene.h"
#include <memory>

class ResultObject;
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
    enum class State {
        kNull = -1,
        kSceneChange, // シーンチェンジ中(スキップ不可)
        kResultUpdateNow, // リザルト発表中(アニメーション,イージング等中)(スキップ可)
        kResultWait, //  リザルト表記完了
        kPlanetEscape, // 惑星からの脱出演出中（UI非表示・カメラ＆機体演出）
        kReturnSelect, // ステージ選択移行
    };

private:
    State state_ = State::kSceneChange;

    std::unique_ptr<ResultObject> resultObject_;
    std::unique_ptr<ResultUI> ResultUI_;
    std::unique_ptr<skydome> skydome_;
    std::unique_ptr<SceneTransition> Transition_;

    float stateTimer_ = 1.0f;

    bool isChange = false;
};
