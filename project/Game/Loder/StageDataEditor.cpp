#include "StageDataEditor.h"
#include "../Enemy/EnemyManager.h"
#include "../Enemy/base/baseEnemy.h"
#include "../stage/StageManager.h"
#include "../stage/stageObjectManager.h"
#include <fstream>
#include <iostream>

#ifdef USE_IMGUI
#include "../externals/imgui/imgui.h"
#endif

using json = nlohmann::json;

void StageDataEditor::Initialize(EnemyManager* enemyManager, stageObjectManager* stageObjManager, StageManager* stageManager)
{
    enemyManager_ = enemyManager;
    stageObjManager_ = stageObjManager;
    stageManager_ = stageManager;

    LoadEnemyJson(currentEnemyFilePath_);
    LoadObjectJson(currentObjectFilePath_);

    if (stageManager_) {
        currentRailFilePath_ = stageManager_->GetJsonFilePath();
    }
    LoadRailJson(currentRailFilePath_);
}

void StageDataEditor::DrawImGui()
{
#ifdef USE_IMGUI
    if (ImGui::Begin("Stage Data Editor (JSON)")) {

        if (ImGui::BeginTabBar("EditorTabs")) {
            if (ImGui::BeginTabItem("Enemy Pop Data")) {
                DrawEnemyEditor();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Stage Object Data")) {
                DrawObjectEditor();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("rail Data")) {
                DrawRailEditor();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
#endif
}

void StageDataEditor::LoadEnemyJson(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
        return;

    try {
        file >> rawEnemyJson_;
        isEnemyJsonLoaded_ = true;
        currentEnemyFilePath_ = filePath;
    } catch (const json::parse_error& e) {
        std::cerr << "JSON Parse Error: " << e.what() << std::endl;
        isEnemyJsonLoaded_ = false;
    }
}

void StageDataEditor::SaveEnemyJson(const std::string& filePath)
{
    std::ofstream file(filePath);
    if (!file.is_open())
        return;

    // spawn_z でソートしてから保存
    if (rawEnemyJson_.contains("enemies") && rawEnemyJson_["enemies"].is_array()) {
        auto& enemies = rawEnemyJson_["enemies"];
        std::sort(enemies.begin(), enemies.end(), [](const json& a, const json& b) {
            return a.value("spawn_z", 0.0f) < b.value("spawn_z", 0.0f);
        });
    }

    file << rawEnemyJson_.dump(4); // インデント4で保存
    std::cout << "Successfully saved: " << filePath << std::endl;

    // マネージャーへ即時リロード反映
    if (enemyManager_) {
        enemyManager_->ReloadJson();
    }
}

void StageDataEditor::LoadObjectJson(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
        return;

    try {
        file >> rawObjectJson_;
        isObjectJsonLoaded_ = true;
        currentObjectFilePath_ = filePath;
    } catch (const json::parse_error& e) {
        std::cerr << "JSON Parse Error: " << e.what() << std::endl;
        isObjectJsonLoaded_ = false;
    }
}

void StageDataEditor::SaveObjectJson(const std::string& filePath)
{
    std::ofstream file(filePath);
    if (!file.is_open())
        return;

    if (rawObjectJson_.contains("Object") && rawObjectJson_["Object"].is_array()) {
        auto& objects = rawObjectJson_["Object"];
        std::sort(objects.begin(), objects.end(), [](const json& a, const json& b) {
            return a.value("spawn_z", 0.0f) < b.value("spawn_z", 0.0f);
        });
    }

    file << rawObjectJson_.dump(4);
    std::cout << "Successfully saved: " << filePath << std::endl;

    if (stageObjManager_) {
        stageObjManager_->ReloadJson();
    }
}

void StageDataEditor::LoadRailJson(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
        return;

    try {
        file >> rawRailJson_;
        isRailJsonLoaded_ = true;
        currentRailFilePath_ = filePath;
    } catch (const json::parse_error& e) {
        std::cerr << "Rail JSON Parse Error: " << e.what() << std::endl;
        isRailJsonLoaded_ = false;
    }
}

void StageDataEditor::SaveRailJson(const std::string& filePath)
{
    std::ofstream file(filePath);
    if (!file.is_open())
        return;

    // Z座標昇順でソートして書き出し
    if (rawRailJson_.contains("control_points") && rawRailJson_["control_points"].is_array()) {
        auto& points = rawRailJson_["control_points"];
        std::sort(points.begin(), points.end(), [](const json& a, const json& b) {
            return a.value("z", 0.0f) < b.value("z", 0.0f);
        });
    }

    file << rawRailJson_.dump(4);
    std::cout << "Successfully saved rail data: " << filePath << std::endl;

    if (stageManager_) {
        stageManager_->ReloadJson(); // StageManagerへ即時反映
    }
}

void StageDataEditor::DrawEnemyEditor()
{
#ifdef USE_IMGUI
    static char pathBuffer[256] = "resources/StageData/enemyPopData1.json";
    ImGui::InputText("File Path##Enemy", pathBuffer, sizeof(pathBuffer));

    if (ImGui::Button("Load JSON##Enemy")) {
        LoadEnemyJson(pathBuffer);
    }
    ImGui::SameLine();
    if (ImGui::Button("Save JSON##Enemy")) {
        SaveEnemyJson(pathBuffer);
    }

    ImGui::Separator();

    if (!isEnemyJsonLoaded_ || !rawEnemyJson_.contains("enemies"))
        return;

    auto& enemies = rawEnemyJson_["enemies"];

    if (ImGui::Button("+ Add New Enemy")) {
        json newEnemy = {
            { "type", "FixedEnemy" },
            { "spawn_z", 0.0f },
            { "pos", { { "x", 0.0f }, { "y", 0.0f }, { "z", 40.0f } } },
            { "UseBullet", "TargetBullet" },
            { "hp", 5 }
        };
        enemies.push_back(newEnemy);
    }

    int removeIndex = -1;
    for (int i = 0; i < static_cast<int>(enemies.size()); ++i) {
        ImGui::PushID(i);
        auto& item = enemies[i];

        std::string headerLabel = "Enemy [" + std::to_string(i) + "] : " + item.value("type", "Unknown");
        if (ImGui::TreeNode(headerLabel.c_str())) {

            // 敵タイプの選択
            const char* enemyTypes[] = { "FixedEnemy", "NormalMoveEnemy", "CircleMoveEnemy", "threeShotsEnemy", "FourEyesBoss", "LaserEnemy" };
            std::string currentType = item.value("type", "FixedEnemy");
            int currentTypeIdx = 0;
            for (int t = 0; t < IM_ARRAYSIZE(enemyTypes); ++t) {
                if (currentType == enemyTypes[t]) {
                    currentTypeIdx = t;
                    break;
                }
            }
            if (ImGui::Combo("Type", &currentTypeIdx, enemyTypes, IM_ARRAYSIZE(enemyTypes))) {
                item["type"] = enemyTypes[currentTypeIdx];
            }

            // Spawn Z
            float spawnZ = item.value("spawn_z", 0.0f);
            if (ImGui::DragFloat("Spawn Z", &spawnZ, 1.0f)) {
                item["spawn_z"] = spawnZ;
            }

            // Position
            float pos[3] = {
                item["pos"].value("x", 0.0f),
                item["pos"].value("y", 0.0f),
                item["pos"].value("z", 0.0f)
            };
            if (ImGui::DragFloat3("Position", pos, 0.5f)) {
                item["pos"]["x"] = pos[0];
                item["pos"]["y"] = pos[1];
                item["pos"]["z"] = pos[2];
            }

            // HP
            int hp = item.value("hp", 5);
            if (ImGui::InputInt("HP", &hp)) {
                item["hp"] = hp;
            }

            // Bullet Type
            const char* bulletTypes[] = { "TargetBullet", "HomingBullet" };
            std::string currentBullet = item.value("UseBullet", "TargetBullet");
            int currentBulletIdx = (currentBullet == "HomingBullet") ? 1 : 0;
            if (ImGui::Combo("Use Bullet", &currentBulletIdx, bulletTypes, IM_ARRAYSIZE(bulletTypes))) {
                item["UseBullet"] = bulletTypes[currentBulletIdx];
            }

            if (currentBulletIdx == 1) {
                float homing = item.value("homingPower", 0.1f);
                if (ImGui::DragFloat("Homing Power", &homing, 0.01f, 0.0f, 1.0f)) {
                    item["homingPower"] = homing;
                }
            }

            // Move Direction (Fixed以外)
            if (currentType != "FixedEnemy") {
                float moveDir[3] = {
                    item["MoveDirection"].value("x", 0.0f),
                    item["MoveDirection"].value("y", 0.0f),
                    item["MoveDirection"].value("z", 1.0f)
                };
                if (ImGui::DragFloat3("Move Direction", moveDir, 0.1f)) {
                    item["MoveDirection"]["x"] = moveDir[0];
                    item["MoveDirection"]["y"] = moveDir[1];
                    item["MoveDirection"]["z"] = moveDir[2];
                }
            }

            if (ImGui::Button("Delete Enemy")) {
                removeIndex = i;
            }

            ImGui::TreePop();
        }
        ImGui::PopID();
    }

    if (removeIndex >= 0) {
        enemies.erase(removeIndex);
    }
#endif
}

void StageDataEditor::DrawObjectEditor()
{
#ifdef USE_IMGUI
    static char pathBuffer[256] = "resources/StageData/stageObjectData1.json";
    ImGui::InputText("File Path##Object", pathBuffer, sizeof(pathBuffer));

    if (ImGui::Button("Load JSON##Object")) {
        LoadObjectJson(pathBuffer);
    }
    ImGui::SameLine();
    if (ImGui::Button("Save JSON##Object")) {
        SaveObjectJson(pathBuffer);
    }

    ImGui::Separator();

    if (!isObjectJsonLoaded_ || !rawObjectJson_.contains("Object"))
        return;

    auto& objects = rawObjectJson_["Object"];

    if (ImGui::Button("+ Add New Object")) {
        json newObj = {
            { "ObjectPatan", "stageCube1" },
            { "spawn_z", 0.0f },
            { "pos", { { "x", 0.0f }, { "y", 0.0f }, { "z", 40.0f } } },
            { "Scale", { { "x", 1.0f }, { "y", 1.0f }, { "z", 1.0f } } }
        };
        objects.push_back(newObj);
    }

    int removeIndex = -1;
    for (int i = 0; i < static_cast<int>(objects.size()); ++i) {
        ImGui::PushID(i);
        auto& item = objects[i];

        std::string headerLabel = "Object [" + std::to_string(i) + "] : " + item.value("ObjectPatan", "Unknown");
        if (ImGui::TreeNode(headerLabel.c_str())) {

            // ObjectPatan
            char patanBuf[128];
            std::string currentPatan = item.value("ObjectPatan", "stageCube1");
            strcpy_s(patanBuf, currentPatan.c_str());
            if (ImGui::InputText("Object Patan", patanBuf, sizeof(patanBuf))) {
                item["ObjectPatan"] = std::string(patanBuf);
            }

            // Spawn Z
            float spawnZ = item.value("spawn_z", 0.0f);
            if (ImGui::DragFloat("Spawn Z", &spawnZ, 1.0f)) {
                item["spawn_z"] = spawnZ;
            }

            // Position
            float pos[3] = {
                item["pos"].value("x", 0.0f),
                item["pos"].value("y", 0.0f),
                item["pos"].value("z", 0.0f)
            };
            if (ImGui::DragFloat3("Position", pos, 0.5f)) {
                item["pos"]["x"] = pos[0];
                item["pos"]["y"] = pos[1];
                item["pos"]["z"] = pos[2];
            }

            // Scale
            float scale[3] = {
                item["Scale"].value("x", 1.0f),
                item["Scale"].value("y", 1.0f),
                item["Scale"].value("z", 1.0f)
            };
            if (ImGui::DragFloat3("Scale", scale, 0.1f)) {
                item["Scale"]["x"] = scale[0];
                item["Scale"]["y"] = scale[1];
                item["Scale"]["z"] = scale[2];
            }

            if (ImGui::Button("Delete Object")) {
                removeIndex = i;
            }

            ImGui::TreePop();
        }
        ImGui::PopID();
    }

    if (removeIndex >= 0) {
        objects.erase(removeIndex);
    }
#endif
}

void StageDataEditor::DrawRailEditor()
{
#ifdef USE_IMGUI
    static char pathBuffer[256] = "resources/StageData/stageData1.json";
    ImGui::InputText("File Path##Rail", pathBuffer, sizeof(pathBuffer));

    if (ImGui::Button("Load JSON##Rail")) {
        LoadRailJson(pathBuffer);
    }
    ImGui::SameLine();
    if (ImGui::Button("Save JSON##Rail")) {
        SaveRailJson(pathBuffer);
    }

    ImGui::Separator();

    //  1. レールの進行・一時停止制御 UI
    if (stageManager_) {
        ImGui::Text("[ Rail Playback Control ]");
        bool isPaused = stageManager_->IsPaused();
        if (ImGui::Checkbox("Pause Rail Motion", &isPaused)) {
            stageManager_->SetPause(isPaused);
        }

        float currentZ = stageManager_->GetCurrentZ();
        if (ImGui::DragFloat("Current Z Position", &currentZ, 1.0f)) {
            stageManager_->SetCurrentZ(currentZ);
        }
        ImGui::Separator();
    }

    if (!isRailJsonLoaded_ || !rawRailJson_.contains("control_points"))
        return;

    auto& points = rawRailJson_["control_points"];

    // 2. 制御点の追加機能
    if (ImGui::Button("+ Add New Control Point")) {
        float lastZ = 0.0f;
        if (!points.empty()) {
            lastZ = points.back().value("z", 0.0f) + 50.0f;
        }
        json newPoint = { { "x", 0.0f }, { "y", 0.0f }, { "z", lastZ } };
        points.push_back(newPoint);
    }

    int removeIndex = -1;

    // 3. 制御点ごとの編集・削除 UI
    for (int i = 0; i < static_cast<int>(points.size()); ++i) {
        ImGui::PushID(i);
        auto& item = points[i];

        std::string headerLabel = "Point [" + std::to_string(i) + "] Z: " + std::to_string(item.value("z", 0.0f));
        if (ImGui::TreeNode(headerLabel.c_str())) {

            float pos[3] = {
                item.value("x", 0.0f),
                item.value("y", 0.0f),
                item.value("z", 0.0f)
            };

            if (ImGui::DragFloat3("Position (X, Y, Z)", pos, 0.5f)) {
                item["x"] = pos[0];
                item["y"] = pos[1];
                item["z"] = pos[2];
            }

            if (ImGui::Button("Delete Point")) {
                removeIndex = i;
            }

            ImGui::TreePop();
        }
        ImGui::PopID();
    }

    if (removeIndex >= 0) {
        points.erase(removeIndex);
    }
#endif
}
