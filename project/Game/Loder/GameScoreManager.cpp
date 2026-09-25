#include "GameScoreManager.h"
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
    defeatCount_ = 0;
}

void GameScoreManager::AddDefeatCount(int count)
{
    defeatCount_ += count;
}