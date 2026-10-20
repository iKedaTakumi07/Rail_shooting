#include "SelectUIObject.h"
#include "../../../Engine/3d/Camera.h"
#include "../../../Engine/3d/CameraManager.h"
#include "../../../Engine/3d/ModelManager.h"
#include "../../../Engine/base/Math.h"
#include "../../../Engine/base/TextureManager.h"
#include <algorithm>
#include <cmath>
#include <numbers>

SelectUIObject::SelectUIObject() = default;

SelectUIObject::~SelectUIObject()
{
    LightManager::GetInstance()->UnregisterPointLight(&pointLight_);

    DirectionalLight light;
    light.active = 1;
    light.color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    light.direction = Vector3(0.0f, -1.0f, 0.0f);
    light.intensity = 1.0f;
    LightManager::GetInstance()->SetDirectionalLight(light);
}

void SelectUIObject::Initialize()
{
    TextureManager::getInstance()->LoadTexture("resources/skydone/sky_sphere.png");
    TextureManager::getInstance()->LoadTexture("resources/player/1x1white.png");
    TextureManager::getInstance()->LoadTexture("resources/stage/uvChecker.png");
    TextureManager::getInstance()->LoadTexture("resources/stage/1x1white.png");
    TextureManager::getInstance()->LoadTexture("resources/stage/test.png");

    ModelManager::GetInstance()->LoadModel("player/Player.obj");
    ModelManager::GetInstance()->LoadModel("skydone/Selectskydome.obj");
    ModelManager::GetInstance()->LoadModel("stage/baseLab.obj");
    ModelManager::GetInstance()->LoadModel("stage/lamp.obj");
    ModelManager::GetInstance()->LoadModel("stage/lampCover.obj");

    stagePos_[0] = { 0.0f, 0.0f, 20.0f };
    stagePos_[1] = { 15.0f, 0.0f, 20.0f };

    objectModel_ = std::make_unique<Model>();
    objectModel_->Initialize("resources/skydone", "Selectskydome.obj");
    for (int i = 0; i < maxStage; i++) {
        object3d_[i] = std::make_unique<Object3d>();
        object3d_[i]->Initialize();
        object3d_[i]->SetModel(objectModel_.get());
        object3d_[i]->SetTranslate(stagePos_[i]);
        object3d_[i]->SetScale(Vector3(3.0f, 3.0f, 3.0f));
        object3d_[i]->SetRotate(Vector3(0.0f, 0.0f, 0.0f));
    }

    currentPlanetPos_ = Vector3(stagePos_[0].x, stagePos_[0].y - 1.0f, 0.0f);
    targetPlanetPos_ = currentPlanetPos_;

    playerObjectModel_ = std::make_unique<Model>();
    playerObjectModel_->Initialize("resources/player", "Player.obj");
    playerObject3d_ = std::make_unique<Object3d>();
    playerObject3d_->Initialize();
    playerObject3d_->SetModel(playerObjectModel_.get());

    // Z-方向から
    Vector3 startLandingPos = currentPlanetPos_;
    startLandingPos.z -= 15.0f;
    startLandingPos.y += 8.0f;
    playerObject3d_->SetTranslate(startLandingPos);

    baseLabObjectModel_ = std::make_unique<Model>();
    baseLabObjectModel_->Initialize("resources/stage", "baseLab.obj");
    baseLabObject3d_ = std::make_unique<Object3d>();
    baseLabObject3d_->Initialize();
    baseLabObject3d_->SetModel(baseLabObjectModel_.get());
    baseLabObject3d_->SetTranslate(Vector3 { 0.0f, 0.0f, 0.0f });

    lampModel_ = std::make_unique<Model>();
    lampModel_->Initialize("resources/stage", "lamp.obj");
    lampObject3d_ = std::make_unique<Object3d>();
    lampObject3d_->Initialize();
    lampObject3d_->SetModel(lampModel_.get());
    lampObject3d_->SetTranslate(lanpPos_);

    lampCoverModel_ = std::make_unique<Model>();
    lampCoverModel_->Initialize("resources/stage", "lampCover.obj");
    lampCoverModel_->SetMaterialColor(Vector4 { 1.0f, 1.0f, 1.0f, 0.2f });
    lampCoverObject3d_ = std::make_unique<Object3d>();
    lampCoverObject3d_->Initialize();
    lampCoverObject3d_->SetModel(lampCoverModel_.get());
    lampCoverObject3d_->SetTranslate(lampCover);

    // 光源セット
    pointLight_.color = Vector4(1.0f, 0.2f, 0.2f, 1.0f);
    pointLight_.position = lanpPos_;
    pointLight_.intensity = 2.5f;
    pointLight_.radius = 5.0f;
    pointLight_.decay = 1.0f;
    pointLight_.active = 0;

    // LightManager に登録
    LightManager::GetInstance()->RegisterPointLight(&pointLight_);

    sequenceState_ = SequenceState::kLanding;
    sequenceTimer_ = 0.0f;
}

