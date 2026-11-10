#include "CollisionManager.h"
#include "../../Engine/3d/Camera.h"
#include "../../Engine/3d/Object3dCommon.h"
#include <algorithm>
#include <numbers>
#include <cmath>

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
            if (colA->GetCollisionGroup() == CollisionGroup::kPlayer && colB->GetCollisionGroup() == CollisionGroup::kPlayerDodge)
                continue;
            if (colA->GetCollisionGroup() == CollisionGroup::kPlayerDodge && colB->GetCollisionGroup() == CollisionGroup::kPlayer)
                continue;
            if (colA->GetCollisionGroup() == CollisionGroup::kPlayerDodge && colB->GetCollisionGroup() == CollisionGroup::kPlayerBullet)
                continue;
            if (colA->GetCollisionGroup() == CollisionGroup::kPlayerBullet && colB->GetCollisionGroup() == CollisionGroup::kPlayerDodge)
                continue;
            if (colA->GetCollisionGroup() == CollisionGroup::kPlayer && colB->GetCollisionGroup() == CollisionGroup::kPlayerBullet)
                continue;
            if (colA->GetCollisionGroup() == CollisionGroup::kPlayerBullet && colB->GetCollisionGroup() == CollisionGroup::kPlayer)
                continue;
            if (colA->GetCollisionGroup() == CollisionGroup::kEnemy && colB->GetCollisionGroup() == CollisionGroup::kEnemyBullet)
                continue;
            if (colA->GetCollisionGroup() == CollisionGroup::kEnemyBullet && colB->GetCollisionGroup() == CollisionGroup::kEnemy)
                continue;

            // 交差していれば、お互いのオーバーライドされたOnCollisionを呼び出す
            if (CheckCollisionPair(colA, colB)) {
                colA->OnCollision(colB);
                colB->OnCollision(colA);
            }
        }
    }
}

void CollisionManager::DrawDebug(Camera* camera)
{
#ifdef USE_IMGUI
    if (!camera || colliders_.empty())
        return;

    std::vector<Vector4> allLineVertices;

    // 登録されている全コライダーの OBB を Vector4 頂点配列に集積
    for (Collider* col : colliders_) {
        if (!col)
            continue;

        if (col->GetColliderShape() == ColliderShape::kSphere) {
            auto sphereVerts = GenerateSphereLineVertices(col->GetSphere());
            allLineVertices.insert(allLineVertices.end(), sphereVerts.begin(), sphereVerts.end());
        } else {

            AllOBB allOBB = col->GetAllOBB();

            // 全体包囲ボックス
            auto wholeVerts = GenerateOBBLineVertices(allOBB.wholeBox);
            allLineVertices.insert(allLineVertices.end(), wholeVerts.begin(), wholeVerts.end());

            // 分割コライダー
            for (const auto& box : allOBB.dividBoxes) {
                auto divVerts = GenerateOBBLineVertices(box);
                allLineVertices.insert(allLineVertices.end(), divVerts.begin(), divVerts.end());
            }
        }
    }

    // フレームワーク層の描画機能へ処理を委譲（ゲーム層で低レイヤー処理を行わない）
    Object3dCommon::GetInstance()->DrawLines(allLineVertices, camera);
#endif // USE_IMGUI
}

std::vector<Vector4> CollisionManager::GenerateOBBLineVertices(const OBB& obb)
{
    Vector3 extX = Multiply(obb.orientations[0], obb.size.x);
    Vector3 extY = Multiply(obb.orientations[1], obb.size.y);
    Vector3 extZ = Multiply(obb.orientations[2], obb.size.z);

    // OBBの8つの頂点を Vector4 (w = 1.0f) で計算
    Vector4 v[8];
    Vector3 p[8] = {
        obb.center - extX - extY - extZ,
        obb.center + extX - extY - extZ,
        obb.center + extX + extY - extZ,
        obb.center - extX + extY - extZ,
        obb.center - extX - extY + extZ,
        obb.center + extX - extY + extZ,
        obb.center + extX + extY + extZ,
        obb.center - extX + extY + extZ
    };

    for (int i = 0; i < 8; ++i) {
        v[i] = { p[i].x, p[i].y, p[i].z, 1.0f };
    }

    // 12本の辺（24頂点）
    return {
        // 底面
        v[0], v[1], v[1], v[2], v[2], v[3], v[3], v[0],
        // 上面
        v[4], v[5], v[5], v[6], v[6], v[7], v[7], v[4],
        // 側面
        v[0], v[4], v[1], v[5], v[2], v[6], v[3], v[7]
    };
}

