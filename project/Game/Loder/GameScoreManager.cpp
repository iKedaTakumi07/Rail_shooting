#include "GameScoreManager.h"
#include "../../resources/nlohmann/json.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>

using json = nlohmann::json;
std::unique_ptr<GameScoreManager> GameScoreManager::instance_ = nullptr;

GameScoreManager* GameScoreManager::GetInstance()
{
    if (!instance_) {
        // privateコンストラクタのため new を使って unique_ptr を生成
        instance_ = std::unique_ptr<GameScoreManager>(new GameScoreManager());
    }
    return instance_.get();
}

void GameScoreManager::Reset()
{
    // すべて初期化
    defeatCount_ = 0;
    totalScore_ = 0;
    comboGraceFrames_ = 0;
    simultaneousKillCount_ = 0;
    maxHitCount_ = 0;
}

void GameScoreManager::Update()
{
    if (comboGraceFrames_ > 0) {
        --comboGraceFrames_;
        if (comboGraceFrames_ <= 0) {
            // 30Fの猶予が切れたため同時撃破カウントをリセット
            comboGraceFrames_ = 0;
            simultaneousKillCount_ = 0;
        }
    }
}

void GameScoreManager::OnEnemyDefeated(int baseScore)
{
    if (comboGraceFrames_ > 0) {
        // 猶予内に撃破した場合は同時撃破カウント増加
        ++simultaneousKillCount_;
    } else {
        // 新規撃破開始
        simultaneousKillCount_ = 1;
        comboGraceFrames_ = kMaxComboGraceFrames;
    }

    // 最大hit数を更新
    maxHitCount_ = (std::max)(maxHitCount_, simultaneousKillCount_);
    // 撃破数のカウント
    defeatCount_ += 1;

    // 基本撃破スコア加算
    int totalAddScore = baseScore;
    if (simultaneousKillCount_ >= 2) {
        int bonus = (simultaneousKillCount_ - 1) * kMultiKillBonus;
        totalAddScore += bonus;
    }
    totalScore_ += totalAddScore;
}

void GameScoreManager::AddDefeatCount(int count)
{
    defeatCount_ += count;
    totalScore_ += count * kBaseKillScore;
}

bool GameScoreManager::LoadMedalData(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open medal data file: " << filePath << std::endl;
        return false;
    }

    json data;
    try {
        file >> data;
    } catch (const json::parse_error& e) {
        std::cerr << "JSON Parse Error (medalData): " << e.what() << std::endl;
        return false;
    }

    medalLines_.clear();

    if (data.contains("medalData") && data["medalData"].is_array()) {
        for (const auto& item : data["medalData"]) {
            if (item.contains("stage_id") && item.contains("medalLine")) {
                int stageId = item.value("stage_id", 0);
                int medalLine = item.value("medalLine", 0);
                medalLines_[stageId] = medalLine;
            }
        }
    }

    return true;
}

int GameScoreManager::GetMedalLine(int stageId) const
{
    auto it = medalLines_.find(stageId);
    if (it != medalLines_.end()) {
        return it->second;
    }
    return 0;
}

bool GameScoreManager::HasEarnedMedal(int stageId) const
{
    auto it = medalLines_.find(stageId);
    if (it == medalLines_.end()) {
        return false;
    }
    return defeatCount_ >= it->second;
}
