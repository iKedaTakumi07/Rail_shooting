#pragma once
#include "../../Engine/3d/LightManager.h"
#include "../../Engine/3d/Model.h"
#include "../../Engine/3d/Object3d.h"
#include "../../Engine/base/Math.h"
#include <memory>
#include <unordered_set>

#include "../OnCollison/Collider.h"
#include "../Particle/LaserParticle.h"
#include "../Particle/chargeParticle.h"
#include "../Particle/player/shockWaveParticle.h"

class Camera;
class EnemyManager;

class PlayerBullet : public Collider {
public:
    PlayerBullet();
    ~PlayerBullet() override;

    void Initialize(Camera* camera, const Vector3& position, const Vector3& rotation);

    void Update(float deltaTime);

    void Draw();

public:
    // Get関数
    bool IsDead() const { return isDead_; }
    bool IsChargBullet() const { return isChargeBullet; }
    bool IsAlive() const { return isAlive_; }

    AllAABB GetAllAABB() const override;
    AllOBB GetAllOBB() const override;
    ColliderShape GetColliderShape() const override
    {
        return isShockWave ? ColliderShape::kSphere : ColliderShape::kOBB;
    }
    Sphere GetSphere() const override
    {
        return Sphere { transform_.translate, radius };
    }
    CollisionGroup GetCollisionGroup() const override { return CollisionGroup::kPlayerBullet; }
    void OnCollision(Collider* other) override;
    int GetDamage() const override;

    // set
    void SetTarget(uint32_t targetId, int targetIndex, EnemyManager* manager)
    {
        targetId_ = targetId;
        targetIndex_ = targetIndex;
        enemyManager_ = manager;

    } // 対象をセット
    void SetisChargeBullet(bool num)
    {
        isChargeBullet = num;
        if (isChargeBullet) {
            life = 5;
            speed_ = chageSpeed_;
            deathTimer_ = chageDeathTimer_;
        } else {
            life = 2;
            speed_ = normalSpeed_;
            deathTimer_ = normalDeathTimer_;
        }
        UpdateVelocity();
    }

private:
    void UpdateVelocity();

    void SpawnImpact();
    void shockwaveUpdate(float deltaTime);

private:
    // 多重ヒット防止
    std::unordered_set<Collider*> hitColliders_;

    Transform transform_ = { 0.0f, 0.0f, 0.0f }; // 座標
    Transform Pretransform_ = { 0.0f, 0.0f, 0.0f }; // 座標

    // 当たり判定
    float size = 0.5f; // OBBに移植後は知らん。

    Vector3 velocity_ = { 0.0f, 0.0f, 0.0f }; // 移動ベクトル

    float speed_ = 50.0f; // 弾速
    float deathTimer_ = 3.0f; // 弾の寿命（秒）

    // 通常弾
    float normalSpeed_ = 125.0f; // 弾速
    float normalDeathTimer_ = 1.0f; // 弾の寿命（秒）

    // チャージ弾
    float chageSpeed_ = 50.0f; // 弾速
    float chageDeathTimer_ = 3.0f; // 弾の寿命（秒）
    bool isDead_ = false;
    bool isAlive_ = false; // 衝撃波含め消滅したか

    // チャージ弾衝撃波
    float shockwaveTimer = 0.2f; // 衝撃波の寿命
    float radius = 1.0f; // 衝撃波の半径
    const float kExpandSpeed = 10.0f;
    bool isShockWave = false; // 衝撃波があるか
    bool isShockWaveDead_ = false; //

    uint32_t targetId_ = 0; // 追尾する対象
    int targetIndex_ = 0; // 追尾する部位の番号

    EnemyManager* enemyManager_ = nullptr;
    float homingStrength_ = 0.0f; // 追ビ性能(ほぼ必中で良い)
    bool isChargeBullet = false;
    int Dameg = 5; // 通常弾
    int ChageDameg = 10; // チャージ弾(衝撃波含む)

    int life = 2;

    PointLigth pointLight_ { };

    // 3dモデル
    std::unique_ptr<Model> model;
    std::unique_ptr<Object3d> object3d;

    std::unique_ptr<LaserParticle> laserParticle_;
    std::unique_ptr<chargeParticle> chargeParticle_;
    std::unique_ptr<shockWaveParticle> shockWaveParticle_;
};
