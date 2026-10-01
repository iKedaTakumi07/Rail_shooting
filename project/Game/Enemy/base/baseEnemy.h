#pragma once

#include "../../../Engine/base/Math.h"
#include "baseEnemyBullet.h"
#include <memory>
class Camera;
class Player;

#include "../../OnCollison/Collider.h"
#include <functional>

class baseEnemy : public Collider {
public:
    virtual void Initialize(Vector3 pos) = 0;

    virtual void Update() = 0;

    virtual void Draw() = 0;

    virtual void SpriteDraw() { }; // (ほぼ)ボス専用

    virtual void WithdrawalUpdate() { };

public:
    // 弾の追加
    void AddBullet(std::unique_ptr<baseEnemyBullet> bullet);

    bool isBulletEmpty() const { return enemyBullet_.empty(); } // レーザビーム用
    void RemoveBulletsIf(const std::function<bool(const baseEnemyBullet*)>& predicate); // 特定の要素を削除する

    // 弾の更新
    void UpdateBullets(float deltaTime, const Vector3& playerPos);

    // 描画
    void DrawBullets() const;

public:
    /* Set関数 */
    virtual void SetTargetPlayer(Player* target) { }; // 対象に向かわせる
    virtual void SetHp(int num) { };
    virtual void SetUseBullet(int num) { };
    virtual void SetHomingPower(float num) { };
    virtual void SetMove(Vector3 num) { }
    virtual void SetbasePos(Vector3 num) { }
    virtual void SetId(uint32_t id) { id_ = id; }
    virtual void SetLastStopPos(Vector3 num) { };

    /* Get関数 */
    const std::vector<std::unique_ptr<baseEnemyBullet>>& GetBullets() const { return enemyBullet_; }
    virtual Vector3 GetTranslate() = 0;
    virtual bool GetIsAvile_() = 0;
    virtual uint32_t GetId() const { return id_; }
    virtual std::vector<Vector3> GetTargetPositions() { return { GetTranslate() }; }

private:
    // 弾
    std::vector<std::unique_ptr<baseEnemyBullet>> enemyBullet_;
    uint32_t id_ = 0;
};