void SelectUIObject::Update(float deltaTime, int uiState)
{
    MoveUpdate(deltaTime);
    UpdateLightEffect(deltaTime);

    playerObject3d_->Update();
    baseLabObject3d_->Update();
    lampObject3d_->Update();
    lampCoverObject3d_->Update();

    for (int i = 0; i < maxStage; i++) {
        object3d_[i]->Update();
    }
}

// void SelectUIObject::Update(float deltaTime, bool isSortie)
//{
//     MoveUpdate(deltaTime, isSortie);
//
//     playerObject3d_->Update();
//     baseLabObject3d_->Update();
//     lampObject3d_->Update();
//     lampCoverObject3d_->Update();
//
//     for (int i = 0; i < maxStage; i++) {
//         object3d_[i]->Update();
//     }
// }

void SelectUIObject::Draw()
{
    playerObject3d_->Draw();
    baseLabObject3d_->Draw();
    lampObject3d_->Draw();
    lampCoverObject3d_->Draw();

    for (int i = 0; i < maxStage; i++) {
        object3d_[i]->Draw();
    }
}

void SelectUIObject::ChangeStage(int stageIndex)
{
    if (sequenceState_ != SequenceState::kIdle)
        return;

    int idx = std::clamp(stageIndex - 1, 0, maxStage - 1);
    startPlanetPos_ = currentPlanetPos_;
    targetPlanetPos_ = Vector3(stagePos_[idx].x, stagePos_[idx].y - 1.0f, -2.0f);

    moveTimer_ = 0.0f;
    moveDuration_ = kmoveDuration_;
    isMoving_ = true;
}

void SelectUIObject::StartSortie(int stageIndex)
{
    int idx = std::clamp(stageIndex - 1, 0, maxStage - 1);
    startPlanetPos_ = playerObject3d_->GetTranslate();
    targetPlanetPos_ = stagePos_[idx];

    sequenceState_ = SequenceState::kSortieRise;
    sequenceTimer_ = 0.0f;
}

// void SelectUIObject::StartSortie(int stageIndex, float duration)
//{
//     int idx = std::clamp(stageIndex - 1, 0, maxStage - 1);
//     startPlanetPos_ = currentPlanetPos_;
//     targetPlanetPos_ = Vector3(stagePos_[idx].x, stagePos_[idx].y, stagePos_[idx].z + 5.0f);
//
//     moveTimer_ = 0.0f;
//     moveDuration_ = duration;
//     isMoving_ = false;
// }

void SelectUIObject::StartTitleBack()
{
    sequenceState_ = SequenceState::kTitleBack;
    sequenceTimer_ = 0.0f;
    startPlanetPos_ = playerObject3d_->GetTranslate();
}

