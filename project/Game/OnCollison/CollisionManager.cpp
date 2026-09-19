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
            if (CheckAllOBBCollision(colA->GetAllOBB(), colB->GetAllOBB())) {
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

static bool IsSeparatedOnAxis(const Vector3& axis, const OBB& a, const OBB& b, const Vector3& T)
{
    // 辺同士が平行な場合、クロス積が零ベクトルになるため判定をスキップ
    float sqrLen = Dot(axis, axis);
    if (sqrLen < 0.0f) {
        return false;
    }

    // 軸の正規化
    Vector3 normAxis = Multiply(axis, 1.0f / std::sqrt(sqrLen));

    // 中心距離の投影
    float distance = std::abs(Dot(T, normAxis));

    // OBB A の投影半径
    float radiusA = a.size.x * std::abs(Dot(a.orientations[0], normAxis)) + a.size.y * std::abs(Dot(a.orientations[1], normAxis)) + a.size.z * std::abs(Dot(a.orientations[2], normAxis));

    // OBB B の投影半径
    float radiusB = b.size.x * std::abs(Dot(b.orientations[0], normAxis)) + b.size.y * std::abs(Dot(b.orientations[1], normAxis)) + b.size.z * std::abs(Dot(b.orientations[2], normAxis));

    // 分離していれば true
    return distance > (radiusA + radiusB);
}

bool CollisionManager::CheckOBB(const OBB& a, const OBB& b)
{
    Vector3 T = {
        b.center.x - a.center.x,
        b.center.y - a.center.y,
        b.center.z - a.center.z
    };

    // 軸 1~3: OBB A のローカル3軸
    for (int i = 0; i < 3; ++i) {
        if (IsSeparatedOnAxis(a.orientations[i], a, b, T))
            return false;
    }

    // 軸 4~6: OBB B のローカル3軸
    for (int i = 0; i < 3; ++i) {
        if (IsSeparatedOnAxis(b.orientations[i], a, b, T))
            return false;
    }

    // 軸 7~15: OBB A と OBB B の各辺のクロス積 (3x3 = 9軸)
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            Vector3 crossAxis = Cross(a.orientations[i], b.orientations[j]);
            if (IsSeparatedOnAxis(crossAxis, a, b, T))
                return false;
        }
    }

    // 15軸すべてで分離軸が見つからない場合は交差している
    return true;
}

bool CollisionManager::CheckAllOBBCollision(const AllOBB& a, const AllOBB& b) const
{
    // 本体判定が非交差なら早期リターン
    if (!CheckOBB(a.wholeBox, b.wholeBox)) {
        return false;
    }

    // 複合OBB（詳細判定用）の総当たり
    for (const auto& boxA : a.dividBoxes) {
        for (const auto& boxB : b.dividBoxes) {
            if (CheckOBB(boxA, boxB)) {
                return true;
            }
        }
    }
    return false;
}
