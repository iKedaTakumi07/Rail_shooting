#include "StageManager.h"
#include "../../Engine/scene/SceneManager.h"
#include "../../resources/nlohmann/json.hpp"
#include <algorithm>

#include <fstream>
#include <iostream>

using json = nlohmann::json;

void StageManager::Initialize(const std::string& filePath)
{
    jsonFilePath_ = filePath;
    isBossCutscene_ = false;
    isBossBattle_ = false;
    currentZ_ = -kScrollSpeed * 5.0f; // 進行速度の5秒分

    LoadStageData(jsonFilePath_);
}

void StageManager::Update()
{
    if (isBossCutscene_) {
        return;
    }

    float deltaTime = SceneManager::GetInstance()->GetDeltaTime();

    currentZ_ += kScrollSpeed * deltaTime;
}

void StageManager::Draw3D()
{
    // いらぬ
}

Vector3 StageManager::CalcRailPosition()
{
    auto& points = stageData_.controlPoints;

    // エラー、ポイント頭がなかったら指定の座標に固定
    if (points.empty())
        return { 0.0f, 0.0f, currentZ_ };
    if (points.size() == 1)
        return { points[0].x, points[0].y, currentZ_ };

    if (currentZ_ <= points[0].z) {
        return { points[0].x, points[0].y, currentZ_ };
    }

    for (size_t i = 0; i < points.size() - 1; ++i) {
        const Vector3& p0 = points[i];
        const Vector3& p1 = points[i + 1];

        // p0 と p1 の間にいる場合
        if (currentZ_ >= p0.z && currentZ_ <= p1.z) {
            // Z座標の進行割合を計算
            float t = (currentZ_ - p0.z) / (p1.z - p0.z);

            // XとYを線形補間(Lerp)
            Vector3 result;
            result.x = p0.x + (p1.x - p0.x) * t;
            result.y = p0.y + (p1.y - p0.y) * t;
            result.z = currentZ_;
            return result;
        }
    }

    //
    const Vector3& lastP = points.back();
    return { lastP.x, lastP.y, currentZ_ };
}

void StageManager::LoadStageData(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
        return;

    json data; // ← 注意: ここの変数名も data
    try {
        file >> data;
    } catch (const json::parse_error& e) {
        std::cerr << "JSON Parse Error: " << e.what() << std::endl;
        return;
    }

    stageData_.controlPoints.clear();
    stageData_.stageID = 0;

    if (data.contains("stage_id") && data["stage_id"].is_number()) {
        // ID取得
        stageData_.stageID = data.value("stage_id", 0);
    } else {
        std::cerr << "Warning: 'stage_id' is missing or not a number." << std::endl; // エラー
    }

    // 各ポイントを取得
    if (data.contains("control_points") && data["control_points"].is_array()) {
        for (const auto& pointData : data["control_points"]) {
            if (pointData.is_object()) {
                Vector3 pos;
                pos.x = pointData.value("x", 0.0f);
                pos.y = pointData.value("y", 0.0f);
                pos.z = pointData.value("z", 0.0f);

                // メンバ変数に直接追加
                stageData_.controlPoints.push_back(pos);
            }
        }
    } else {
        std::cerr << "Warning: 'control_points' is missing or not an array." << std::endl; // エラー
    }
}
