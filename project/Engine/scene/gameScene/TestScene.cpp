#include "TestScene.h"

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

#ifdef USE_IMGUI
#include "../externals/imgui/imgui.h"
#endif // USE_IMGUI

TestScene::TestScene()
{
}

TestScene::~TestScene() = default;

void TestScene::Initialize()
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
    TextureManager::getInstance()->LoadTexture("resources/circle2.png");

    ModelManager::GetInstance()->LoadModel("axis.obj");
    ModelManager::GetInstance()->LoadModel("terrain.obj");
    ModelManager::GetInstance()->LoadModel("plane.gltf");
    ModelManager::GetInstance()->LoadModel("AnimatedCube.gltf");
    ModelManager::GetInstance()->LoadModel("simpleSkin/simpleSkin.gltf");
    ModelManager::GetInstance()->LoadModel("human/walk.gltf");
    ModelManager::GetInstance()->LoadModel("human/sneakWalk.gltf");

    skydox = std::make_unique<Skybox>();
    // skydox->Initialize("resources/rostock_laage_airport_4k.dds");

    object3d = std::make_unique<Object3d>();
    object3d->Initialize();

    model = std::make_unique<Model>();
    model->Initialize("resources", "terrain.obj");
    // model->SetEvnTexturefilePath(skydox->GetTextureFilePath());
    object3d->SetModel(model.get());

    object3d_2 = std::make_unique<Object3d>();
    object3d_2->Initialize();

    model_2 = std::make_unique<Model>();
    model_2->Initialize("resources", "human/walk.gltf");
    object3d_2->SetScale({ 100.0f, 100.0f, 100.0f });
    object3d_2->SetRotate({ -1.5f, 3.14f, 0.0f });
    // model_2->SetEvnTexturefilePath(skydox->GetTextureFilePath());
    object3d_2->SetModel(model_2.get()); // スケルトンもセットで構築
    object3d_2->LoadAnimation("resources", "human/walk.gltf", "walk");
    object3d_2->LoadAnimation("resources", "human/sneakWalk.gltf", "sneakWalk");
    object3d_2->PlayAnimation("sneakWalk", true);

    CPUParticleManager::getInstance()->CreateParticleGroup("pori", "resources/circle2.png", ParticleMeshType::kPlane);
    CPUParticleManager::getInstance()->CreateParticleGroup("circle3", "resources/circle3.png", ParticleMeshType::kPlane);
    CPUParticleManager::getInstance()->CreateParticleGroup("Plane", "resources/uvChecker.png", ParticleMeshType::kPlane);
    CPUParticleManager::getInstance()->CreateParticleGroup("gradationLine", "resources/gradationLine.png", ParticleMeshType::kRing);
    CPUParticleManager::getInstance()->CreateParticleGroup("Cylinder", "resources/gradationLine.png", ParticleMeshType::kCylinder);

    // ポストエフェクトのON/OFFならこれ。
    PostProcess::GetInstance()->SetEnableBoxFilter(false);
    PostProcess::GetInstance()->SetKernelSizeBoxFilter(7);

    // 板ポリ
    Transform emitter { };
    emitter.translate = { 0.0f, 2.0f, 0.0f };
    emitter.rotate = { 0.0f, 0.0f, 1.0f };
    emitter.scale = { 0.05f, 1.0f, 1.0f };
    EmitterParam fireParam;

    // particleEmitter2 = std::make_unique<ParticleEmitter>("gradationLine", emitter, 0.8f, 3, true);
    // fireParam.maxRotate = { std::numbers::pi_v<float>, std::numbers::pi_v<float>, 0.0f };
    // fireParam.minRotate = { -std::numbers::pi_v<float>, -std::numbers::pi_v<float>, 0.0f };
    // fireParam.maxScale = { 1.0f, 1.0f, 1.0f };
    // fireParam.minScale = { 1.0f, 0.4f, 1.0f };
    // fireParam.SetStartColor({ 1.0f, 1.0f, 0.5f, 1.0f });
    // fireParam.SetEndColor({ 1.0f, 1.0f, 1.0f, 0.0f });
    // fireParam.SetVelocity({ 0.0f, 0.0f, 0.0f });
    // fireParam.SetLifeTime(1.2f);
    // particleEmitter2->SetParam(fireParam);

    /*emitter.translate = { 0.0f, 0.0f, 0.0f };
    particleEmitter4 = std::make_unique<ParticleEmitter>("Cylinder", emitter, 10.0f, 1, false);
    fireParam.SetScale({ 1.0f, 1.0f, 1.0f });
    fireParam.SetRotate({ 0.0f, 0.0f, 0.0f });
    fireParam.SetStartColor({ 1.0f, 1.0f, 1.0f, 1.0f });
    fireParam.SetEndColor({ 1.0f, 1.0f, 1.0f, 1.0f });
    fireParam.SetVelocity({ 0.0f, 0.0f, 0.0f });
    fireParam.SetLifeTime(10.0f);
    fireParam.isInfinite = true;
    particleEmitter4->SetParam(fireParam);
    ParticleManager::getInstance()->SetGroupScrollSpeed("Cylinder", { 0.2f, 0.0f });*/

    player_ = std::make_unique<Player>();
    player_->Initialize();

    // hitParticle = std::make_unique<HitParticle>();
    // hitParticle->Initialize();
    // hitParticle->NewTransform();

    auto* gpuParticleMgr = GPUParticleManager::getInstance();

    gpuParticleMgr->CreateParticleGroup(
        "TestRandomGroup", "resources/circle2.png", ParticleMeshType::kPlane, kBlendModeAdd, Vector4(1.0f, 1.0f, 1.0f, 1.0f));

    // 2. エミッターの生成
    testGpuEmitterHandle_ = gpuParticleMgr->AddEmitter(
        "TestRandomGroup", testEmitterPos_, 0.5f, emitCount_, 0.05f, emitTimer_, gravity_, acceleration_, airResistance_);

    // 3. テスト条件の直接セット (要求仕様の実装)
    EmitterSphereData* emitterData = gpuParticleMgr->GetEmitterData(testGpuEmitterHandle_);
    if (emitterData) {
        // [ランダム生成設定] スケール・回転・速度の幅をセット
        emitterData->minVelocity = Vector3(-2.0f, 1.0f, -2.0f);
        emitterData->maxVelocity = Vector3(2.0f, 4.0f, 2.0f);
        emitterData->minSpeed = 0.8f;
        emitterData->maxSpeed = 2.5f;

        emitterData->minStartScale = Vector3(0.2f, 0.2f, 0.2f);
        emitterData->maxStartScale = Vector3(0.8f, 0.8f, 0.8f);
        emitterData->minEndScale = Vector3(0.0f, 0.0f, 0.0f); // 徐々に小さく
        emitterData->maxEndScale = Vector3(0.0f, 0.0f, 0.0f);

        emitterData->minStartRotation = Vector3(0.0f, 0.0f, -3.1415f);
        emitterData->maxStartRotation = Vector3(0.0f, 0.0f, 3.1415f);
        emitterData->minEndRotation = Vector3(0.0f, 0.0f, -6.2831f);
        emitterData->maxEndRotation = Vector3(0.0f, 0.0f, 6.2831f);

        // [時間経過グラデーション設定] lifeTime 1.0f (誕生時) = 赤 / 0.0f (死亡時) = 透明
        emitterData->minStartColor = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
        emitterData->maxStartColor = Vector4(1.0f, 0.2f, 0.0f, 1.0f);
        emitterData->minEndColor = Vector4(1.0f, 0.0f, 0.0f, 0.0f);
        emitterData->maxEndColor = Vector4(1.0f, 0.2f, 0.0f, 0.0f);
    }
}

