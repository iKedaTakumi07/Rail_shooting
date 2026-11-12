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
    static bool CheckSphereOBB(const Sphere& sphere, const OBB& obb);
    static bool CheckSphereSphere(const Sphere& a, const Sphere& b);
    static bool CheckSegmentOBB(const Segment& segment, const OBB& obb);
    static bool CheckContinuousOBBCollision(const Collider* colA, const Collider* colB);

private:
    bool CheckCollisionPair(Collider* colA, Collider* colB);

    // 当たり判定可視化OBB
    static std::vector<Vector4> GenerateOBBLineVertices(const OBB& obb);
    // 当たり判定可視化球体
    static std::vector<Vector4> GenerateSphereLineVertices(const Sphere& sphere, uint32_t segments = 16);

    // 現在仕様しないもの(軽量化で解禁するカモメ)
    bool CheckAABB(const AABB& a, const AABB& b) const;
    bool CheckAllAABBCollision(const AllAABB& a, const AllAABB& b) const;
    bool CheckAllOBBCollision(const AllOBB& a, const AllOBB& b) const;

    std::list<Collider*> colliders_;
};
