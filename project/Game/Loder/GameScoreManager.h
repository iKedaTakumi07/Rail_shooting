#pragma once
#include <memory>
#include <string>
#include <unordered_map>

class GameScoreManager {
    friend struct std::default_delete<GameScoreManager>;

public:
    static GameScoreManager* GetInstance();

    GameScoreManager(const GameScoreManager&) = delete;
    GameScoreManager& operator=(const GameScoreManager&) = delete;

public:
    /// <summary>
    /// カウントのリセット（ゲーム開始・リトライ時に呼び出す）
    /// </summary>
    void Reset();

    void Update();

    void OnEnemyDefeated();

    /// <summary>
    /// 撃破数の加算
    /// </summary>
    void AddDefeatCount(int count = 1);

    /// <summary>
    /// [追加] 勲章データのJSONファイルを読み込む
    /// </summary>
    /// <param name="filePath">medalData.json のパス</param>
    /// <returns>読み込み成功時 true</returns>
    bool LoadMedalData(const std::string& filePath = "resources/medalData.json");

    // ゲッター
    void SetCurrentStageId(int stageId) { currentStageId_ = stageId; }
    int GetCurrentStageId() const { return currentStageId_; }

    int GetDefeatCount() const { return defeatCount_; }
    int GetSimultaneousKillCount() const { return simultaneousKillCount_; }
    int GetComboGraceFrames() const { return comboGraceFrames_; }

    int GetMedalLine(int stageId) const;
    int GetCurrentMedalLine() const { return GetMedalLine(currentStageId_); }
    bool HasEarnedMedal(int stageId) const;
    bool HasEarnedMedal() const { return HasEarnedMedal(currentStageId_); }

private:
    GameScoreManager() = default;
    ~GameScoreManager() = default;

private:
    static std::unique_ptr<GameScoreManager> instance_;

    int defeatCount_ = 0;

    // 加算ライン

    static constexpr int kMaxComboGraceFrames = 10; // 同時撃破猶予フレーム数
    static constexpr int kBaseKillScore = 1; // 1体撃破あたりの基本スコア
    static constexpr int kMultiKillBonus = 1; // 同時撃破時の追加ボーナス係数
    int comboGraceFrames_ = 0; // 残り猶予フレーム
    int simultaneousKillCount_ = 0; // 猶予内の連続同時撃破数

    // 各ステ―ジの勲章ライン
    std::unordered_map<int, int> medalLines_;
    int currentStageId_ = 1;
};
