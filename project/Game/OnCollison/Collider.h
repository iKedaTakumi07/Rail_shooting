#pragma once

#include "../../Engine/base/Math.h"

enum class CollisionGroup {
    kPlayer,
    kPlayerBullet,
    kEnenmy,
    kEnemyBullet,
    kStageObject,
};

struct AllOBB {
    OBB wholeBox; // 分割しないオブジェクト自体の大きさ
    std::vector<OBB> dividBoxes; // 分割した詳細判定用OBB
};

struct AllAABB {
    AABB wholeBox; // 分割しないオブジェクト自体の大きさ
    std::vector<AABB> dividBoxes; // 分割した判定
};

class Collider {
public:
    // [予定]AABBでの判定作成→OBBの判定に変更→翼、プレイヤーなど本体の分離型OBB判定に変更。

    virtual ~Collider() = default;

    virtual AllAABB GetAllAABB() const = 0;

    virtual CollisionGroup GetCollisionGroup() const = 0;

    //virtual AllOBB GetAllOBB() const = 0;

    //virtual CollisionGroup GetCollisionGroup() const = 0;

    /// <summary>
    /// 当たり半テオ
    /// </summary>
    /// <param name="other">当たった側のポインタ</param>
    virtual void OnCollision(Collider* other) = 0;

    /// <summary>
    /// 自身が相手に与えるダメージ量
    /// </summary>
    /// <returns>デフォルト0</returns>
    virtual int GetDamage() const { return 0; }

private:
    // 衝突半径
    float radius_ = 5.0f;
};