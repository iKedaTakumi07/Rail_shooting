#pragma once
#include "Collider.h"
#include <list>

class CollisionManager {
public:
    // 判定対象
    void AddCollider(Collider* collider);

    void Clear();

    void CheckAllCollisions();

private:
    bool CheckAABB(const AABB& a, const AABB& b) const;

    bool CheckAllAABBCollision(const AllAABB& a, const AllAABB& b) const;

    bool CheckOBB(const OBB& a, const OBB& b) const;
    bool CheckAllOBBCollision(const AllOBB& a, const AllOBB& b) const;

    std::list<Collider*> colliders_;
};
