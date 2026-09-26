#include "SelectScene.h"
#include "../SceneManager.h"

#include "../../../Game/SceneTransition.h"
#include "../../../Game/stage/skydome.h"
#include "../../../Game/stage/stageDataLoad.h"
#include "../../../Game/stage/stageSelectUI.h"
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

    TextureManager::getInstance()->LoadTexture("resources/selectUI/stage1UI.png");
    TextureManager::getInstance()->LoadTexture("resources/selectUI/stage2UI.png");

    SatgeUI1 = std::make_unique<Sprite>();
    SatgeUI1->Initialize("resources/selectUI/stage1UI.png");
    SatgeUI1->SetPosition(Vector2(400.0f, 280.0f));

    SatgeUI2 = std::make_unique<Sprite>();
    SatgeUI2->Initialize("resources/selectUI/stage2UI.png");
    SatgeUI2->SetPosition(Vector2(400.0f, 280.0f));

    stageNumber = 1;

    stageSelectUI_ = std::make_unique<stageSelectUI>();
    stageSelectUI_->Initialize();

    Transition_ = std::make_unique<SceneTransition>();
    Transition_->Initialize("resources/noise3.png");

    Transition_->Start(SceneTransition::State::In, 1.0f);

    skydome_ = std::make_unique<skydome>();
    skydome_->Initialize();
}

void SelectScene::Update()
{
    auto* input = Input::getInstance();
    float deltaTime = SceneManager::GetInstance()->GetDeltaTime();

    if (!selectStop && !stageSelectUI_->GetisMoving()) {
        int prevStage = stageNumber;

        if (input->TriggerKey(DIK_D) || input->TriggerKey(DIK_RIGHTARROW)) {
            if (stageNumber < MaxStageNumber) {
                stageNumber++;
            }
        }
        if (input->TriggerKey(DIK_A) || input->TriggerKey(DIK_LEFTARROW)) {
            if (stageNumber > MinStageNumber) {
                stageNumber--;
            }
        }

        if (stageNumber != prevStage) {
            stageSelectUI_->ChangeStage(stageNumber);
        }

        if (input->TriggerKey(DIK_SPACE)) {
            selectStop = true;
            GameChange = true;
            GameChangeTimer = kGameChangeTimer;

            stageSelectUI_->StartSortie(stageNumber, GameChangeTimer);
            stageDataLoad::GetInstance()->SetStage(stageNumber);
        }

        if (input->TriggerKey(DIK_BACKSPACE)) {
            selectStop = true;
            isTitile = true;
            Transition_->Start(SceneTransition::State::Out, 1.0f);
        }
    }

    if (GameChange) {
        GameChangeTimer -= deltaTime;

        Vector2 Center = { 0.5f, 0.5f }; // 中心位置

        float Blur = 1.0f - (GameChangeTimer / 0.5f);

        PostProcess::GetInstance()->SetRadialBlur(true);
        PostProcess::GetInstance()->SetRadialBlurParam(Center, Blur);

        if (GameChangeTimer <= 0.0f) {
            GameChangeTimer = 0.0f;
            SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
        }
    }
    if (isTitile) {
        titleChangeTimer -= deltaTime;

        if (titleChangeTimer <= 0.0f) {

            SceneManager::GetInstance()->ChangeScene("TITLE");
        }
    }

    stageSelectUI_->Update(deltaTime);
    SatgeUI1->Update();
    SatgeUI2->Update();
    skydome_->Update();
    Transition_->Update(deltaTime);
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
    // skydox->Draw();

    //
    // 2d/スプライト
    //
    SpriteCommon::GetInstance()->PrepareSpriteDraw();
    if (stageNumber == 1) {
        SatgeUI1->Draw();
    } else if (stageNumber == 2) {
        SatgeUI2->Draw();
    }

    stageSelectUI_->SpriteDraw();

    CPUParticleManager::getInstance()->Draw();
}
