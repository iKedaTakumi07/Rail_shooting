#pragma once
#include <memory>

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

    /// <summary>
    /// 撃破数の加算
    /// </summary>
    void AddDefeatCount(int count = 1);

    // ゲッター
    int GetDefeatCount() const { return defeatCount_; }

private:
    GameScoreManager() = default;
    ~GameScoreManager() = default;

private:
    static std::unique_ptr<GameScoreManager> instance_;

    int defeatCount_ = 0;
};
