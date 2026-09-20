#include "GamePlayScene.h"
#include "../SceneManager.h"

#include "../../base/WinApp.h"

#include "../../2d/SpriteCommon.h"
#include "../../base/TextureManager.h"

#include "../../3d/Model.h"
#include "../../3d/ModelManager.h"
#include "../../3d/Object3d.h"
#include "../../3d/Object3dCommon.h"

#include "../../3d/CPUParticle/CPUParticleManager.h"
#include "../../3d/CPUParticle/ParticleEmitter.h"

#include "../../3d/Skybox/SkyBoxCommon.h"
#include "../../3d/Skybox/Skybox.h"

#include "../../io/Input.h"

#include "../../3d/CameraManager.h"

#include "../../../Game/Camera/CameraController.h"
#include "../../../Game/Enemy/EnemyManager.h"
#include "../../../Game/Enemy/base/baseEnemy.h"
#include "../../../Game/OnCollison/CollisionManager.h"
#include "../../../Game/Player/Player.h"
#include "../../../Game/clearUI.h"
#include "../../../Game/stage/StageManager.h"
#include "../../../Game/stage/stageDataLoad.h"
#include "../../../Game/stage/stageObjectManager.h"

#include "math.h"

void GamePlayScene::Finalize()
{
    fanfare.Unload();
    clearSe.Unload();
}

GamePlayScene::GamePlayScene()
{
}

GamePlayScene::~GamePlayScene() = default;

void GamePlayScene::Initialize()
{
    CameraManager::GetInstance()->Clear();

    Camera* mainCamera = CameraManager::GetInstance()->CreateCamera("PlayMain");
    mainCamera->SetTranslate({ 0.0f, 0.0f, -15.0f });
    mainCamera->SetRotate({ 0.0f, 0.0f, 0.0f });

    Camera* bossCamera = CameraManager::GetInstance()->CreateCamera("PlayBoss");
    bossCamera->SetTranslate({ 0.0f, 10.0f, -40.0f });

    TextureManager::getInstance()->LoadTexture("resources/uvChecker.png");
    TextureManager::getInstance()->LoadTexture("resources/monsterBall.png");
    TextureManager::getInstance()->LoadTexture("resources/rostock_laage_airport_4k.dds");

    skydox = std::make_unique<Skybox>();
    skydox->Initialize("resources/rostock_laage_airport_4k.dds");

    ModelManager::GetInstance()->LoadModel("Plane.obj");
    ModelManager::GetInstance()->LoadModel("axis.obj");

    fanfare.SoundLoadFile("resources/fanfare.wav");
    clearSe.SoundLoadFile("resources/stage.mp3");

    StageManager_ = std::make_unique<StageManager>();
    StageManager_->Initialize(stageDataLoad::GetInstance()->GetStageData());

    player_ = std::make_unique<Player>();
    player_->Initialize();

    enemyManager_ = std::make_unique<EnemyManager>();
    enemyManager_->Initialize(player_.get(), stageDataLoad::GetInstance()->GetEnemyPopData()); // 読み込むファイルを決めるクラス(インスタンス化)で作成する
    player_->SetEnemyManager(enemyManager_.get());

    collisionManager_ = std::make_unique<CollisionManager>();

    cameraController_ = std::make_unique<CameraController>();
    cameraController_->Initialize(player_.get());

    stageObject_ = std::make_unique<stageObjectManager>();
    stageObject_->Initialize(stageDataLoad::GetInstance()->GetStageObjectData(), player_.get());

    ClearUI_ = std::make_unique<clearUI>();
    ClearUI_->Initialize();

    sceneState_ = SceneState::kIntro;
    clearTimer_ = 0.0f;

    // 音がうるさいので停止中
    // Audio::GetInstance()->Play(fanfare);
    // Audio::GetInstance()->Play(clearSe);
}

void GamePlayScene::Update()
{
    auto* input = Input::getInstance();

    if (isSceneFinished_) {
        return;
    }

    if (input->TriggerKey(DIK_1)) {
        isSceneFinished_ = true;
        SceneManager::GetInstance()->ChangeScene("TITLE");
    }

    if (input->TriggerKey(DIK_9)) {
        CameraManager::GetInstance()->SetActiveCamera("PlayMain");
    }
    if (input->TriggerKey(DIK_0)) {
        CameraManager::GetInstance()->SetActiveCamera("PlayBoss");
    }

    Vector3 railPos = StageManager_->CalcRailPosition(); // 現在のレーる位置を取得

    switch (sceneState_) {
    case GamePlayScene::SceneState::kIntro: {

        StageManager_->Update();

        player_->SetBasePosition(railPos);
        player_->UpdateIntro(); // 操作不能

        // 進行速度の5秒分
        float currentZ = StageManager_->GetCurrentZ();
        float progress = (currentZ - (-25.0f)) / 25.0f;

        if (progress >= 1.0f) {
            progress = 1.0f;
            sceneState_ = SceneState::kPlay;
        }

        cameraController_->UpdateIntro(railPos, progress);

        enemyManager_->Update();
        stageObject_->Update();

        break;
    }
    case GamePlayScene::SceneState::kPlay: {
        // ステージ振興
        StageManager_->Update();

        player_->SetBasePosition(railPos);
        player_->Update();

        cameraController_->Update(railPos);

        enemyManager_->Update();
        stageObject_->Update();

        // 当たり判定一括
        collisionManager_->Clear();
        collisionManager_->AddCollider(player_.get());
        for (auto& bullet : player_->GetBullets()) {
            collisionManager_->AddCollider(bullet.get());
        }
        for (auto& enemy : enemyManager_->GetEnemyes()) {
            collisionManager_->AddCollider(enemy.get());
            for (auto& enemyBullet : enemy->GetBullets()) {
                collisionManager_->AddCollider(enemyBullet.get());
            }
        }
        for (auto& stageObj : stageObject_->GetstageObjects()) {
            collisionManager_->AddCollider(stageObj.get());
        }

        collisionManager_->CheckAllCollisions();

        // 終了条件
        if (enemyManager_->IsAllEnemiesCleared()) {
            sceneState_ = SceneState::kClear;
            clearTimer_ = 0.0f;
        }

        break;
    }
    case GamePlayScene::SceneState::kClear: {
        float deltaTime = SceneManager::GetInstance()->GetDeltaTime();
        clearTimer_ += deltaTime;

        player_->UpdateClear();

        stageObject_->Update();
        ClearUI_->Update(clearTimer_);

        // 3秒経過後にリザルト画面へ移行
        if (clearTimer_ >= 5.0f) {
            isSceneFinished_ = true;
            SceneManager::GetInstance()->ChangeScene("RESULT");
        }
        break;
    }
    }
}

void GamePlayScene::Draw()
{

    Object3dCommon::GetInstance()->PrepareObjectDraw();

    //
    // モデルデータ
    //
    player_->Draw();
    enemyManager_->Draw();

    stageObject_->Draw();

    SkyBoxCommon::GetInstance()->PrepareObjectDraw();
    // skydox->Draw();

    //
    // 2d/スプライト
    //
    SpriteCommon::GetInstance()->PrepareSpriteDraw();
    player_->SpritDraw();
    enemyManager_->SpriteDraw();
    ClearUI_->SpritDraw();

    CPUParticleManager::getInstance()->Draw();
}