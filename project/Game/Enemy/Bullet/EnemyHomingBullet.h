#pragma once
#include "../base/baseEnemyBullet.h"

#include "../../../Engine/3d/Model.h"
#include "../../../Engine/3d/Object3d.h"
#include "../../../Engine/base/Math.h"
#include "../../Particle/LaserParticle.h"
#include <memory>
#include "../../OnCollison/Collider.h"

class Player;

class EnemyHomingBullet : public baseEnemyBullet {
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
    void SetPlayerPos(Vector3 pos) override { targetPos_ = pos; }

private:
    void MoveUpdate();
    void RoateUpdate();
    void CheckCameraCulling(); // カリング処理

private:
    Transform transform_;

    // 当たり判定
    float size = 0.5f; // OBBに移植後は知らん。

    Vector3 acceleration_; // 弾の速さ(個別で設定)
    float accelerationScalar = 0.15f; // 加速度の強さ
    Vector3 velocity_ = { 0.0f, 0.0f, 0.0f }; // 移動ベクトル
    static inline const float maxSpeed = 0.80f; // 弾の最高速度(青天井でおk)
    float deathTimer_ = 3.0f; // 弾の寿命（秒）
    float LockOnTimer_ = 2.0f; // 弾の捕捉時間
    float homingPower = 0.10f; // 捕捉力
    bool isDead_ = false;

    float particleTimer_ = 0.0f; // 経過時間タイマー
    const float kParticleInterval_ = 0.025f; // パーティクル発生間隔

    int dameg_ = 2;

    Vector3 targetPos_; // 追跡対象、または狙う場所

    // 3dモデル
    std::unique_ptr<Model> model;
    std::unique_ptr<Object3d> object3d;

    std::unique_ptr<LaserParticle> laserParticle_;
};
