#include "Meteorite.h"

#include "../../Engine/3d/CameraManager.h"
#include "../../Engine/3d/ModelManager.h"
#include "../../Engine/3d/Object3d.h"
#include "../../Engine/base/Math.h"
#include "../../Engine/base/TextureManager.h"
#include "../../Engine/scene/SceneManager.h"

void Meteorite::Initialize()
{
    TextureManager::getInstance()->LoadTexture("resources/skydone/meteorite.png");
    ModelManager::GetInstance()->LoadModel("skydone/metrorite.obj");

    std::random_device seedGenerator;
    randomEngine = std::mt19937(seedGenerator());

    camera_ = CameraManager::GetInstance()->GetActiveCamera();

    ObjectModel = std::make_unique<Model>();
    ObjectModel->Initialize("resources/skydone", "metrorite.obj"); // 指定したパターンに

    for (int i = 0; i < maxFrontSideMetrorite; i++) {
        frontSidetransform_[i].translate = {
            randomFloat(randomeFrontSidePosMin.x, randomeFrontSidePosMax.x),
            randomFloat(randomeFrontSidePosMin.y, randomeFrontSidePosMax.y),
            randomFloat(randomeFrontSidePosMin.z, randomeFrontSidePosMax.z)
        };
        frontSidetransform_[i].rotate = { 0.0f, 0.0f, 0.0f };
        frontSidetransform_[i].scale = { 1.0f, 1.0f, 1.0f };

        frontSideMetroriteObject_[i] = std::make_unique<Object3d>();
        frontSideMetroriteObject_[i]->Initialize();
        frontSideMetroriteObject_[i]->SetModel(ObjectModel.get());

        frontSideMetroriteObject_[i]->SetTranslate(frontSidetransform_[i].translate);
        frontSideMetroriteObject_[i]->SetScale(frontSidetransform_[i].scale);
        frontSideMetroriteObject_[i]->SetRotate(frontSidetransform_[i].rotate);

        frontSideSpeed[i] = {
            randomFloat(speedMin.x, speedMax.x),
            randomFloat(speedMin.y, speedMax.y),
            randomFloat(speedMin.z, speedMax.z)
        };
    }
    for (int i = 0; i < maxBackSideMetrorite; i++) {
        backSidetransform_[i].translate = {
            randomFloat(randomeBackSidePosMin.x, randomeBackSidePosMax.x),
            randomFloat(randomeBackSidePosMin.y, randomeBackSidePosMax.y),
            randomFloat(randomeBackSidePosMin.z, randomeBackSidePosMax.z)
        };
        backSidetransform_[i].rotate = { 0.0f, 0.0f, 0.0f };
        backSidetransform_[i].scale = { 1.0f, 1.0f, 1.0f };

        backSideMetroriteObject_[i] = std::make_unique<Object3d>();
        backSideMetroriteObject_[i]->Initialize();
        backSideMetroriteObject_[i]->SetModel(ObjectModel.get());

        backSideMetroriteObject_[i]->SetTranslate(backSidetransform_[i].translate);
        backSideMetroriteObject_[i]->SetScale(backSidetransform_[i].scale);
        backSideMetroriteObject_[i]->SetRotate(backSidetransform_[i].rotate);

        backSideSpeed[i] = {
            randomFloat(speedMin.x, speedMax.x),
            randomFloat(speedMin.y, speedMax.y),
            randomFloat(speedMin.z, speedMax.z)
        };
    }
}

void Meteorite::Update()
{
    float deltaTime = SceneManager::GetInstance()->GetDeltaTime();
    if (camera_ != CameraManager::GetInstance()->GetActiveCamera()) {
        camera_ = CameraManager::GetInstance()->GetActiveCamera();
    }

    // 移動ナウ
    for (int i = 0; i < maxFrontSideMetrorite; i++) {
        Vector3 cameraPos = camera_->GetTranslate();
        if (ChangePos.x + cameraPos.x >= frontSidetransform_[i].translate.x) {
            frontSidetransform_[i].translate = {
                randomFloat(randomeFrontSidePosMin.x + cameraPos.x, randomeFrontSidePosMax.x + cameraPos.x),
                randomFloat(randomeFrontSidePosMin.y + cameraPos.y, randomeFrontSidePosMax.y + cameraPos.y),
                randomFloat(randomeFrontSidePosMin.z + cameraPos.z, randomeFrontSidePosMax.z + cameraPos.z)
            };
        }

        frontSidetransform_[i].translate.x += frontSideSpeed[i].x * deltaTime;
        frontSidetransform_[i].translate.y += frontSideSpeed[i].y * deltaTime;
        frontSidetransform_[i].translate.z += frontSideSpeed[i].z * deltaTime;

        // それっぽく回転もさせとく
        frontSidetransform_[i].rotate.x += frontSideSpeed[i].x * deltaTime;
        frontSidetransform_[i].rotate.y += frontSideSpeed[i].y * deltaTime;
        frontSidetransform_[i].rotate.z += frontSideSpeed[i].z * deltaTime;

        frontSideMetroriteObject_[i]->SetTranslate(frontSidetransform_[i].translate);
        frontSideMetroriteObject_[i]->SetRotate(frontSidetransform_[i].rotate);
        frontSideMetroriteObject_[i]->Update();
    }

    // 奥側の更新
    for (int i = 0; i < maxBackSideMetrorite; i++) {
        Vector3 cameraPos = camera_->GetTranslate();
        if (ChangePos.x + cameraPos.x >= backSidetransform_[i].translate.x) {
            backSidetransform_[i].translate = {
                randomFloat(randomeBackSidePosMin.x + cameraPos.x, randomeBackSidePosMax.x + cameraPos.x),
                randomFloat(randomeBackSidePosMin.y + cameraPos.y, randomeBackSidePosMax.y + cameraPos.y),
                randomFloat(randomeBackSidePosMin.z + cameraPos.z, randomeBackSidePosMax.z + cameraPos.z)
            };
        }

        backSidetransform_[i].translate.x += backSideSpeed[i].x * deltaTime;
        backSidetransform_[i].translate.y += backSideSpeed[i].y * deltaTime;
        backSidetransform_[i].translate.z += backSideSpeed[i].z * deltaTime;

        // それっぽく回転もさせとく
        backSidetransform_[i].rotate.x += backSideSpeed[i].x * deltaTime;
        backSidetransform_[i].rotate.y += backSideSpeed[i].y * deltaTime;
        backSidetransform_[i].rotate.z += backSideSpeed[i].z * deltaTime;

        backSideMetroriteObject_[i]->SetTranslate(backSidetransform_[i].translate);
        backSideMetroriteObject_[i]->SetRotate(backSidetransform_[i].rotate);
        backSideMetroriteObject_[i]->Update();
    }
}

void Meteorite::Draw()
{
    for (int i = 0; i < maxFrontSideMetrorite; i++) {
        frontSideMetroriteObject_[i]->Draw();
    }
    for (int i = 0; i < maxBackSideMetrorite; i++) {
        backSideMetroriteObject_[i]->Draw();
    }
}
