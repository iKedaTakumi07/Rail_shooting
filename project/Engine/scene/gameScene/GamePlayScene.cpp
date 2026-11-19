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
#include "../../../Game/Loder/GameScoreManager.h"
#include "../../../Game/Loder/StageDataEditor.h"
#include "../../../Game/Loder/stageDataLoad.h"
#include "../../../Game/OnCollison/CollisionManager.h"
#include "../../../Game/Player/PlayerManager.h"
#include "../../../Game/SceneTransition.h"
#include "../../../Game/UI/Game/IntroUI.h"
#include "../../../Game/UI/Game/pauseUI.h"
#include "../../../Game/UI/Result/clearUI.h"
#include "../../../Game/stage/StageManager.h"
#include "../../../Game/stage/skydome.h"
#include "../../../Game/stage/stageObjectManager.h"
#include "../../3d/GPUParticleManager.h"

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
    GameScoreManager::GetInstance()->Reset();

    Camera* mainCamera = CameraManager::GetInstance()->CreateCamera("PlayMain");
    mainCamera->SetTranslate({ 0.0f, 0.0f, -15.0f });
    mainCamera->SetRotate({ 0.0f, 0.0f, 0.0f });

    Camera* bossCamera = CameraManager::GetInstance()->CreateCamera("PlayBoss");
    bossCamera->SetTranslate({ 0.0f, 10.0f, -40.0f });

    TextureManager::getInstance()->LoadTexture("resources/uvChecker.png");
    TextureManager::getInstance()->LoadTexture("resources/monsterBall.png");
    TextureManager::getInstance()->LoadTexture("resources/rostock_laage_airport_4k.dds");
    TextureManager::getInstance()->FlushUploads();

    skydox = std::make_unique<Skybox>();
    skydox->Initialize("resources/rostock_laage_airport_4k.dds");

    ModelManager::GetInstance()->LoadModel("Plane.obj");
    ModelManager::GetInstance()->LoadModel("axis.obj");

    fanfare.SoundLoadFile("resources/fanfare.wav");
    clearSe.SoundLoadFile("resources/stage.mp3");

    StageManager_ = std::make_unique<StageManager>();
    StageManager_->Initialize(stageDataLoad::GetInstance()->GetStageData());

    playerManager_ = std::make_unique<PlayerManager>();
    playerManager_->Initialize();
    Player* player = playerManager_->GetPlayer();

    enemyManager_ = std::make_unique<EnemyManager>();
    enemyManager_->Initialize(player, stageDataLoad::GetInstance()->GetEnemyPopData()); // 読み込むファイルを決めるクラス(インスタンス化)で作成する
    player->SetEnemyManager(enemyManager_.get());

    collisionManager_ = std::make_unique<CollisionManager>();

    cameraController_ = std::make_unique<CameraController>();
    cameraController_->Initialize(player);

    Transition_ = std::make_unique<SceneTransition>();
    Transition_->Initialize("resources/noise2.png");

    stageObject_ = std::make_unique<stageObjectManager>();
    stageObject_->Initialize(stageDataLoad::GetInstance()->GetStageObjectData(), player);

    ClearUI_ = std::make_unique<clearUI>();
    ClearUI_->Initialize();

    PauseUI_ = std::make_unique<pauseUI>();
    PauseUI_->Initialize();

    introUI_ = std::make_unique<IntroUI>();
    introUI_->Initialize();

    gameOverUI_ = std::make_unique<GameOverUI>();
    gameOverUI_->Initialize();

    skydome_ = std::make_unique<skydome>();
    skydome_->Initialize();

    stageDataEditor_ = std::make_unique<StageDataEditor>();
    stageDataEditor_->Initialize(enemyManager_.get(), stageObject_.get(), StageManager_.get());

    pausedState_.reset();
    ChangeState(std::make_unique<IntroState>());

    // 音がうるさいので停止中
    // Audio::GetInstance()->Play(fanfare);
    // Audio::GetInstance()->Play(clearSe);
}

void GamePlayScene::Update()
{
    auto* input = Input::GetInstance();
    float deltaTime = SceneManager::GetInstance()->GetDeltaTime();

    if (isSceneFinished_) {
        return;
    }

#ifdef USE_IMGUI
    // リリース版使用不可
    if (input->TriggerKey(DIK_T)) {
        FinishAndChangeScene("TITLE");
        return;
    }

    if (input->TriggerKey(DIK_P)) {
        CameraManager::GetInstance()->SetActiveCamera("PlayMain");
    }
    if (input->TriggerKey(DIK_B)) {
        CameraManager::GetInstance()->SetActiveCamera("PlayBoss");
    }

    stageDataEditor_->DrawImGui();

#endif // DEBUG

    skydome_->Update();

    if (currentState_) {
        currentState_->Update(*this, deltaTime);
    }
}

void GamePlayScene::Draw()
{

    Object3dCommon::GetInstance()->PrepareObjectDraw();

    //
    // モデルデータ
    //
    skydome_->Draw();
    stageObject_->Draw();

    playerManager_->Draw();
    enemyManager_->Draw();

#ifdef USE_IMGUI
    // リリース版使用不可
    Object3dCommon::GetInstance()->PreLineObjectDraw();
    if (currentState_) {
        currentState_->DrawDebug(*this);
    }

#endif // DEBUG

    SkyBoxCommon::GetInstance()->PrepareObjectDraw();

    CPUParticleManager::getInstance()->Draw();

    GPUParticleManager::getInstance()->Draw();

    //
    // 2d/スプライト
    //
    SpriteCommon::GetInstance()->PrepareSpriteDraw();
    introUI_->SpritDraw();
    playerManager_->SpritDraw();
    enemyManager_->SpriteDraw();
    ClearUI_->SpritDraw();
    PauseUI_->SpritDraw();

    if (currentState_) {
        currentState_->DrawSprite(*this);
    }
}

void GamePlayScene::ChangeState(std::unique_ptr<IGamePlayState> nextState)
{
    if (currentState_) {
        currentState_->Exit(*this);
    }
    currentState_ = std::move(nextState);
    if (currentState_) {
        currentState_->Enter(*this);
    }
}

void GamePlayScene::PushPauseState()
{
    if (currentState_) {
        currentState_->OnPause(*this);
        pausedState_ = std::move(currentState_);
    }
    currentState_ = std::make_unique<PauseState>();
    currentState_->Enter(*this);
}

void GamePlayScene::PopPauseState()
{
    if (currentState_) {
        currentState_->Exit(*this);
    }
    if (pausedState_) {
        currentState_ = std::move(pausedState_);
        currentState_->OnResume(*this);
    }
}

void GamePlayScene::FinishAndChangeScene(const std::string& nextSceneName)
{
    isSceneFinished_ = true;
    SceneManager::GetInstance()->ChangeScene(nextSceneName);
}

void GamePlayScene::UpdateCollisions(bool includeStageObjectRoot)
{
    collisionManager_->Clear();
    playerManager_->RegisterColliders(collisionManager_.get());

    for (auto& enemy : enemyManager_->GetEnemyes()) {
        collisionManager_->AddCollider(enemy.get());
        for (auto& enemyBullet : enemy->GetBullets()) {
            collisionManager_->AddCollider(enemyBullet.get());
        }
    }
    for (auto& stageObj : stageObject_->GetstageObjects()) {
        collisionManager_->AddCollider(stageObj.get());
    }
    if (includeStageObjectRoot) {
        collisionManager_->AddCollider(stageObject_.get());
    }

    collisionManager_->CheckAllCollisions();
}
