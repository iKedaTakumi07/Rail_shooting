#include "CollisionManager.h"

void CollisionManager::AddCollider(Collider* collider)
{
    colliders_.push_back(collider);
}

void CollisionManager::Clear()
{
    colliders_.clear();
}

void CollisionManager::CheckAllCollisions()
{
    // 総当たり
    auto itA = colliders_.begin();
    for (; itA != colliders_.end(); ++itA) {
        Collider* colA = *itA;

        auto itB = itA;
        ++itB; // 次の要素から比較
        for (; itB != colliders_.end(); ++itB) {
            Collider* colB = *itB;

            // 同じ陣営はスキップ
            if (colA->GetCollisionGroup() == colB->GetCollisionGroup())
                continue;
            if (colA->GetCollisionGroup() == CollisionGroup::kPlayer && colB->GetCollisionGroup() == CollisionGroup::kPlayerBullet)
                continue;
            if (colA->GetCollisionGroup() == CollisionGroup::kPlayerBullet && colB->GetCollisionGroup() == CollisionGroup::kPlayer)
                continue;

            // 交差していれば、お互いのオーバーライドされたOnCollisionを呼び出す
            if (CheckAllAABBCollision(colA->GetAllAABB(), colB->GetAllAABB())) {
                colA->OnCollision(colB);
                colB->OnCollision(colA);
            }
        }
    }
}

bool CollisionManager::CheckAABB(const AABB& a, const AABB& b) const
{
    // AABB
    if (a.min.x <= b.max.x && a.max.x >= b.min.x && a.min.y <= b.max.y && a.max.y >= b.min.y && a.min.z <= b.max.z && a.max.z >= b.min.z) {
        return true;
    }
    return false;
}

bool CollisionManager::CheckAllAABBCollision(const AllAABB& a, const AllAABB& b) const
{
    // 本体自体が当たってないなら早期リターン
    if (!CheckAABB(a.wholeBox, b.wholeBox)) {
        return false;
    }

    for (const auto& boxA : a.dividBoxes) {
        for (const auto& boxB : b.dividBoxes) {
            if (CheckAABB(boxA, boxB)) {
                return true;
            }
        }
    }
    return false;
}

bool CollisionManager::CheckOBB(const OBB& a, const OBB& b) const
{
    return false;
}

bool CollisionManager::CheckAllOBBCollision(const AllOBB& a, const AllOBB& b) const
{
    return false;
}
