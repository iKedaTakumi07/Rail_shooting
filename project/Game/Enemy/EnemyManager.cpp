#include "EnemyManager.h"
#include "Normal/CircleMoveEnemy.h"
#include "Normal/FixedEnemy.h"
#include "Normal/LaserEnemy.h"
#include "Normal/NormalMoveEnemy.h"
#include "Normal/threeShotsEnemy.h"
#include "base/baseEnemy.h"
#include "boss/FourEyesBoss.h"

#include "../../Engine/3d/CameraManager.h"
#include "../../resources/nlohmann/json.hpp"
#include <algorithm>

#include <fstream>
#include <iostream>

using json = nlohmann::json;

void EnemyManager::Initialize(Player* player, const std::string& filePath)
{
    player_ = player;
    PopEnemyFilePath_ = filePath;
    currentSpawnIndex_ = 0;

    LoadJsonPopData();
}

void EnemyManager::Update()
{
    camera_ = CameraManager::GetInstance()->GetActiveCamera();
    Vector3 pos = player_->GetTranslate();

    while (currentSpawnIndex_ < popDatas_.size() && pos.z >= popDatas_[currentSpawnIndex_].spawn_Z) {
        PopEnemyCheck(popDatas_[currentSpawnIndex_]);
        currentSpawnIndex_++;
    }

    for (auto& enemy : enemies_) {
        // 敵更新
        enemy->Update();
        // スコア加算
    }

    // 死亡している奴ら削除
    std::erase_if(enemies_, [](const std::unique_ptr<baseEnemy>& enemy) {
        return !enemy->GetIsAvile_();
    });
}

void EnemyManager::Draw()
{
    for (auto& enemy : enemies_) {
        enemy->Draw();
    }
}

void EnemyManager::SpriteDraw()
{
    for (auto& enemy : enemies_) {
        enemy->SpriteDraw();
    }
}

baseEnemy* EnemyManager::GetEnemyById(uint32_t id) const
{
    if (id == 0)
        return nullptr;

    // 対象のIDを検索
    for (const auto& enemy : enemies_) {
        if (enemy->GetId() == id) {
            return enemy.get();
        }
    }

    // 破棄されている場合はnullptr
    return nullptr;
}

void EnemyManager::LoadJsonPopData()
{
    std::ifstream file(PopEnemyFilePath_);
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

    if (data.contains("enemies") && data["enemies"].is_array()) {
        for (const auto& enemyData : data["enemies"]) {
            EnemyPopData popData;

            // 絶対に代入するもの
            popData.enemyPopType = enemyData.value("type", "FixedEnemy");
            popData.spawn_Z = enemyData.value("spawn_z", 0.0f);
            popData.hp = enemyData.value("hp", 5);

            if (enemyData.contains("pos")) {
                const auto& posData = enemyData["pos"];
                popData.popPosition.x = posData.value("x", 0.0f);
                popData.popPosition.y = posData.value("y", 0.0f);
                popData.popPosition.z = posData.value("z", 40.0f);
            } else {
                popData.popPosition = { 0.0f, 0.0f, 40.0f };
            }

            popData.useBullet = enemyData.value("UseBullet", "TargetBullet");

            // 追尾弾のみ
            if (popData.useBullet == "HomingBullet") {
                popData.homingPower = enemyData.value("homingPower", 0.1f);
            }

            // 固定以外
            if (popData.enemyPopType != "FixedEnemy") {
                if (enemyData.contains("MoveDirection")) {
                    const auto& posData = enemyData["MoveDirection"];
                    popData.moveDirection.x = posData.value("x", 0.0f);
                    popData.moveDirection.y = posData.value("y", 0.0f);
                    popData.moveDirection.z = posData.value("z", 1.0f); // エラーの時わかりやすくする溜
                } else {
                    popData.moveDirection = { 0.0f, 0.0f, 1.0f };
                }
            }

            // そうにゅ
            popDatas_.push_back(popData);
        }
    }

    // sort
    std::sort(popDatas_.begin(), popDatas_.end(), [](const EnemyPopData& a, const EnemyPopData& b) {
        return a.spawn_Z < b.spawn_Z;
    });
}

void EnemyManager::PopEnemyCheck(const EnemyPopData& data)
{
    // 敵生成
    std::unique_ptr<baseEnemy> newEnemy = nullptr;

    // タイプごとの初期化
    if (data.enemyPopType == "FixedEnemy") {
        newEnemy = std::make_unique<FixedEnemy>();
    } else if (data.enemyPopType == "NormalMoveEnemy") {
        newEnemy = std::make_unique<NormalMoveEnemy>();
    } else if (data.enemyPopType == "CircleMoveEnemy") {
        newEnemy = std::make_unique<CircleMoveEnemy>();
    } else if (data.enemyPopType == "threeShotsEnemy") {
        newEnemy = std::make_unique<threeShotsEnemy>();
    } else if (data.enemyPopType == "FourEyesBoss") {
        newEnemy = std::make_unique<FourEyesBoss>();
    } else if (data.enemyPopType == "LaserEnemy") {
        newEnemy = std::make_unique<LaserEnemy>();
    } else {
        // 万が一のエラー対策
        newEnemy = std::make_unique<FixedEnemy>();
    }

    if (newEnemy) {
        newEnemy->Initialize(data.popPosition);
        newEnemy->SetbasePos(data.popPosition);
        newEnemy->SetTargetPlayer(player_);
        newEnemy->SetMove(data.moveDirection);
        newEnemy->SetHp(data.hp);
        // ID割り当て
        newEnemy->SetId(nextEnemyId_++);
        // 今後弾を追加したなら変更
        if (data.useBullet == "HomingBullet") {
            newEnemy->SetHomingPower(data.homingPower);
            newEnemy->SetUseBullet(1); // 1番をHomingBullet
        } else {
            newEnemy->SetUseBullet(0); // 0番をTargetBullet
        }

        if (data.enemyPopType == "LaserEnemy") {
            Vector3 endPos = { data.popPosition.x + data.moveDirection.x, data.popPosition.y + data.moveDirection.y, data.popPosition.z + data.moveDirection.z };
            newEnemy->SetLastStopPos(endPos);
        }

        enemies_.push_back(std::move(newEnemy));
    }
}
