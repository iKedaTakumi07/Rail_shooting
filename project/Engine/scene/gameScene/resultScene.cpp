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

    state_ = State::kSceneChange;
    stateTimer_ = 1.0f;
    Transition_->Start(SceneTransition::State::In, 1.0f);
}

void resultScene::Finalize()
{
}

void resultScene::Update()
{
    float deltaTime = SceneManager::GetInstance()->GetDeltaTime();
    auto* input = Input::GetInstance();

    switch (state_) {
    case resultScene::State::kSceneChange:
        stateTimer_ -= deltaTime;
        if (stateTimer_ <= 0.0f) {
            state_ = State::kResultUpdateNow;
            ResultUI_->StartAnimation();
        }

        break;
    case resultScene::State::kResultUpdateNow:
        // 演出スキップ入力
        if (input->TriggerKey(DIK_RETURN)) {
            ResultUI_->SkipAnimation();
            state_ = State::kResultWait;
        } else if (ResultUI_->IsAnimationEnd()) {
            state_ = State::kResultWait;
        }

        break;
    case resultScene::State::kResultWait:
        // リザルト確認完了後、Enterで惑星脱出演出へ移行
        if (input->TriggerKey(DIK_RETURN)) {
            ResultUI_->SetVisible(false); // 脱出演出の邪魔にならないようUIを非表示化
            resultObject_->StartEscape(); // 機体・カメラの脱出シークエンス開始
            state_ = State::kPlanetEscape;
        }
        break;
    case resultScene::State::kPlanetEscape:
        // 脱出演出の完了を待機してからフェードアウト開始
        if (resultObject_->IsEscapeFinished()) {
            Transition_->Start(SceneTransition::State::Out, 1.0f);
            stateTimer_ = 1.0f;
            state_ = State::kReturnSelect;
        }
        break;
    case resultScene::State::kReturnSelect:
        stateTimer_ -= deltaTime;
        if (stateTimer_ <= 0.0f) {
            SceneManager::GetInstance()->ChangeScene("SELECT");
            return;
        }
        break;
    }

    Transition_->Update(deltaTime);
    resultObject_->Update();
    ResultUI_->Update();
    skydome_->Update();
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
