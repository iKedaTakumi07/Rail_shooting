#pragma once
#include "../base/BaseScene.h"
#include <memory>

#include "../../3d/Camera.h"
#include "../../audio/Sound.h"

class Model;
class Object3d;
class Skybox;
class Player;
class clearUI;
class EnemyManager;
class CollisionManager;
class CameraController;
class stageObjectManager;
class StageManager;
class SceneTransition;
class pauseUI;

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

private:
    enum class SceneState {
        knull = -1,
        kIntro,
        kPlay,
        kPause,
        kClear
    };
    SceneState sceneState_ = SceneState::kIntro;
    SceneState PreState_ = SceneState::knull;
    float clearTimer_ = 0.0f;

    std::unique_ptr<Skybox> skydox;

    // プレイヤー
    std::unique_ptr<Player> player_;
    // 敵
    std::unique_ptr<EnemyManager> enemyManager_;

    // カメラ管理
    std::unique_ptr<CameraController> cameraController_;
    std::unique_ptr<Camera> PlayerMainCamera_; // プレイヤーのやつ
    std::unique_ptr<Camera> BossCamera_; // ボス演出用

    // ステージ置物
    std::unique_ptr<stageObjectManager> stageObject_;
    std::unique_ptr<StageManager> StageManager_;
    std::unique_ptr<clearUI> ClearUI_;
    std::unique_ptr<pauseUI> PauseUI_;
    std::unique_ptr<SceneTransition> Transition_;

    // 当たり半テオ
    std::unique_ptr<CollisionManager> collisionManager_;

    bool isSceneFinished_ = false;
    bool isChange = false; // ステージ変更
    bool isReset = false; // (再トライ)リセット
    float ChangeTimer = 0.0f;

    // 音声データ
    Sound fanfare;
    Sound clearSe;
};
