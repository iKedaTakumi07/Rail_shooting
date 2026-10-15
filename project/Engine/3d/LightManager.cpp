#define NOMINMAX

#include "LightManager.h"
#include "../base/Math.h"
#include "Object3dCommon.h"
#include <numbers>
#ifdef USE_IMGUI
#include "../externals/imgui/imgui.h"
#endif

LightManager* LightManager::GetInstance()
{
    static LightManager instance;
    return &instance;
}

void LightManager::Initialize()
{
    directionalLightInitialize();
    pointLightInitialize();
    spotLightInitialize();
}

void LightManager::Update()
{
    // 正規化など更新系はここに記述
    directionalLightData_->direction = Normalize(directionalLightData_->direction);

    size_t count = std::min(activePointLights_.size(), kMaxPointLights);
    pointLightData_->numPointLights = static_cast<int32_t>(count);

    for (size_t i = 0; i < kMaxPointLights; ++i) {
        if (i < count && activePointLights_[i] != nullptr) {
            pointLightData_->pointLights[i] = *activePointLights_[i];
            pointLightData_->pointLights[i].active = 1;
        } else {
            pointLightData_->pointLights[i].active = 0;
        }
    }
}

void LightManager::DrawImGui()
{
#ifdef USE_IMGUI
    ImGui::Begin("Light Control");
    // --- Directional Light ---
    if (ImGui::CollapsingHeader("Directional Light")) {
        ImGui::Indent();
        ImGui::SliderFloat3("direction##Directional", &directionalLightData_->direction.x, -1.0f, 1.0f);
        ImGui::ColorEdit4("Color##Directional", &(directionalLightData_->color).x);
        ImGui::DragFloat("intensity##Directional", &directionalLightData_->intensity, 0.01f);
        bool isLighting = (directionalLightData_->active != 0);
        if (ImGui::Checkbox("Lightactive##Directional", &isLighting)) {
            directionalLightData_->active = isLighting ? 1 : 0;
        }
        ImGui::Unindent();
    }

    // --- Point Light(list化中) ---
    if (ImGui::CollapsingHeader("Point Light (Active List)")) {
        ImGui::Indent();
        ImGui::Text("Active Lights: %d / %zu", pointLightData_->numPointLights, kMaxPointLights);
        for (int i = 0; i < pointLightData_->numPointLights; ++i) {
            std::string label = "Light [" + std::to_string(i) + "]";
            if (ImGui::TreeNode(label.c_str())) {
                ImGui::ColorEdit4(("Color##Point" + std::to_string(i)).c_str(), &(pointLightData_->pointLights[i].color).x);
                ImGui::DragFloat3(("Position##Point" + std::to_string(i)).c_str(), &pointLightData_->pointLights[i].position.x, 0.01f);
                ImGui::DragFloat(("Intensity##Point" + std::to_string(i)).c_str(), &pointLightData_->pointLights[i].intensity, 0.01f);
                ImGui::DragFloat(("Radius##Point" + std::to_string(i)).c_str(), &pointLightData_->pointLights[i].radius, 0.01f);
                ImGui::TreePop();
            }
        }
        ImGui::Unindent();
    }

    // --- Spot Light ---
    if (ImGui::CollapsingHeader("Spot Light")) {
        ImGui::Indent();
        ImGui::ColorEdit4("color##Spot", &(spotLightData_->color).x);
        ImGui::DragFloat3("position##Spot", &spotLightData_->position.x, 0.01f);
        ImGui::DragFloat("intensity##Spot", &spotLightData_->intensity, 0.01f);
        ImGui::DragFloat3("direction##Spot", &spotLightData_->direction.x, 0.01f);
        ImGui::DragFloat("distance##Spot", &spotLightData_->distance, 0.01f);
        ImGui::DragFloat("decay##Spot", &spotLightData_->decay, 0.01f);
        ImGui::DragFloat("cosAngle##Spot", &spotLightData_->cosAngle, 0.01f);
        ImGui::DragFloat("cosFalloffStart##Spot", &spotLightData_->cosFalloffStart, 0.01f);
        bool isLighting = (spotLightData_->active != 0);
        if (ImGui::Checkbox("Lightactive##Spot", &isLighting)) {
            spotLightData_->active = isLighting ? 1 : 0;
        }
        ImGui::Unindent();
    }
    ImGui::End();
#endif
}

void LightManager::RegisterPointLight(PointLigth* light)
{
    if (!light)
        return;
    // 重複登録防止
    auto it = std::find(activePointLights_.begin(), activePointLights_.end(), light);
    if (it == activePointLights_.end()) {
        activePointLights_.push_back(light);
    }
}

void LightManager::UnregisterPointLight(PointLigth* light)
{
    if (!light)
        return;
    auto it = std::find(activePointLights_.begin(), activePointLights_.end(), light);
    if (it != activePointLights_.end()) {
        activePointLights_.erase(it);
    }
}

void LightManager::directionalLightInitialize()
{
    auto dxCommon = Object3dCommon::GetInstance()->GetDxCommon();

    directionalLightResource_ = dxCommon->CreateBufferResource(sizeof(DirectionalLight));
    // アドレスを取得
    directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
    // 書き込み
    directionalLightData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    directionalLightData_->direction = { 0.0f, -1.0f, 0.0f };
    directionalLightData_->intensity = 1.0f;
    directionalLightData_->active = 1;
}

void LightManager::pointLightInitialize()
{
    auto dxCommon = Object3dCommon::GetInstance()->GetDxCommon();
    pointLightResource_ = dxCommon->CreateBufferResource(sizeof(PointLightGroup));
    pointLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&pointLightData_));

    pointLightData_->numPointLights = 0;
    for (size_t i = 0; i < kMaxPointLights; ++i) {
        pointLightData_->pointLights[i].active = 0;
    }
}

void LightManager::spotLightInitialize()
{
    auto dxCommon = Object3dCommon::GetInstance()->GetDxCommon();
    spotLightResource_ = dxCommon->CreateBufferResource(sizeof(SpotLigth));

    // mapして書き込み
    spotLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&spotLightData_));
    // 今回は白を書き込んでみる
    spotLightData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    spotLightData_->position = Vector3(0.0f, 2.0f, 0.0f);
    spotLightData_->distance = 1.0f;
    spotLightData_->direction = Normalize({ -1.0f, -1.0f, 0.0f });
    spotLightData_->intensity = 0.0f;
    spotLightData_->decay = 2.0f;
    spotLightData_->cosAngle = std::cos(std::numbers::pi_v<float> / 3.0f);
    spotLightData_->cosFalloffStart = std::cos(std::numbers::pi_v<float> / 4.0f);
    spotLightData_->active = 0;
}
