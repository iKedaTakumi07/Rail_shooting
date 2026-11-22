#pragma once
#include "../../../Game/SceneTransition.h"
#include "../base/BaseScene.h"
#include "../gameState/ResultSceneState.h"
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

public:
    void ChangeState(std::unique_ptr<IResultSceneState> nextState);
    void FinishAndChangeScene(const std::string& nextSceneName);

    ResultObject* GetResultObject() const { return resultObject_.get(); }
    ResultUI* GetResultUI() const { return ResultUI_.get(); }
    SceneTransition* GetTransition() const { return Transition_.get(); }

private:
    std::unique_ptr<IResultSceneState> currentState_ = nullptr;

    std::unique_ptr<ResultObject> resultObject_;
    std::unique_ptr<ResultUI> ResultUI_;
    std::unique_ptr<skydome> skydome_;
    std::unique_ptr<SceneTransition> Transition_;

    

    bool isSceneFinished_ = false;

    bool isChange = false;
};