void TestScene::Finalize()
{
}

void TestScene::Update()
{

    auto* input = Input::GetInstance();
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

    if (ImGui::Begin("Particle Random & Gradient Test Inspector")) {
        auto* gpuParticleMgr = GPUParticleManager::getInstance();
        EmitterSphereData* emitterData = gpuParticleMgr->GetEmitterData(testGpuEmitterHandle_);

        if (emitterData) {
            if (ImGui::CollapsingHeader("1. Emitter Basic Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
                if (ImGui::DragFloat3("Position (Translate)", &testEmitterPos_.x, 0.1f)) {
                    gpuParticleMgr->SetEmitterPosition(testGpuEmitterHandle_, testEmitterPos_);
                }

                const char* emitTypes[] = { "Sphere (Sphere Radius)", "Box (AABB Offset Range)" };
                int currentType = static_cast<int>(emitterData->emitType);
                if (ImGui::Combo("Emit Shape Type", &currentType, emitTypes, IM_ARRAYSIZE(emitTypes))) {
                    emitterData->emitType = static_cast<uint32_t>(currentType);
                }

                if (emitterData->emitType == 0) {
                    ImGui::DragFloat("Sphere Radius", &emitterData->radius, 0.1f, 0.0f, 50.0f);
                } else {
                    ImGui::DragFloat3("Min Emit Offset", &emitterData->minEmitOffset.x, 0.1f);
                    ImGui::DragFloat3("Max Emit Offset", &emitterData->maxEmitOffset.x, 0.1f);
                }

                int count = static_cast<int>(emitterData->count);
                if (ImGui::SliderInt("Emit Count", &count, 1, 512)) {
                    emitterData->count = static_cast<uint32_t>(count);
                }
                ImGui::DragFloat("Frequency Interval", &emitterData->frequency, 0.01f, 0.001f, 5.0f);

                if (ImGui::SliderFloat("LifeTime", &emitTimer_, 0.1f, 10.0f)) {
                    gpuParticleMgr->SetEmitterParticleLifeTime(testGpuEmitterHandle_, emitTimer_);
                }
            }
            if (ImGui::CollapsingHeader("2. Physics Parameters", ImGuiTreeNodeFlags_DefaultOpen)) {
                bool physicsChanged = false;
                physicsChanged |= ImGui::DragFloat3("Gravity", &gravity_.x, 0.1f);
                physicsChanged |= ImGui::DragFloat3("Acceleration", &acceleration_.x, 0.1f);
                physicsChanged |= ImGui::SliderFloat("Air Resistance", &airResistance_, 0.0f, 1.0f);

                if (physicsChanged) {
                    gpuParticleMgr->SetEmitterPhysics(testGpuEmitterHandle_, gravity_, acceleration_, airResistance_);
                }
            }
            if (ImGui::CollapsingHeader("3. Velocity & Speed (Random Range)")) {
                ImGui::DragFloat3("Min Velocity Dir", &emitterData->minVelocity.x, 0.1f);
                ImGui::DragFloat3("Max Velocity Dir", &emitterData->maxVelocity.x, 0.1f);
                ImGui::SliderFloat("Min Speed", &emitterData->minSpeed, 0.0f, 20.0f);
                ImGui::SliderFloat("Max Speed", &emitterData->maxSpeed, 0.0f, 20.0f);
            }
            if (ImGui::CollapsingHeader("4. Scale (Start -> End)")) {
                ImGui::Text("[Start Scale]");
                ImGui::DragFloat3("Min Start Scale", &emitterData->minStartScale.x, 0.05f, 0.0f, 10.0f);
                ImGui::DragFloat3("Max Start Scale", &emitterData->maxStartScale.x, 0.05f, 0.0f, 10.0f);

                ImGui::Separator();
                ImGui::Text("[End Scale]");
                ImGui::DragFloat3("Min End Scale", &emitterData->minEndScale.x, 0.05f, 0.0f, 10.0f);
                ImGui::DragFloat3("Max End Scale", &emitterData->maxEndScale.x, 0.05f, 0.0f, 10.0f);
            }
            if (ImGui::CollapsingHeader("5. Rotation (Start -> End)")) {
                ImGui::Text("[Start Rotation (Rad)]");
                ImGui::DragFloat3("Min Start Rotation", &emitterData->minStartRotation.x, 0.05f, -6.28f, 6.28f);
                ImGui::DragFloat3("Max Start Rotation", &emitterData->maxStartRotation.x, 0.05f, -6.28f, 6.28f);

                ImGui::Separator();
                ImGui::Text("[End Rotation (Rad)]");
                ImGui::DragFloat3("Min End Rotation", &emitterData->minEndRotation.x, 0.05f, -6.28f, 6.28f);
                ImGui::DragFloat3("Max End Rotation", &emitterData->maxEndRotation.x, 0.05f, -6.28f, 6.28f);
            }
            if (ImGui::CollapsingHeader("6. Color & Gradient (Start -> End)")) {
                ImGui::Text("[Start Color (Life 1.0)]");
                ImGui::ColorEdit4("Min Start Color", &emitterData->minStartColor.x);
                ImGui::ColorEdit4("Max Start Color", &emitterData->maxStartColor.x);

                ImGui::Separator();
                ImGui::Text("[End Color (Life 0.0)]");
                ImGui::ColorEdit4("Min End Color", &emitterData->minEndColor.x);
                ImGui::ColorEdit4("Max End Color", &emitterData->maxEndColor.x);
            }
            ImGui::Separator();
            if (ImGui::Button("Trigger Test Burst (One-Shot)", ImVec2(-1, 30))) {
                gpuParticleMgr->TriggerOneShot(testGpuEmitterHandle_, emitterData->count);
            }
        }
        ImGui::End();
    }
#endif // USE_IMGUI

    // skydox->Update();

    object3d->Update();
    object3d_2->Update();

    player_->Update();

    // particleEmitter2->Update();
    // particleEmitter4->Update();

    // 5秒ごとに生成(軽いテストなう)
    float deltaTime = SceneManager::GetInstance()->GetDeltaTime();
    testTimer += deltaTime;

    // hitParticle->Update();

    // IMGUI
    object3d->DrawImGui("Terrain");
    object3d_2->DrawImGui("Plane");
}

void TestScene::Draw()
{
    //
    // モデルデータ
    //
    Object3dCommon::GetInstance()->PrepareObjectDraw();

    object3d->Draw();
    object3d_2->Draw();
    player_->Draw();

#ifdef USE_IMGUI
    object3d_2->DrawSkeleton();
#endif // USE_IMGUI

    CPUParticleManager::getInstance()->Draw();

    GPUParticleManager::getInstance()->Draw();

    SkyBoxCommon::GetInstance()->PrepareObjectDraw();
    // skydox->Draw();

    SpriteCommon::GetInstance()->PrepareSpriteDraw();
}
