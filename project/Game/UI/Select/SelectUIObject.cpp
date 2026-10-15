#include "SelectUIObject.h"
#include "../../../Engine/3d/Camera.h"
#include "../../../Engine/3d/CameraManager.h"
#include "../../../Engine/3d/ModelManager.h"
#include "../../../Engine/base/TextureManager.h"
#include <algorithm>

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
    playerObject3d_->SetTranslate(currentPlanetPos_);

    baseLabObjectModel_ = std::make_unique<Model>();
    baseLabObjectModel_->Initialize("resources/stage", "baseLab.obj");
    baseLabObject3d_ = std::make_unique<Object3d>();
    baseLabObject3d_->Initialize();
    baseLabObject3d_->SetModel(baseLabObjectModel_.get());
    baseLabObject3d_->SetTranslate(currentPlanetPos_);

    lampModel_ = std::make_unique<Model>();
    lampModel_->Initialize("resources/stage", "lamp.obj");
    lampObject3d_ = std::make_unique<Object3d>();
    lampObject3d_->Initialize();
    lampObject3d_->SetModel(lampModel_.get());
    lampObject3d_->SetTranslate(Vector3 { 0.0f, 3.5f, 4.4f });

    lampCoverModel_ = std::make_unique<Model>();
    lampCoverModel_->Initialize("resources/stage", "lampCover.obj");
    lampCoverModel_->SetMaterialColor(Vector4 { 1.0f, 1.0f, 1.0f, 0.2f });
    lampCoverObject3d_ = std::make_unique<Object3d>();
    lampCoverObject3d_->Initialize();
    lampCoverObject3d_->SetModel(lampCoverModel_.get());
    lampCoverObject3d_->SetTranslate(Vector3 { 0.0f, 3.5f, 4.0f });
}

void SelectUIObject::Update(float deltaTime, bool isSortie)
{
    MoveUpdate(deltaTime, isSortie);

    playerObject3d_->Update();
    baseLabObject3d_->Update();
    lampObject3d_->Update();
    lampCoverObject3d_->Update();

    for (int i = 0; i < maxStage; i++) {
        object3d_[i]->Update();
    }
}

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
    int idx = std::clamp(stageIndex - 1, 0, maxStage - 1);
    startPlanetPos_ = currentPlanetPos_;
    targetPlanetPos_ = Vector3(stagePos_[idx].x, stagePos_[idx].y - 1.0f, -2.0f);

    moveTimer_ = 0.0f;
    moveDuration_ = kmoveDuration_;
    isMoving_ = true;
}

void SelectUIObject::StartSortie(int stageIndex, float duration)
{
    int idx = std::clamp(stageIndex - 1, 0, maxStage - 1);
    startPlanetPos_ = currentPlanetPos_;
    targetPlanetPos_ = Vector3(stagePos_[idx].x, stagePos_[idx].y, stagePos_[idx].z + 5.0f);

    moveTimer_ = 0.0f;
    moveDuration_ = duration;
    isMoving_ = false;
}

void SelectUIObject::MoveUpdate(float deltaTime, bool isSortie)
{
    auto* camera = CameraManager::GetInstance()->GetActiveCamera();

    if (isMoving_) {
        moveTimer_ += deltaTime;
        float t = std::clamp(moveTimer_ / moveDuration_, 0.0f, 1.0f);
        float easedT = EaseOutCubic(t);

        currentPlanetPos_ = Lerp(startPlanetPos_, targetPlanetPos_, easedT);
        playerObject3d_->SetTranslate(currentPlanetPos_);
        baseLabObject3d_->SetTranslate(currentPlanetPos_);

        if (t >= 1.0f) {
            isMoving_ = false;
        }
    } else if (isSortie) {
        moveTimer_ += deltaTime;
        float t = std::clamp(moveTimer_ / moveDuration_, 0.0f, 1.0f);
        float easedT = EaseOutCubic(t);

        currentPlanetPos_ = Lerp(startPlanetPos_, targetPlanetPos_, easedT);
        playerObject3d_->SetTranslate(currentPlanetPos_);
    }

    for (int i = 0; i < maxStage; i++) {
        Vector3 rot = object3d_[i]->GetRotate();
        rot.y += 0.5f * deltaTime;
        object3d_[i]->SetRotate(rot);
    }

    Vector3 cameraPos = currentPlanetPos_;
    cameraPos.z += -15.0f;
    cameraPos.y += 1.0f;
    camera->SetTranslate(cameraPos);
}