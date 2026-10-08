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

#include "../../../Game/Player/TitleFloating.h"
#include "../../../Game/SceneTransition.h"
#include "../../../Game/stage/meteorite.h"
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
    TextureManager::getInstance()->LoadTexture("resources/UI/Title.png");
    TextureManager::getInstance()->LoadTexture("resources/UI/TitleUI.png");

    ModelManager::GetInstance()->LoadModel("axis.obj");
    ModelManager::GetInstance()->LoadModel("terrain.obj");
    ModelManager::GetInstance()->LoadModel("plane.gltf");
    ModelManager::GetInstance()->LoadModel("AnimatedCube.gltf");
    ModelManager::GetInstance()->LoadModel("simpleSkin/simpleSkin.gltf");
    ModelManager::GetInstance()->LoadModel("human/walk.gltf");
    ModelManager::GetInstance()->LoadModel("human/sneakWalk.gltf");

    Transition_ = std::make_unique<SceneTransition>();
    Transition_->Initialize("resources/noise2.png");

    skydome_ = std::make_unique<skydome>();
    skydome_->Initialize();

    TitleScene_ = std::make_unique<Sprite>();
    TitleScene_->Initialize("resources/UI/Title.png");
    TitleScene_->SetPosition(Vector2(WinApp::KClientWidth / 2.0f, WinApp::KClientHeight / 8.0f * 1.0f));
    TitleScene_->SetAnchorPoint(Vector2(0.5f, 0.5f));

    TitleScenestateUI_ = std::make_unique<Sprite>();
    TitleScenestateUI_->Initialize("resources/UI/TitleUI.png");
    TitleScenestateUI_->SetPosition(Vector2(WinApp::KClientWidth / 2.0f, WinApp::KClientHeight / 8.0f * 7.0f)); // 中心位置
    TitleScenestateUI_->SetAnchorPoint(Vector2(0.5f, 0.5f));

    TitleFloating_ = std::make_unique<TitleFloating>();
    TitleFloating_->Initialize();
    TitleFloating_->SetStartPos(Vector3(-20.0f, 0.0f, 0.0f));
    TitleFloating_->SetEndPos(Vector3(0.0f, 0.0f, 0.0f));

    meteorite_ = std::make_unique<Meteorite>();
    meteorite_->Initialize();

    Transition_->Start(SceneTransition::State::In, 0.1f);
}

void TitleScene::Finalize()
{
}

void TitleScene::Update()
{
    auto* input = Input::GetInstance();
    float deltaTime = SceneManager::GetInstance()->GetDeltaTime();

    if (input->TriggerKey(DIK_ESCAPE)) {
        SceneManager::GetInstance()->RequestEnd();
        return;
    }

    if (!isChange) {
        if (input->TriggerKey(DIK_RETURN)) {
            isChange = true;
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

    skydome_->Update();
    TitleFloating_->Update();
    meteorite_->Update();
    Transition_->Update(deltaTime);
    TitleScene_->Update();
    TitleScenestateUI_->Update();

    // x座標だけ追尾
    if (!TitleFloating_->GetIntro() && !TitleFloating_->GetSortie()) {
        auto camera = CameraManager::GetInstance()->GetActiveCamera();
        CameraPos = camera->GetTranslate();
        CameraPos.x = TitleFloating_->GetPosition().x;
        CameraManager::GetInstance()->GetActiveCamera()->SetTranslate(CameraPos);
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
    TitleScene_->Draw();
    TitleScenestateUI_->Draw();

    // GPUParticleManager::getInstance()->Draw();
}