void SelectUIObject::MoveUpdate(float deltaTime)
{
    auto* camera = CameraManager::GetInstance()->GetActiveCamera();

    // ステージ移動
    if (isMoving_) {
        moveTimer_ += deltaTime;
        float t = std::clamp(moveTimer_ / moveDuration_, 0.0f, 1.0f);
        float easedT = EaseOutCubic(t);

        currentPlanetPos_ = Lerp(startPlanetPos_, targetPlanetPos_, easedT);
        playerObject3d_->SetTranslate(currentPlanetPos_);
        baseLabObject3d_->SetTranslate(Vector3(currentPlanetPos_.x, 0.0f, 0.0f));
        lampObject3d_->SetTranslate(Vector3(lanpPos_.x + currentPlanetPos_.x, lanpPos_.y, lanpPos_.z));
        lampCoverObject3d_->SetTranslate(Vector3(lampCover.x + currentPlanetPos_.x, lampCover.y, lampCover.z));

        if (t >= 1.0f) {
            isMoving_ = false;
        }
    }

    // 演出
    sequenceTimer_ += deltaTime;

    switch (sequenceState_) {
    case SequenceState::kLanding: {
        // 奥から手前に移動
        float t = std::clamp(sequenceTimer_ / kLandingDuration, 0.0f, 1.0f);
        float easedT = EaseOutCubic(t);
        Vector3 startPos = currentPlanetPos_;
        startPos.z -= 15.0f;
        startPos.y += 3.0f;

        playerObject3d_->SetTranslate(Lerp(startPos, currentPlanetPos_, easedT));
        if (t >= 1.0f) {
            sequenceState_ = SequenceState::kIdle;
        }
        break;
    }

    case SequenceState::kSortieRise: {
        // 少し浮遊する
        float t = std::clamp(sequenceTimer_ / kRiseDuration, 0.0f, 1.0f);
        float easedT = EaseOutCubic(t);
        Vector3 targetPos = currentPlanetPos_;
        targetPos.y += 1.0f;

        playerObject3d_->SetTranslate(Lerp(currentPlanetPos_, targetPos, easedT));
        if (t >= 1.0f) {
            sequenceState_ = SequenceState::kSortieCharge;
            sequenceTimer_ = 0.0f;
        }
        break;
    }

    case SequenceState::kSortieCharge: {
        // 溜め時間
        if (sequenceTimer_ >= kChargeDuration) {
            sequenceState_ = SequenceState::kSortieLaunch;
            sequenceTimer_ = 0.0f;
            startPlanetPos_ = playerObject3d_->GetTranslate();
        }
        break;
    }

    case SequenceState::kSortieLaunch: {
        // 惑星に向かう
        float t = std::clamp(sequenceTimer_ / kLaunchDuration, 0.0f, 1.0f);
        float easedT = EaseInQuad(t);

        playerObject3d_->SetTranslate(Lerp(startPlanetPos_, targetPlanetPos_, easedT));
        break;
    }

    case SequenceState::kTitleBack: {
        // 後ろを向いてから移動
        if (sequenceTimer_ <= kRotateDuration) {

            float tRotate = std::clamp(sequenceTimer_ / kRotateDuration, 0.0f, 1.0f);
            float easedRotate = EaseOutCubic(tRotate);

            Vector3 rot = playerObject3d_->GetRotate();
            rot.y = Lerp(0.0f, std::numbers::pi_v<float>, easedRotate);
            playerObject3d_->SetRotate(rot);

            playerObject3d_->SetTranslate(startPlanetPos_);
        } else {

            float tMove = std::clamp((sequenceTimer_ - kRotateDuration) / kMoveDuration, 0.0f, 1.0f);
            float easedMove = EaseInQuad(tMove);

            Vector3 rot = playerObject3d_->GetRotate();
            rot.y = std::numbers::pi_v<float>;
            playerObject3d_->SetRotate(rot);

            Vector3 backTarget = startPlanetPos_;
            backTarget.z -= 25.0f;
            playerObject3d_->SetTranslate(Lerp(startPlanetPos_, backTarget, easedMove));
        }
        break;
    }

    case SequenceState::kIdle:
        break;
    }

    // 惑星自転
    for (int i = 0; i < maxStage; i++) {
        Vector3 rot = object3d_[i]->GetRotate();
        rot.y += 0.5f * deltaTime;
        object3d_[i]->SetRotate(rot);
    }

    // カメラ追従
    Vector3 cameraPos = playerObject3d_->GetTranslate();
    cameraPos.z += -15.0f;
    cameraPos.y += 1.0f;
    camera->SetTranslate(cameraPos);
}

void SelectUIObject::UpdateLightEffect(float deltaTime)
{
    if (sequenceState_ == SequenceState::kSortieRise || sequenceState_ == SequenceState::kSortieCharge) {

        DirectionalLight dirLight;
        dirLight.active = 1;
        dirLight.color = Vector4(1.0f, 0.2f, 0.2f, 1.0f);
        dirLight.direction = Vector3(0.0f, -1.0f, 0.0f);
        dirLight.intensity = 1.0f;
        LightManager::GetInstance()->SetDirectionalLight(dirLight);

        pointLight_.active = 1;
        pointLight_.color = Vector4(1.0f, 0.2f, 0.2f, 1.0f);
        pointLight_.position = Vector3(lanpPos_.x + currentPlanetPos_.x, lanpPos_.y, lanpPos_.z);

        float sineVal = (std::sin(sequenceTimer_ * 30.0f) + 1.0f) * 0.5f;
        float stepCount = std::floor(sineVal * 2.99f);
        pointLight_.intensity = 1.0f + stepCount * 4.0f;

    } else {

        DirectionalLight dirLight;
        dirLight.active = 1;
        dirLight.color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
        dirLight.direction = Vector3(0.0f, -1.0f, 0.0f);
        dirLight.intensity = 1.0f;
        LightManager::GetInstance()->SetDirectionalLight(dirLight);

        pointLight_.active = 0;
    }
}