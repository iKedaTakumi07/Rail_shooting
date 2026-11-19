#include "GamePlayState.h"
#include "../gameScene/GamePlayScene.h"

#include "../../3d/CameraManager.h"
#include "../SceneManager.h"

#include "../../../Game/Camera/CameraController.h"
#include "../../../Game/Enemy/EnemyManager.h"
#include "../../../Game/OnCollison/CollisionManager.h"
#include "../../../Game/Player/Player.h"
#include "../../../Game/Player/PlayerManager.h"
#include "../../../Game/SceneTransition.h"
#include "../../../Game/UI/Game/GameOverUI.h"
#include "../../../Game/UI/Game/IntroUI.h"
#include "../../../Game/UI/Game/pauseUI.h"
#include "../../../Game/UI/Result/clearUI.h"
#include "../../../Game/stage/StageManager.h"
#include "../../../Game/stage/stageObjectManager.h"
#include <memory>

// イントロ
void IntroState::Update(GamePlayScene& scene, float deltaTime)
{
    (void)deltaTime;
    auto* stageManager = scene.GetStageManager();
    auto* pauseUI = scene.GetPauseUI();
    auto* introUI = scene.GetIntroUI();
    auto* playerManager = scene.GetPlayerManager();

    stageManager->Update();
    Vector3 railPos = stageManager->CalcRailPosition();

    pauseUI->Update();
    if (pauseUI->GetPause()) {
        scene.PushPauseState();
        return;
    }

    Player* player = playerManager->GetPlayer();
    player->SetBasePosition(railPos);

    // 進行速度の5秒分
    float currentZ = stageManager->GetCurrentZ();
    float progress = (currentZ - (-100.0f)) / 100.0f;

    bool isFinished = false;
    if (progress >= 1.0f) {
        progress = 1.0f;
        introUI->SetIntroEnd(true);
        isFinished = true;
    }

    playerManager->UpdateIntro(progress);
    scene.GetCameraController()->UpdateIntro(railPos);
    introUI->Update(progress);

    scene.GetEnemyManager()->Update();
    scene.GetStageObjectManager()->Update();

    if (isFinished) {
        scene.ChangeState(std::make_unique<PlayState>());
    }
}

void IntroState::OnPause(GamePlayScene& scene)
{
    scene.GetIntroUI()->SetiskPause(true);
}

void IntroState::OnResume(GamePlayScene& scene)
{
    scene.GetIntroUI()->SetiskPause(false);
}

// プレイ画面
void PlayState::Update(GamePlayScene& scene, float deltaTime)
{
    (void)deltaTime;
    auto* stageManager = scene.GetStageManager();
    auto* pauseUI = scene.GetPauseUI();
    auto* playerManager = scene.GetPlayerManager();
    auto* enemyManager = scene.GetEnemyManager();

    stageManager->Update();
    Vector3 railPos = stageManager->CalcRailPosition();

    pauseUI->Update();
    if (pauseUI->GetPause()) {
        scene.PushPauseState();
        return;
    }

    Player* player = playerManager->GetPlayer();
    player->SetBasePosition(railPos);
    playerManager->Update();

    if (player->IsExplosion()) {
        scene.ChangeState(std::make_unique<GameOverState>(true));
        return;
    } else if (player->IsDead()) {
        scene.ChangeState(std::make_unique<DeadState>());
        return;
    }

    scene.GetCameraController()->Update(railPos);
    enemyManager->Update();
    scene.GetStageObjectManager()->Update();

    // 当たり判定一括更新
    scene.UpdateCollisions(false);

    // クリア条件判定
    if (enemyManager->IsAllEnemiesCleared() && stageManager->IsReachedEnd()) {
        scene.ChangeState(std::make_unique<ClearState>());
    }
}

void PlayState::DrawDebug(GamePlayScene& scene)
{
    scene.GetCollisionManager()->DrawDebug(CameraManager::GetInstance()->GetActiveCamera());
}

void DeadState::Enter(GamePlayScene& scene)
{
    scene.GetGameOverUI()->Start(false);
}

// 死亡演出
void DeadState::Update(GamePlayScene& scene, float deltaTime)
{
    auto* stageManager = scene.GetStageManager();
    auto* playerManager = scene.GetPlayerManager();
    auto* gameOverUI = scene.GetGameOverUI();

    Player* player = playerManager->GetPlayer();
    stageManager->Update();
    Vector3 railPos = stageManager->CalcRailPosition();

    player->SetBasePosition(railPos);
    playerManager->Update();

    gameOverUI->Update(deltaTime);

    if (player->IsExplosion()) {
        gameOverUI->ClearExplosionTrigger();
        scene.ChangeState(std::make_unique<GameOverState>(false));
        return;
    }

    scene.GetCameraController()->UpdateDead(railPos);
    scene.GetEnemyManager()->Update();
    scene.GetStageObjectManager()->Update();

    scene.UpdateCollisions(true);
}

// ゲームオーバー
void GameOverState::Enter(GamePlayScene& scene)
{
    if (isImmediateExplosion_) {
        scene.GetGameOverUI()->Start(true);
    }
}

void GameOverState::Update(GamePlayScene& scene, float deltaTime)
{
    auto* gameOverUI = scene.GetGameOverUI();

    scene.GetPlayerManager()->Update();
    scene.GetEnemyManager()->Update();
    scene.GetStageObjectManager()->Update();

    gameOverUI->Update(deltaTime);

    if (gameOverUI->IsInteractive() && gameOverUI->IsSelected()) {
        if (gameOverUI->GetSelectedOption() == GameOverUI::SelectOption::kRetry) {
            scene.FinishAndChangeScene("GAMEPLAY");
        } else {
            scene.FinishAndChangeScene("SELECT");
        }
    }
}

void GameOverState::DrawSprite(GamePlayScene& scene)
{
    scene.GetGameOverUI()->SpritDraw();
}

// ポーズ
void PauseState::Update(GamePlayScene& scene, float deltaTime)
{
    auto* pauseUI = scene.GetPauseUI();
    pauseUI->Update();

    if (pauseUI->GetResetOrder()) {
        if (!isChange_ && !isReset_) {
            isReset_ = true;
            scene.FinishAndChangeScene("GAMEPLAY");
        }
        return;
    }

    if (pauseUI->GetSelectOrder()) {
        if (!isChange_ && !isReset_) {
            isChange_ = true;
            scene.GetTransition()->Start(SceneTransition::State::Out, 0.5f);
        }

        changeTimer_ -= deltaTime;
        if (changeTimer_ <= 0.0f) {
            scene.FinishAndChangeScene("SELECT");
        }
        return;
    }

    if (!pauseUI->GetPause() && !isChange_ && !isReset_) {
        scene.PopPauseState();
    }
}

// クリア
void ClearState::Enter(GamePlayScene& scene)
{
    clearTimer_ = 0.0f;
    scene.GetPlayerManager()->GetPlayer()->SetisClear(true);
}

void ClearState::Update(GamePlayScene& scene, float deltaTime)
{
    clearTimer_ += deltaTime;

    scene.GetPlayerManager()->UpdateClear();

    auto* transition = scene.GetTransition();
    if (clearTimer_ >= 4.0f && transition->GetState() == SceneTransition::State::None) {
        transition->Start(SceneTransition::State::Out, 1.0f);
    }

    transition->Update(deltaTime);
    scene.GetStageObjectManager()->ClearUpdate();
    scene.GetClearUI()->Update(clearTimer_);

    if (clearTimer_ >= 5.0f) {
        scene.FinishAndChangeScene("RESULT");
    }
}