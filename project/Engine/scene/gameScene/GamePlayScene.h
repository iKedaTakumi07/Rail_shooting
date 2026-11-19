#pragma once
#include "../base/BaseScene.h"
#include "../gameState/GamePlayState.h"
#include <memory>

#include "../../3d/Camera.h"
#include "../../audio/Sound.h"

#include "../../../Game/UI/Game/GameOverUI.h"
class Model;
class Object3d;
class Skybox;
class PlayerManager;
class clearUI;
class EnemyManager;
class CollisionManager;
class CameraController;
class stageObjectManager;
class StageManager;
class SceneTransition;
class pauseUI;
class skydome;
class IntroUI;
class StageDataEditor;

class GamePlayScene : public BaseScene {
public:
    GamePlayScene();
    ~GamePlayScene();

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
    void ChangeState(std::unique_ptr<IGamePlayState> nextState);
    void PushPauseState();
    void PopPauseState();
    void FinishAndChangeScene(const std::string& nextSceneName);
    void UpdateCollisions(bool includeStageObjectRoot = false);

    // 各get
    StageManager* GetStageManager() const { return StageManager_.get(); }
    PlayerManager* GetPlayerManager() const { return playerManager_.get(); }
    EnemyManager* GetEnemyManager() const { return enemyManager_.get(); }
    CameraController* GetCameraController() const { return cameraController_.get(); }
    stageObjectManager* GetStageObjectManager() const { return stageObject_.get(); }
    CollisionManager* GetCollisionManager() const { return collisionManager_.get(); }
    SceneTransition* GetTransition() const { return Transition_.get(); }
    IntroUI* GetIntroUI() const { return introUI_.get(); }
    GameOverUI* GetGameOverUI() const { return gameOverUI_.get(); }
    clearUI* GetClearUI() const { return ClearUI_.get(); }
    pauseUI* GetPauseUI() const { return PauseUI_.get(); }

private:
    std::unique_ptr<IGamePlayState> currentState_ = nullptr;
    std::unique_ptr<IGamePlayState> pausedState_ = nullptr;

    std::unique_ptr<Skybox> skydox;

    // プレイヤー
    std::unique_ptr<PlayerManager> playerManager_;
    // 敵
    std::unique_ptr<EnemyManager> enemyManager_;

    // カメラ管理
    std::unique_ptr<CameraController> cameraController_;
    std::unique_ptr<Camera> PlayerMainCamera_; // プレイヤーのやつ
    std::unique_ptr<Camera> BossCamera_; // ボス演出用

    std::unique_ptr<skydome> skydome_;
    std::unique_ptr<IntroUI> introUI_;
    std::unique_ptr<GameOverUI> gameOverUI_;

    // ステージ置物
    std::unique_ptr<stageObjectManager> stageObject_;
    std::unique_ptr<StageManager> StageManager_;
    std::unique_ptr<clearUI> ClearUI_;
    std::unique_ptr<pauseUI> PauseUI_;
    std::unique_ptr<SceneTransition> Transition_;

    // 当たり半テオ
    std::unique_ptr<CollisionManager> collisionManager_;
    std::unique_ptr<StageDataEditor> stageDataEditor_;

    bool isSceneFinished_ = false;

    // 音声データ(のち音声クラス(インスタンス化)になると思われ)
    Sound fanfare;
    Sound clearSe;
};
