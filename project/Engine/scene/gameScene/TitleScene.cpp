#include "TitleScene.h"
#include "../SceneManager.h"

#include "../../base/PostProcess.h"

#include <random>

#include "../../3d/Camera.h"
#include "../../base/WinApp.h"

#include "../../2d/SpriteCommon.h"
#include "../../base/TextureManager.h"

#include "../../3d/ModelManager.h"
#include "../../3d/Object3d.h"
#include "../../3d/Object3dCommon.h"

#include "../../3d/Skybox/SkyBoxCommon.h"

#include "../../3d/CPUParticle/CPUParticleManager.h"
#include "../../3d/GPUParticleManager.h"

#include "../../io/Input.h"

#include "../../../Game/Particle/GalaxyParticle.h"
#include "../../../Game/Player/TitleFloating.h"
#include "../../../Game/SceneTransition.h"
#include "../../../Game/UI/Title/TitleUI.h"
#include "../../../Game/stage//object/Meteorite.h"
#include "../../../Game/stage/skydome.h"
#include "../../3d/CameraManager.h"
#include "math.h"

TitleScene::TitleScene()
{
}

TitleScene::~TitleScene() = default;

void TitleScene::Initialize()
{
    Camera* mainCamera = CameraManager::GetInstance()->CreateCamera("PlayMain");
    mainCamera->SetTranslate({ 0.0f, 0.0f, -30.0f });

    Camera* subCamera = CameraManager::GetInstance()->CreateCamera("SubView");
    subCamera->SetTranslate({ 0.0f, 10.0f, -40.0f });

    CameraManager::GetInstance()->SetActiveCamera("PlayMain");

    TextureManager::getInstance()->LoadTexture("resources/rostock_laage_airport_4k.dds");
    TextureManager::getInstance()->LoadTexture("resources/uvChecker.png");
    TextureManager::getInstance()->LoadTexture("resources/grass.png");
    TextureManager::getInstance()->LoadTexture("resources/AnimatedCube_BaseColor.png");
    TextureManager::getInstance()->LoadTexture("resources/AnimatedCube_MetallicRoughness.png");
    TextureManager::getInstance()->LoadTexture("resources/simpleSkin/uvChecker.png");
    // TextureManager::getInstance()->LoadTexture("resources/human/white.png");

    ModelManager::GetInstance()->LoadModel("axis.obj");
    ModelManager::GetInstance()->LoadModel("terrain.obj");
    ModelManager::GetInstance()->LoadModel("plane.gltf");
    ModelManager::GetInstance()->LoadModel("AnimatedCube.gltf");
    ModelManager::GetInstance()->LoadModel("simpleSkin/simpleSkin.gltf");
    ModelManager::GetInstance()->LoadModel("human/walk.gltf");
    ModelManager::GetInstance()->LoadModel("human/sneakWalk.gltf");

    Transition_ = std::make_unique<SceneTransition>();
    Transition_->Initialize("resources/noise5.png");

    galaxyParticle_ = std::make_unique<GalaxyParticle>();
    galaxyParticle_->Initialize();

    particleTimer_ = 0.0f;

    titleUI_ = std::make_unique<TitleUI>();
    titleUI_->Initialize();

    skydome_ = std::make_unique<skydome>();
    skydome_->Initialize();
    skydome_->SetTexture("resources/skydone/sky_night.png");

    TitleFloating_ = std::make_unique<TitleFloating>();
    TitleFloating_->Initialize();
    TitleFloating_->SetStartPos(Vector3(-20.0f, 0.0f, 0.0f));
    TitleFloating_->SetEndPos(Vector3(0.0f, 0.0f, 0.0f));

    meteorite_ = std::make_unique<Meteorite>();
    meteorite_->Initialize();

    Transition_->Start(SceneTransition::State::In, 1.0f);
}

void TitleScene::Finalize()
{
}

void TitleScene::Update()
{
    auto* input = Input::GetInstance();
    float deltaTime = SceneManager::GetInstance()->GetDeltaTime();
    auto* camera = CameraManager::GetInstance()->GetActiveCamera();
    CameraPos = camera->GetTranslate();

#ifdef USE_IMGUI
    if (input->TriggerKey(DIK_9)) {
        CameraManager::GetInstance()->SetActiveCamera("PlayMain");
    }
    if (input->TriggerKey(DIK_0)) {
        CameraManager::GetInstance()->SetActiveCamera("SubView");
    }
    if (input->TriggerKey(DIK_1)) {
        SceneManager::GetInstance()->ChangeScene("TEST");
    }
#endif // USE_IMGUI

    titleUI_->Update();
    skydome_->Update();
    TitleFloating_->Update();
    meteorite_->Update();
    Transition_->Update(deltaTime);

    if (titleUI_->GetIsEndGame()) {
        SceneManager::GetInstance()->RequestEnd();
        return;
    }

    sceneTransitionTimer_ -= deltaTime;
    if (sceneTransitionTimer_ <= 0.0f) {
        titleSceneTransition_ = true;
    }

    // ゲームスタート処理
    if (!isChange) {
        if (titleUI_->GetIsStartGame() && titleSceneTransition_) {
            isChange = true;
            Transition_->SetMask("resources/noise4.png");
            Transition_->Start(SceneTransition::State::Out, SceneChangeTimer);

            Vector3 pos = TitleFloating_->GetPosition();
            if (TitleFloating_->GetIntro()) {
                pos = Vector3(0.0f, 0.0f, 0.0f);
            }
            float offsetX = 20.0f;
            pos.x += offsetX;
            TitleFloating_->SetEndPos(Vector3(pos.x, 0.0f, 0.0f));
            TitleFloating_->SetSortie(true);
        }
    } else {
        SceneChangeTimer -= deltaTime;
        if (SceneChangeTimer <= 0.0f) {
            SceneManager::GetInstance()->ChangeScene("SELECT");
        }
    }

    // x座標だけ追尾
    if (!TitleFloating_->GetIntro() && !TitleFloating_->GetSortie()) {

        CameraPos.x = TitleFloating_->GetPosition().x;
        CameraManager::GetInstance()->GetActiveCamera()->SetTranslate(CameraPos);
    }

    // 星の輝きをパーティクルで再現
    particleTimer_ += deltaTime;
    if (particleTimer_ >= 0.5f) {
        Vector3 TragetPosMax = { CameraPos.x + 100.0f, CameraPos.y + 100.0f, 200.0f };
        Vector3 TragetPosMin = { CameraPos.x - 100.0f, CameraPos.y - 100.0f, 100.0f };
        galaxyParticle_->SetMaxPos(TragetPosMax);
        galaxyParticle_->SetMinPos(TragetPosMin);

        particleTimer_ -= 0.5f;
        galaxyParticle_->NewParticle();
    }
}

void TitleScene::Draw()
{
    //
    // モデルデータ
    //
    Object3dCommon::GetInstance()->PrepareObjectDraw();

    skydome_->Draw();
    TitleFloating_->Draw();
    meteorite_->Draw();

#ifdef USE_IMGUI

#endif // USE_IMGUI

    CPUParticleManager::getInstance()->Draw();

    SkyBoxCommon::GetInstance()->PrepareObjectDraw();

    SpriteCommon::GetInstance()->PrepareSpriteDraw();
    titleUI_->SpriteDraw();

    // GPUParticleManager::getInstance()->Draw();
}
