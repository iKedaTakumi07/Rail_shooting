#include "meteorite.h"

#include "../../Engine/3d/CameraManager.h"
#include "../../Engine/3d/ModelManager.h"
#include "../../Engine/3d/Object3d.h"
#include "../../Engine/base/Math.h"
#include "../../Engine/base/TextureManager.h"
#include "../../Engine/scene/SceneManager.h"

void meteorite::Initialize()
{
    TextureManager::getInstance()->LoadTexture("resources/skydone/meteorite.png");
    ModelManager::GetInstance()->LoadModel("skydone/metrorite.obj");

    std::random_device seedGenerator;
    randomEngine = std::mt19937(seedGenerator());
    std::uniform_real_distribution<float> DirXdist(-2.0f, -1.0f);
    std::uniform_real_distribution<float> DirYZdist(-1.0f, 1.0f);

    std::uniform_real_distribution<float> PosXdist(20.0f, 40.0f);
    std::uniform_real_distribution<float> PosYdist(-20.0f, 20.0f);
    std::uniform_real_distribution<float> PosZdist(-30.0f, 30.0f);

    camera_ = CameraManager::GetInstance()->GetActiveCamera();

    ObjectModel = std::make_unique<Model>();
    ObjectModel->Initialize("resources/skydone", "metrorite.obj"); // 指定したパターンに

    for (int i = 0; i < maxMetrorite; i++) {
        transform_[i].translate = { PosXdist(randomEngine), PosYdist(randomEngine), PosZdist(randomEngine) };
        transform_[i].rotate = { 0.0f, 0.0f, 0.0f };
        transform_[i].scale = { 1.0f, 1.0f, 1.0f };

        Object3d_[i] = std::make_unique<Object3d>();
        Object3d_[i]->Initialize();
        Object3d_[i]->SetModel(ObjectModel.get());

        Object3d_[i]->SetTranslate(transform_[i].translate);
        Object3d_[i]->SetScale(transform_[i].scale);
        Object3d_[i]->SetRotate(transform_[i].rotate);

        Speed[i] = { DirXdist(randomEngine), DirYZdist(randomEngine), DirYZdist(randomEngine) };
    }
}

void meteorite::Update()
{
    float deltaTime = SceneManager::GetInstance()->GetDeltaTime();

    // 移動ナウ
    for (int i = 0; i < maxMetrorite; i++) {
        if (ChangePos.x >= transform_[i].translate.x) {
            std::uniform_real_distribution<float> PosXdist(20.0f, 40.0f);
            std::uniform_real_distribution<float> PosYdist(-20.0f, 20.0f);
            std::uniform_real_distribution<float> PosZdist(-30.0f, 30.0f);
            transform_[i].translate = { PosXdist(randomEngine), PosYdist(randomEngine), PosZdist(randomEngine) };
        }

        transform_[i].translate.x += Speed[i].x * deltaTime;
        transform_[i].translate.y += Speed[i].y * deltaTime;
        transform_[i].translate.z += Speed[i].z * deltaTime;

        // それっぽく回転もさせとく
        transform_[i].rotate.x += Speed[i].x * deltaTime;
        transform_[i].rotate.y += Speed[i].y * deltaTime;
        transform_[i].rotate.z += Speed[i].z * deltaTime;

        Object3d_[i]->SetTranslate(transform_[i].translate);
        Object3d_[i]->SetRotate(transform_[i].rotate);
        Object3d_[i]->Update();
    }
}

void meteorite::Draw()
{
    for (int i = 0; i < maxMetrorite; i++) {
        Object3d_[i]->Draw();
    }
}
