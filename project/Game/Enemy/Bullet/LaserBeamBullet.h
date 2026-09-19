#pragma once
#include "../base/baseEnemyBullet.h"

#include "../../../Engine/3d/Model.h"
#include "../../../Engine/3d/Object3d.h"
#include "../../../Engine/base/Math.h"
#include "../../OnCollison/Collider.h"
#include "../../Particle/LaserParticle.h"
#include <memory>

class Player;
class LaserBeamBullet : public baseEnemyBullet {
public:
    void Initialize(Vector3 pos, const Vector3& rotation) override;

    void Update(float deltaTime) override;

    void Draw() override;

public:
    // Get関数
    bool GetIsDead() const override { return isDead_; };

    AllAABB GetAllAABB() const override;
    AllOBB GetAllOBB() const override;
    CollisionGroup GetCollisionGroup() const override { return CollisionGroup::kEnemyBullet; }
    void OnCollision(Collider* other) override;
    int GetDamage() const override { return dameg_; }

    // Set関数
    void SetTargetPosition(Vector3 Pos);
    void SetIsDead(bool num) { isDead_ = num; }
    void SetPlayerPos(Vector3 pos) override { pos; }
    void SetPositions(const Vector3& start, const Vector3& end);

private:
    void MoveUpdate();
    void RoateUpdate();
    void CheckCameraCulling(); // カリング処理

private:
    Transform transform_;

    // 3dモデル
    std::unique_ptr<Model> model;
    std::unique_ptr<Object3d> object3d;

    Vector3 startPos_;
    Vector3 endPos_;
    float laserRadius_ = 0.25f;

    // 当たり判定
    float size = 0.5f; // OBBに移植後は知らん。

    bool isDead_ = false;

    int dameg_ = 2;

    // 3dモデル(オブジェクトを出さずトレイルエフェクトを利用して描画する予定)
    std::unique_ptr<LaserParticle> laserParticle_;
};
