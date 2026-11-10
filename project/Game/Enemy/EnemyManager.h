#pragma once

#include <memory>
#include <vector>

class baseEnemy;
class FixedEnemy;
class Camera;
#include "../Player/Player.h"

struct EnemyPopData {
    std::string enemyPopType; // 敵の種類
    float spawn_Z; // 出現するタイミング
    Vector3 popPosition; // 出現場所
    std::string useBullet; // 使う弾の種類
    int hp; // 体力

    // 動く系のみ
    Vector3 moveDirection; // 進行方向
    //

    // 追尾弾のみ
    float homingPower;
    //
};

class EnemyManager {
public:
    void Initialize(Player* player, const std::string& filePath);

    void Update();

    void Draw();

    void SpriteDraw();

    void ReloadJson(const std::string& filePath = "");

public:
    // Get関数
    const std::vector<std::unique_ptr<baseEnemy>>& GetEnemyes() const { return enemies_; } // 弾の入手
    bool IsAllPopFinished() const { return currentSpawnIndex_ >= popDatas_.size(); } // 全てpopしたか
    bool IsAllEnemiesCleared() const { return IsAllPopFinished() && enemies_.empty(); } // なおかつ敵が全滅しているか。
    baseEnemy* GetEnemyById(uint32_t id) const;

    void LoadJsonPopData();

    void PopEnemyCheck(const EnemyPopData& data);

private:
    Camera* camera_ = nullptr; // カメラポインタ
    Player* player_ = nullptr; // ターゲット用のプレイヤーポインタ

    std::vector<std::unique_ptr<baseEnemy>> enemies_; // 生きている敵のリスト

    size_t currentSpawnIndex_ = 0; // 次に出現させる敵のインデックス
    std::string PopEnemyFilePath_; // 現在読み込んでいるファイル
    std::vector<EnemyPopData> popDatas_; // 読み込んだデータ

    uint32_t nextEnemyId_ = 1; // 敵のIDカウンター
};
