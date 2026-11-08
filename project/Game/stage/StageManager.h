#include "../../Engine/base/Math.h"
#include <vector>

struct StageData {
    int stageID;
    std::vector<Vector3> controlPoints;
};

class StageManager {
public:
    void Initialize(const std::string& filePath);

    void Update();

    void Draw3D();

    void ReloadJson()
    {
        LoadStageData(jsonFilePath_);
    }

public:
    // Get関数
    float GetCurrentZ() const { return currentZ_; }
    void SetCurrentZ(float z) { currentZ_ = z; }

    bool IsPaused() const { return isPaused_; }
    void SetPause(bool pause) { isPaused_ = pause; }

    StageData& GetStageData() { return stageData_; }
    const std::string& GetJsonFilePath() const { return jsonFilePath_; }

    // レールの中心座標を求める
    Vector3 CalcRailPosition();

private:
    /// <summary>
    /// Json読み込み―
    /// </summary>
    /// <param name="filePath">読み込み対象のファイル</param>
    void LoadStageData(const std::string& filePath);

private:
    // 書き込みのデータ
    std::string jsonFilePath_; // 読み込んでいるJSONのパス
    // ステージ内容
    StageData stageData_;

    // 進行制御用
    float currentZ_ = 0.0f; // 現在の進行Z座標
    const float kScrollSpeed = 20.0f; // 前進する速度(ブースト頭で切り替わるカモメ)

    bool isPaused_ = false; // 特殊停止
    bool isBossCutscene_ = false; // 演出
    bool isBossBattle_ = false; // ボス戦中フラグ
};