std::vector<Vector4> CollisionManager::GenerateSphereLineVertices(const Sphere& sphere, uint32_t segments)
{
    std::vector<Vector4> vertices;
    float step = 2.0f * std::numbers::pi_v<float> / float(segments);
    for (uint32_t i = 0; i < segments; ++i) {
        float r1 = i * step;
        float r2 = (i + 1) * step;
        float cos1 = std::cos(r1) * sphere.radius;
        float sin1 = std::sin(r1) * sphere.radius;
        float cos2 = std::cos(r2) * sphere.radius;
        float sin2 = std::sin(r2) * sphere.radius;
        // XY平面の円
        vertices.push_back({ sphere.center.x + cos1, sphere.center.y + sin1, sphere.center.z, 1.0f });
        vertices.push_back({ sphere.center.x + cos2, sphere.center.y + sin2, sphere.center.z, 1.0f });
        // YZ平面の円
        vertices.push_back({ sphere.center.x, sphere.center.y + cos1, sphere.center.z + sin1, 1.0f });
        vertices.push_back({ sphere.center.x, sphere.center.y + cos2, sphere.center.z + sin2, 1.0f });
        // ZX平面の円
        vertices.push_back({ sphere.center.x + sin1, sphere.center.y, sphere.center.z + cos1, 1.0f });
        vertices.push_back({ sphere.center.x + sin2, sphere.center.y, sphere.center.z + cos2, 1.0f });
    }
    return vertices;
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

bool CollisionManager::CheckSphereOBB(const Sphere& sphere, const OBB& obb)
{
    Vector3 d = sphere.center - obb.center;
    Vector3 closestPoint = obb.center;

    float sizes[3] = { obb.size.x, obb.size.y, obb.size.z };
    for (int i = 0; i < 3; ++i) {
        float dist = Dot(d, obb.orientations[i]);
        dist = std::clamp(dist, -sizes[i], sizes[i]);
        closestPoint = closestPoint + Multiply(obb.orientations[i], dist);
    }

    Vector3 diff = sphere.center - closestPoint;
    float sqDistance = Dot(diff, diff);

    return sqDistance <= (sphere.radius * sphere.radius);
}

bool CollisionManager::CheckSphereSphere(const Sphere& a, const Sphere& b)
{
    Vector3 diff = a.center - b.center;
    float sqDistance = Dot(diff, diff);
    float radiusSum = a.radius + b.radius;
    return sqDistance <= (radiusSum * radiusSum);
}

bool CollisionManager::CheckSegmentOBB(const Segment& segment, const OBB& obb)
{
    Vector3 p = segment.origin - obb.center;
    Vector3 d = segment.diff; // 移動量ベクトル

    float tMin = 0.0f;
    float tMax = 1.0f;

    // OBBの3つのローカル軸に対してスラブ判定を実行
    for (int i = 0; i < 3; ++i) {
        const Vector3& axis = obb.orientations[i];
        float halfSize = (i == 0) ? obb.size.x : (i == 1) ? obb.size.y
                                                          : obb.size.z;

        float e = Dot(axis, p);
        float f = Dot(axis, d);

        if (std::abs(f) > 0.0f) {
            float t1 = (-e - halfSize) / f;
            float t2 = (-e + halfSize) / f;

            if (t1 > t2)
                std::swap(t1, t2);

            tMin = (std::max)(tMin, t1);
            tMax = (std::min)(tMax, t2);

            if (tMin > tMax)
                return false; // 共通範囲が存在しないため非交差
        } else {
            // 線分が軸に平行な場合、OBBの範囲外であれば非交差
            if (-e - halfSize > 0.0f || -e + halfSize < 0.0f) {
                return false;
            }
        }
    }

    return true;
}

bool CollisionManager::CheckContinuousOBBCollision(const Collider* colA, const Collider* colB)
{
    AllOBB obbA = colA->GetAllOBB();
    AllOBB obbB = colB->GetAllOBB();

    if (!CheckOBB(obbA.wholeBox, obbB.wholeBox)) {
        bool sweepHit = false;

        if (colA->IsPrevPositionInitialized()) {
            Segment segA = colA->GetSweepSegment();
            if (Dot(segA.diff, segA.diff) > 0.0f) {
                for (const auto& boxB : obbB.dividBoxes) {
                    if (CheckSegmentOBB(segA, boxB)) {
                        sweepHit = true;
                        break;
                    }
                }
            }
        }
        if (!sweepHit && colB->IsPrevPositionInitialized()) {
            Segment segB = colB->GetSweepSegment();
            if (Dot(segB.diff, segB.diff) > 0.0f) {
                for (const auto& boxA : obbA.dividBoxes) {
                    if (CheckSegmentOBB(segB, boxA)) {
                        sweepHit = true;
                        break;
                    }
                }
            }
        }
        return sweepHit;
    }

    for (const auto& boxA : obbA.dividBoxes) {
        for (const auto& boxB : obbB.dividBoxes) {
            if (CheckOBB(boxA, boxB)) {
                return true; // 実際のパーツ同士が交差した
            }
        }
    }

    return false;
}

bool CollisionManager::CheckCollisionPair(Collider* colA, Collider* colB)
{
    ColliderShape shapeA = colA->GetColliderShape();
    ColliderShape shapeB = colB->GetColliderShape();

    // 球体同士
    if (shapeA == ColliderShape::kSphere && shapeB == ColliderShape::kSphere) {
        return CheckSphereSphere(colA->GetSphere(), colB->GetSphere());
    }

    // 片方球体/片方OBB
    if (shapeA == ColliderShape::kSphere && shapeB == ColliderShape::kOBB) {
        for (const auto& box : colB->GetAllOBB().dividBoxes) {
            if (CheckSphereOBB(colA->GetSphere(), box)) {
                return true;
            }
        }
        return false;
    }
    if (shapeA == ColliderShape::kOBB && shapeB == ColliderShape::kSphere) {
        for (const auto& box : colA->GetAllOBB().dividBoxes) {
            if (CheckSphereOBB(colB->GetSphere(), box)) {
                return true;
            }
        }
        return false;
    }

    // 両方OBB
    if (shapeA == ColliderShape::kOBB && shapeB == ColliderShape::kOBB) {
        return CheckContinuousOBBCollision(colA, colB);
    }

    // ここを通る時点で該当判定がない
    return false;
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
