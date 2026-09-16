#pragma once
#include "../base/baseEnemy.h"
#include <memory>

#include "../../../Engine/3d/Object3d.h"
#include "../../../Engine/base/Math.h"

class Player;
class Model;

class NormalMoveEnemy : public baseEnemy {
public:
    void Initialize(Vector3 posh) override;

    void Update() override;

    void Draw() override;

public:
    // Get関数
    AllAABB GetAllAABB() const override;
    AllOBB GetAllOBB() const override;
    CollisionGroup GetCollisionGroup() const override { return CollisionGroup::kEnenmy; }
    void OnCollision(Collider* other) override;
    int GetDamage() const override { return dameg_; }
    bool GetIsAvile_() override { return isAvile_; }
    Vector3 GetTranslate() override { return transform_.translate; }

    // set関数
    void SetTargetPlayer(Player* target) override { player_ = target; }
    void SetIsDead(bool num) { isDead_ = num; }
    void SetMove(Vector3 num) override { move = num; }
    void SetbasePos(Vector3 num) override { basePos = num; } // 折り返し地点の中心位置
    void SetUseBullet(int num) override { useBullet = num; }
    void SetHomingPower(float num) override { homingPower = num; }
    void SetHp(int num) override { health_ = num; }

private:
    void BulletUpdate();
    void withdrawalUpdate() override;

private:
    Camera* camera_ = nullptr; // カメラ(ポインタ)
    Player* player_ = nullptr;

    // 3dモデル
    std::unique_ptr<Model> model;
    std::unique_ptr<Object3d> object3d;

    // 当たり判定
    float size = 0.5f; // OBBに移植後は知らん。
    int useBullet = 0; // 使う弾
    float homingPower = 0.0f;

    Transform transform_ = { 0.0f }; // 座標系

    int health_ = 5; // 体力(jsonで設定予定)
    int dameg_ = 3;

    float interval = 3.0f; // 弾を発射する間隔
    static inline const float maxInterval = 3.0f; // 間隔

    Vector3 move = { 0.0f };
    Vector3 basePos = { 0.0f }; // 折り返し地点の中心位置
    float maxBaseMove = 5.0f; // どこまで進むか

    bool isAvile_ = true; // 存在しているか
    bool isDead_ = false; // 死んでいるか
    bool isRanAway_ = false;
    float RanAwayOffset_ = 30.0f;
};