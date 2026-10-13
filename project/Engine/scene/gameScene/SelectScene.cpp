#include "SelectScene.h"
#include "../SceneManager.h"

#include "../../../Game/Loder/stageDataLoad.h"
#include "../../../Game/Particle/GalaxyParticle.h"
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
    skydome_->SetTexture("resources/skydone/sky_night.png");

    galaxyParticle_ = std::make_unique<GalaxyParticle>();
    galaxyParticle_->Initialize();

    particleTimer_ = 0.0f;
}

void SelectScene::Update()
{
    float deltaTime = SceneManager::GetInstance()->GetDeltaTime();
    auto* camera = CameraManager::GetInstance()->GetActiveCamera();

    stageSelectUI_->Update(deltaTime);
    skydome_->Update();

    // 星の輝きをパーティクルで再現
    particleTimer_ += deltaTime;
    if (particleTimer_ >= 0.5f) {
        Vector3 CameraPos = camera->GetTranslate();
        Vector3 TragetPosMax = { CameraPos.x + 100.0f, CameraPos.y + 100.0f, 200.0f };
        Vector3 TragetPosMin = { CameraPos.x - 100.0f, CameraPos.y - 100.0f, 100.0f };
        galaxyParticle_->SetMaxPos(TragetPosMax);
        galaxyParticle_->SetMinPos(TragetPosMin);

        particleTimer_ -= 0.5f;
        galaxyParticle_->NewParticle();
    }

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
