#include "TitleScene.h"
#include "../SceneManager.h"

#include "../../base/PostProcess.h"

#include <random>

#include "../../3d/Camera.h"
#include "../../base/WinApp.h"

#include "../../2d/SpriteCommon.h"
#include "../../base/TextureManager.h"

#include "../../3d/Model.h"
#include "../../3d/ModelManager.h"
#include "../../3d/Object3d.h"
#include "../../3d/Object3dCommon.h"

#include "../../3d/Skybox/SkyBoxCommon.h"
#include "../../3d/Skybox/Skybox.h"

#include "../../3d/CPUParticle/CPUParticleManager.h"
#include "../../3d/CPUParticle/ParticleEmitter.h"
#include "../../3d/GPUParticleManager.h"

#include "../../io/Input.h"

#include "../../../Game/Particle/HitParticle.h"
#include "../../../Game/Particle/LaserParticle.h"
#include "../../../Game/Player/Player.h"
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
    mainCamera->SetTranslate({ 0.0f, 2.0f, -15.0f });

    Camera* subCamera = CameraManager::GetInstance()->CreateCamera("SubView");
    subCamera->SetTranslate({ 0.0f, 10.0f, -40.0f });

    CameraManager::GetInstance()->SetActiveCamera("PlayMain");

    TextureManager::getInstance()->LoadTexture("resources/rostock_laage_airport_4k.dds");
    TextureManager::getInstance()->LoadTexture("resources/uvChecker.png");
    TextureManager::getInstance()->LoadTexture("resources/grass.png");
    TextureManager::getInstance()->LoadTexture("resources/AnimatedCube_BaseColor.png");
    TextureManager::getInstance()->LoadTexture("resources/AnimatedCube_MetallicRoughness.png");
    TextureManager::getInstance()->LoadTexture("resources/simpleSkin/uvChecker.png");
    TextureManager::getInstance()->LoadTexture("resources/human/white.png");

    ModelManager::GetInstance()->LoadModel("axis.obj");
    ModelManager::GetInstance()->LoadModel("terrain.obj");
    ModelManager::GetInstance()->LoadModel("plane.gltf");
    ModelManager::GetInstance()->LoadModel("AnimatedCube.gltf");
    ModelManager::GetInstance()->LoadModel("simpleSkin/simpleSkin.gltf");
    ModelManager::GetInstance()->LoadModel("human/walk.gltf");
    ModelManager::GetInstance()->LoadModel("human/sneakWalk.gltf");

    skydome_ = std::make_unique<skydome>();
    skydome_->Initialize();
}

void TitleScene::Finalize()
{
}

void TitleScene::Update()
{

    auto* input = Input::getInstance();
    Camera* camera = GetCamera();

    // skydox->SetCamera(camera);

    if (input->TriggerKey(DIK_RETURN)) {
        SceneManager::GetInstance()->ChangeScene("SELECT");
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
}

void TitleScene::Draw()
{
    //
    // モデルデータ
    //
    Object3dCommon::GetInstance()->PrepareObjectDraw();

    skydome_->Draw();
#ifdef USE_IMGUI

#endif // USE_IMGUI

    SkyBoxCommon::GetInstance()->PrepareObjectDraw();

    SpriteCommon::GetInstance()->PrepareSpriteDraw();

    CPUParticleManager::getInstance()->Draw();

    GPUParticleManager::getInstance()->Draw();
}
