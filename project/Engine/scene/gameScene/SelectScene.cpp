#include "SelectScene.h"
#include "../SceneManager.h"

#include "../../../Game/Loder/stageDataLoad.h"
#include "../../../Game/SceneTransition.h"
#include "../../../Game/UI/Select/stageSelectUI.h"
#include "../../../Game/stage/skydome.h"
#include "../../2d/SpriteCommon.h"
#include "../../3d/CPUParticle/CPUParticleManager.h"
#include "../../3d/Camera.h"
#include "../../3d/CameraManager.h"
#include "../../3d/Object3dCommon.h"
#include "../../3d/Skybox/SkyBoxCommon.h"
#include "../../base/TextureManager.h"
#include "../../io/Input.h"

SelectScene::SelectScene()
{
}

SelectScene::~SelectScene() = default;

void SelectScene::Finalize()
{
}

void SelectScene::Initialize()
{
    Camera* mainCamera = CameraManager::GetInstance()->CreateCamera("PlayMain");
    mainCamera->SetTranslate({ 0.0f, 2.0f, -15.0f });

    CameraManager::GetInstance()->SetActiveCamera("PlayMain");

    stageNumber = 1;

    stageSelectUI_ = std::make_unique<stageSelectUI>();
    stageSelectUI_->Initialize();

    skydome_ = std::make_unique<skydome>();
    skydome_->Initialize();
}

void SelectScene::Update()
{
    float deltaTime = SceneManager::GetInstance()->GetDeltaTime();

    stageSelectUI_->Update(deltaTime);
    skydome_->Update();

    if (stageSelectUI_->IsGameChangeFinished()) {
        SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
    } else if (stageSelectUI_->IsTitleBackFinished()) {
        SceneManager::GetInstance()->ChangeScene("TITLE");
    }
}

void SelectScene::Draw()
{
    Object3dCommon::GetInstance()->PrepareObjectDraw();
    //
    // モデルデータ
    //
    skydome_->Draw();
    stageSelectUI_->Draw();

    SkyBoxCommon::GetInstance()->PrepareObjectDraw();

    //
    // 2d/スプライト
    //
    SpriteCommon::GetInstance()->PrepareSpriteDraw();

    stageSelectUI_->SpriteDraw();

    CPUParticleManager::getInstance()->Draw();
}
