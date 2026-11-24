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

#include "../../../Game/SceneTransition.h"
#include "../../../Game/UI//Result/ResultObject.h"
#include "../../../Game/UI//Result/ResultUI.h"
#include "../../../Game/stage/skydome.h"
#include "../../3d/CameraManager.h"
#include "math.h"
#include <utility>

resultScene::resultScene()
{
}

resultScene::~resultScene() = default;

void resultScene::Initialize()
{
    GPUParticleManager::getInstance()->ParticleGroupClrea();

    Camera* mainCamera = CameraManager::GetInstance()->CreateCamera("PlayMain");
    mainCamera->SetTranslate({ 0.0f, 2.0f, -15.0f });

    CameraManager::GetInstance()->SetActiveCamera("PlayMain");

    resultObject_ = std::make_unique<ResultObject>();
    resultObject_->Initialize();

    ResultUI_ = std::make_unique<ResultUI>();
    ResultUI_->Initialize();

    Transition_ = std::make_unique<SceneTransition>();
    Transition_->Initialize("resources/noise3.png");

    skydome_ = std::make_unique<skydome>();
    skydome_->Initialize();

    ChangeState(std::make_unique<SceneChange>());
}

void resultScene::Finalize()
{
}

void resultScene::Update()
{
    float deltaTime = SceneManager::GetInstance()->GetDeltaTime();

    if (isSceneFinished_) {
        return;
    }

    Transition_->Update(deltaTime);
    resultObject_->Update();
    ResultUI_->Update();
    skydome_->Update();

    if (currentState_) {
        currentState_->Update(*this, deltaTime);
    }
}

void resultScene::Draw()
{
    Object3dCommon::GetInstance()->PrepareObjectDraw();

    skydome_->Draw();
    resultObject_->Draw();

#ifdef USE_IMGUI

#endif // USE_IMGUI

    SkyBoxCommon::GetInstance()->PrepareObjectDraw();
    // skydox->Draw();

    CPUParticleManager::getInstance()->Draw();
    GPUParticleManager::getInstance()->Draw();

    SpriteCommon::GetInstance()->PrepareSpriteDraw();
    ResultUI_->SpritDraw();
}

void resultScene::ChangeState(std::unique_ptr<IResultSceneState> nextState)
{
    if (currentState_) {
        currentState_->Exit(*this);
    }
    currentState_ = std::move(nextState);
    if (currentState_) {
        currentState_->Enter(*this);
    }
}

void resultScene::FinishAndChangeScene(const std::string& nextSceneName)
{
    isSceneFinished_ = true;
    SceneManager::GetInstance()->ChangeScene(nextSceneName);
}
