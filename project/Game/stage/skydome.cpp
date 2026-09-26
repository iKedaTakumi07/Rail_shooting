#include "skydome.h"

#include "../../Engine/3d/CameraManager.h"
#include "../../Engine/3d/ModelManager.h"
#include "../../Engine/3d/Object3d.h"
#include "../../Engine/base/Math.h"
#include "../../Engine/base/TextureManager.h"

void skydome::Initialize()
{
    TextureManager::getInstance()->LoadTexture("resources/skydone/sky_sphere.png");
    ModelManager::GetInstance()->LoadModel("skydone/skydome.obj");

    camera_ = CameraManager::GetInstance()->GetActiveCamera();

    transform_.translate = { 0.0f, 0.0f, 0.0f };
    transform_.scale = { 1.0f, 1.0f, 1.0f };
    transform_.rotate = { 0.0f, 0.0f, 0.0f };

    Object3d_ = std::make_unique<Object3d>();
    Object3d_->Initialize();

    ObjectModel = std::make_unique<Model>();
    ObjectModel->Initialize("resources/skydone", "skydome.obj"); // 指定したパターンに
    Object3d_->SetModel(ObjectModel.get());

    Object3d_->SetTranslate(transform_.translate);
    Object3d_->SetScale(transform_.scale);
    Object3d_->SetRotate(transform_.rotate);
}

void skydome::Update()
{
    // 天球の外に出ないように調整
    Vector3 pos = camera_->GetTranslate();

    Object3d_->SetTranslate(pos);
    Object3d_->Update();
}

void skydome::Draw()
{
    Object3d_->Draw();
}
