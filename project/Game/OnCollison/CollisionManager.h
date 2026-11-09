#pragma once
#include "Collider.h"
#include <list>
class Camera;

class CollisionManager {
public:
    // 判定対象
    void AddCollider(Collider* collider);

    void Clear();

    void CheckAllCollisions();

    void DrawDebug(Camera* camera);

    static bool CheckOBB(const OBB& a, const OBB& b);

    static bool CheckSegmentOBB(const Segment& segment, const OBB& obb);

    static bool CheckContinuousOBBCollision(const Collider* colA, const Collider* colB);

private:
    static std::vector<Vector3> GenerateOBBLineVertices(const OBB& obb);

    bool CheckAABB(const AABB& a, const AABB& b) const;

    bool CheckAllAABBCollision(const AllAABB& a, const AllAABB& b) const;

    bool CheckAllOBBCollision(const AllOBB& a, const AllOBB& b) const;

    std::list<Collider*> colliders_;
};
