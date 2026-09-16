#include "stageObjectManager.h"
#include "../../Engine/3d/CameraManager.h"
#include "../../Engine/3d/ModelManager.h"
#include "../../Engine/3d/Object3d.h"
#include "../../Engine/base/Math.h"
#include "../../Engine/base/TextureManager.h"
#include "../Player/Player.h"

#include "../../resources/nlohmann/json.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

void stageObjectManager::Initialize(const std::string& filePath, Player* player)
{
    TextureManager::getInstance()->LoadTexture("resources/stage/uvChecker.png");
    ModelManager::GetInstance()->LoadModel("stage/stageGraunod.obj");

    PopObjFilePath_ = filePath;
    player_ = player;

    GrauondInitialize();
    LoadJsonPopData(PopObjFilePath_);
}

void stageObjectManager::Update()
{
    if (player_) {
        Vector3 pos = player_->GetTranslate();
        while (currentSpawnIndex_ < popDatas_.size() && pos.z >= popDatas_[currentSpawnIndex_].spawn_Z) {
            PopEnemyCheck(popDatas_[currentSpawnIndex_]);
            currentSpawnIndex_++;
        }
    }

    const float kDespawnDistance = -50.0f; // 破棄する基準となるプレイヤーからの相対距離
    stageObjects_.erase(std::remove_if(stageObjects_.begin(), stageObjects_.end(), [&](const std::unique_ptr<stageObject>& obj) {
        if (player_) {
            float zDiff = obj->GetAllAABB().wholeBox.max.z - player_->GetTranslate().z;
            return zDiff < kDespawnDistance;
        }
        return false;
    }),
        stageObjects_.end());

    for (auto& obj : stageObjects_) {
        obj->Update();
    }

    GrauondUpdate();
}

void stageObjectManager::Draw()
{
    grauond3d->Draw();
    grauond3d2->Draw();
    grauond3d3->Draw();

    for (auto& obj : stageObjects_) {
        obj->Draw();
    }
}

void stageObjectManager::LoadJsonPopData(const std::string& filePath)
{
    std::ifstream file(PopObjFilePath_);
    if (!file.is_open())
        return;

    json data; // ← 注意: ここの変数名も data
    try {
        file >> data;
    } catch (const json::parse_error& e) {
        std::cerr << "JSON Parse Error: " << e.what() << std::endl;
        return;
    }

    popDatas_.clear();
    if (data.contains("Object") && data["Object"].is_array()) {
        for (const auto& objData : data["Object"]) {
            stageObjectPopData popdata;

            // 絶対に代入するもの
            popdata.ObjectPatan = objData.value("ObjectPatan", "stageCube1");
            popdata.spawn_Z = objData.value("spawn_z", 0.0f);

            if (objData.contains("pos")) {
                const auto& posData = objData["pos"];
                popdata.popPosition.x = posData.value("x", 0.0f);
                popdata.popPosition.y = posData.value("y", 0.0f);
                popdata.popPosition.z = posData.value("z", 40.0f);
            } else {
                popdata.popPosition = { 0.0f, 0.0f, 40.0f };
            }

            if (objData.contains("Scale")) {
                const auto& scaleData = objData["Scale"];
                popdata.sizeScale.x = scaleData.value("x", 2.0f);
                popdata.sizeScale.y = scaleData.value("y", 2.0f);
                popdata.sizeScale.z = scaleData.value("z", 2.0f);
            } else {
                popdata.sizeScale = { 1.0f, 1.0f, 1.0f };
            }

            popDatas_.push_back(popdata);
        }
    }

    // sort
    std::sort(popDatas_.begin(), popDatas_.end(), [](const stageObjectPopData& a, const stageObjectPopData& b) {
        return a.spawn_Z < b.spawn_Z;
    });
}

void stageObjectManager::PopEnemyCheck(const stageObjectPopData& data)
{
    auto newObj = std::make_unique<stageObject>();
    newObj->Initialize(data.ObjectPatan, data.popPosition, data.sizeScale);
    stageObjects_.push_back(std::move(newObj));
}

void stageObjectManager::GrauondInitialize()
{
    grauond3d = std::make_unique<Object3d>();
    grauond3d2 = std::make_unique<Object3d>();
    grauond3d3 = std::make_unique<Object3d>();
    grauond3d->Initialize();
    grauond3d2->Initialize();
    grauond3d3->Initialize();

    grauondModel = std::make_unique<Model>();
    grauondModel->Initialize("resources/stage", "stageGraunod.obj");
    grauond3d->SetModel(grauondModel.get());
    grauond3d2->SetModel(grauondModel.get());
    grauond3d3->SetModel(grauondModel.get());
    grauond3d->SetScale(grauondtransform_.scale);
    grauond3d2->SetScale(grauondtransform_.scale);
    grauond3d3->SetScale(grauondtransform_.scale);

    section = 0;
    grauondtransform_.translate.z = 0.0f;
    grauondtransform2_.translate.z = grauondSize;
    grauondtransform3_.translate.z = grauondSize * 2;
}

void stageObjectManager::GrauondUpdate()
{
    Vector3 pos = player_->GetTranslate();
    if (pos.z >= grauondtransform_.translate.z + grauondSize) {
        grauondtransform_.translate.z = grauondtransform3_.translate.z + grauondSize;
    } else if (pos.z >= grauondtransform2_.translate.z + grauondSize) {
        grauondtransform2_.translate.z = grauondtransform_.translate.z + grauondSize;
    } else if (pos.z >= grauondtransform3_.translate.z + grauondSize) {
        grauondtransform3_.translate.z = grauondtransform2_.translate.z + grauondSize;
    }

    grauond3d->SetTranslate(grauondtransform_.translate);
    grauond3d2->SetTranslate(grauondtransform2_.translate);
    grauond3d3->SetTranslate(grauondtransform3_.translate);
    grauond3d->SetRotate(grauondtransform_.rotate);
    grauond3d2->SetRotate(grauondtransform_.rotate);
    grauond3d3->SetRotate(grauondtransform_.rotate);

    grauond3d->Update();
    grauond3d2->Update();
    grauond3d3->Update();
}
