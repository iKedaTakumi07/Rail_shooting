#pragma once

#include "../../../Engine/base/Math.h"
#include "baseEnemyBullet.h"
#include <memory>
class Camera;
class Player;

#include "../../OnCollison/Collider.h"

class baseEnemy : public Collider {
public:
    virtual void Initialize(Vector3 pos) = 0;

    virtual void Update() = 0;

    virtual void Draw() = 0;

    virtual void SpriteDraw() { }; // (ほぼ)ボス専用

    virtual void withdrawalUpdate() { };

    /* Set関数 */
    virtual void SetTargetPlayer(Player* target) { }; // 対象に向かわせる
    virtual void SetHp(int num) { };
    virtual void SetUseBullet(int num) { };
    virtual void SetHomingPower(float num) { };
    virtual void SetMove(Vector3 num) { }
    virtual void SetbasePos(Vector3 num) { }
    virtual void SetId(uint32_t id) { id_ = id; }
    /* Get関数 */
    const std::vector<std::unique_ptr<baseEnemyBullet>>& GetBullets() const { return enemyBullet_; }
    virtual Vector3 GetTranslate() = 0;
    virtual bool GetIsAvile_() = 0;
    virtual uint32_t GetId() const { return id_; }
    virtual std::vector<Vector3> GetTargetPositions() { return { GetTranslate() }; }

protected:
    // 弾
    std::vector<std::unique_ptr<baseEnemyBullet>> enemyBullet_;
    uint32_t id_ = 0;
};
