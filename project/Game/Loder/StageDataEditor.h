#pragma once
#include "../../Engine/base/Math.h"
#include "../../resources/nlohmann/json.hpp"
#include <string>
#include <vector>

class EnemyManager;
class stageObjectManager;
class StageManager;

class StageDataEditor {
public:
    StageDataEditor() = default;
    ~StageDataEditor() = default;

    /// <summary>
    /// 初期化（対象のマネージャー参照をバインド）
    /// </summary>
    void Initialize(EnemyManager* enemyManager, stageObjectManager* stageObjManager, StageManager* stageManager);

    /// <summary>
    /// ImGui描画・編集ウィンドウ
    /// </summary>
    void DrawImGui();

private:
    // JSONファイル読み込み / 保存
    void LoadEnemyJson(const std::string& filePath);
    void SaveEnemyJson(const std::string& filePath);

    void LoadObjectJson(const std::string& filePath);
    void SaveObjectJson(const std::string& filePath);

    void LoadRailJson(const std::string& filePath);
    void SaveRailJson(const std::string& filePath);

    // ImGui内部描画サブメソッド
    void DrawEnemyEditor();
    void DrawObjectEditor();
    void DrawRailEditor();

private:
    EnemyManager* enemyManager_ = nullptr;
    stageObjectManager* stageObjManager_ = nullptr;
    StageManager* stageManager_ = nullptr;

    std::string currentEnemyFilePath_ = "resources/StageData/enemyPopData1.json";
    std::string currentObjectFilePath_ = "resources/StageData/stageObjectData1.json";
    std::string currentRailFilePath_ = "resources/StageData/stageData1.json";

    // 編集用一時データ構造（nlohmann::json との相互変換用）
    nlohmann::json rawEnemyJson_;
    nlohmann::json rawObjectJson_;
    nlohmann::json rawRailJson_;

    bool isEnemyJsonLoaded_ = false;
    bool isObjectJsonLoaded_ = false;
    bool isRailJsonLoaded_ = false;
};
