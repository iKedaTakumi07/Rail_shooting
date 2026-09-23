#include "resultScene.h"
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
#include "../../../Game/SceneTransition.h"
#include "../../3d/CameraManager.h"
#include "math.h"

resultScene::resultScene()
{
}

resultScene::~resultScene() = default;

void resultScene::Initialize()
{
    Camera* mainCamera = CameraManager::GetInstance()->CreateCamera("PlayMain");
    mainCamera->SetTranslate({ 0.0f, 2.0f, -15.0f });

    CameraManager::GetInstance()->SetActiveCamera("PlayMain");

    player_ = std::make_unique<Player>();
    player_->Initialize();

    Transition_ = std::make_unique<SceneTransition>();
    Transition_->Initialize("resources/noise3.png");

    Transition_->Start(SceneTransition::State::In, 1.0f);
}

void resultScene::Finalize()
{
}

void resultScene::Update()
{
    float deltaTime = SceneManager::GetInstance()->GetDeltaTime();
    auto* input = Input::getInstance();
    if (input->TriggerKey(DIK_1)) {
        SceneManager::GetInstance()->ChangeScene("TITLE");
    }
    Transition_->Update(deltaTime);

    player_->Update();
}

void resultScene::Draw()
{
    Object3dCommon::GetInstance()->PrepareObjectDraw();

    player_->Draw();

#ifdef USE_IMGUI

#endif // USE_IMGUI

    SkyBoxCommon::GetInstance()->PrepareObjectDraw();
    // skydox->Draw();

    SpriteCommon::GetInstance()->PrepareSpriteDraw();

    CPUParticleManager::getInstance()->Draw();

    // GPUParticleManager::getInstance()->Draw();
}
