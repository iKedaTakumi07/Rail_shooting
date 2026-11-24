#include "ResultSceneState.h"
#include "../../../Game/SceneTransition.h"
#include "../../../Game/UI//Result/ResultObject.h"
#include "../../../Game/UI//Result/ResultUI.h"
#include "../../../Game/stage/skydome.h"
#include "../../3d/CameraManager.h"
#include "../../io/Input.h"
#include "../gameScene/resultScene.h"
#include <memory>

void SceneChange::Enter(resultScene& scene)
{
    scene.GetTransition()->Start(SceneTransition::State::In, 1.0f);
}

void SceneChange::Update(resultScene& scene, float deltaTime)
{
    stateTimer_ -= deltaTime;
    if (stateTimer_ <= 0.0f) {
        scene.ChangeState(std::make_unique<PlayerView>());
        return;
    }
}

void PlayerView::Update(resultScene& scene, float deltaTime)
{
    // ただ自機を映すだけ
    stateTimer_ -= deltaTime;
    if (stateTimer_ <= 0.0f) {
        scene.ChangeState(std::make_unique<ResultUpdateNow>());
        return;
    }
}

void ResultUpdateNow::Enter(resultScene& scene)
{
    scene.GetResultUI()->StartAnimation();
}

void ResultUpdateNow::Update(resultScene& scene, float deltaTime)
{
    (void)deltaTime;
    auto* input = Input::GetInstance();
    auto* ResultUI = scene.GetResultUI();
    bool isSkipInput = false;

    if (input->IsControllerConnected() && input->TriggerButton(XINPUT_GAMEPAD_A)) {
        isSkipInput = true;
    }

    if (input->TriggerKey(DIK_RETURN)) {
        isSkipInput = true;
    }

    if (isSkipInput) {
        ResultUI->SkipAnimation();
        scene.ChangeState(std::make_unique<ResultWait>());
        return;
    }

    if (ResultUI->IsAnimationEnd()) {
        scene.ChangeState(std::make_unique<ResultWait>());
        return;
    }
}

void ResultWait::Update(resultScene& scene, float deltaTime)
{
    (void)deltaTime;
    auto* input = Input::GetInstance();

    bool isConfirmInput = false;
    if (input->IsControllerConnected() && input->TriggerButton(XINPUT_GAMEPAD_A)) {
        isConfirmInput = true;
    }
    if (input->TriggerKey(DIK_RETURN)) {
        isConfirmInput = true;
    }

    if (isConfirmInput) {
        scene.ChangeState(std::make_unique<PlanetEscape>());
        return;
    }
}

void PlanetEscape::Enter(resultScene& scene)
{
    scene.GetResultUI()->SetVisible(false);
    scene.GetResultObject()->StartEscape();
}

void PlanetEscape::Update(resultScene& scene, float deltaTime)
{
    (void)deltaTime;
    if (scene.GetResultObject()->IsEscapeFinished()) {
        scene.ChangeState(std::make_unique<ReturnSelect>());
        return;
    }
}

void ReturnSelect::Enter(resultScene& scene)
{
    scene.GetTransition()->Start(SceneTransition::State::Out, 1.0f);
}

void ReturnSelect::Update(resultScene& scene, float deltaTime)
{
    stateTimer_ -= deltaTime;
    if (stateTimer_ <= 0.0f) {
        scene.FinishAndChangeScene("SELECT");
        return;
    }
}
