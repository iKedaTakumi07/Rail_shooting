#pragma once
#include "../../Engine/base/Math.h"

enum class CollisionGroup {
    kPlayer,
    kPlayerDodge,
    kPlayerBullet,
    kEnemy,
    kEnemyBullet,
    kStageObject,
};

enum class ColliderShape {
    kAABB, // min,max
    kOBB,
    kSphere,
};

struct Segment {
    Vector3 origin; // 開始点 (前フレームの座標)
    Vector3 diff; // 移動量ベクトル (現在座標 - 前フレーム座標)
};

class Collider {
public:
    // [予定]AABBでの判定作成→OBBの判定に変更→翼、プレイヤーなど本体の分離型OBB判定に変更。

    virtual ~Collider() = default;

    virtual AllAABB GetAllAABB() const = 0;
    virtual AllOBB GetAllOBB() const = 0;
    virtual Sphere GetSphere() const { return Sphere { currentPosition_, 1.0f }; }

    virtual CollisionGroup GetCollisionGroup() const = 0;
    virtual ColliderShape GetColliderShape() const { return ColliderShape::kOBB; }

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

    /// <summary>
    /// 毎フレームの座標更新処理 (現在座標を渡して過去座標をシフト保存)
    /// </summary>
    /// <param name="currentPos">現在のワールド座標</param>
    void UpdatePrevPosition(const Vector3& currentPos);

    /// <summary>
    /// 初期か
    /// </summary>
    void ResetPrevPosition(const Vector3& pos);

    /// <summary>
    /// 前フレームの座標を取得
    /// </summary>
    const Vector3& GetPrevPosition() const { return prevPosition_; }

    /// <summary>
    /// 現在の座標を取得
    /// </summary>
    const Vector3& GetCurrentPosition() const { return currentPosition_; }

    /// <summary>
    /// 前フレーム〜現フレームの移動線分（Sweep Segment）を取得
    /// </summary>
    Segment GetSweepSegment() const
    {
        return Segment { prevPosition_, currentPosition_ - prevPosition_ };
    }

    /// <summary>
    /// 過去座標が初期化されているか
    /// </summary>
    bool IsPrevPositionInitialized() const { return isPrevPositionInitialized_; }

private:
    Vector3 prevPosition_ { 0.0f, 0.0f, 0.0f };
    Vector3 currentPosition_ { 0.0f, 0.0f, 0.0f };
    bool isPrevPositionInitialized_ = false;
};