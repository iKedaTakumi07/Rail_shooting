#pragma once
#include "../base/baseEnemy.h"
#include <memory>

#include "../../../Engine/3d/Object3d.h"
#include "../../../Engine/base/Math.h"

#include "../../OnCollison/Collider.h"
#include "../../Player/Player.h"

class Model;
class Camera;

class threeShotsEnemy : public baseEnemy {
    void Initialize(Vector3 pos) override;

    void Update() override;

    void Draw() override;

public:
    // Get関数
    AllAABB GetAllAABB() const override;
    CollisionGroup GetCollisionGroup() const override { return CollisionGroup::kEnenmy; }
    void OnCollision(Collider* other) override;
    int GetDamage() const override { return dameg_; }
    bool GetIsAvile_() override { return isAvile_; }
    Vector3 GetTranslate() override { return transform_.translate; }

    // set関数
    void SetTargetPlayer(Player* target) override { player_ = target; }
    void SetIsDead(bool num) { isDead_ = num; }
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
    int dameg_ = 5;

    float interval = 2.0f; // 弾を発射する間隔
    static inline const float maxInterval = 2.0f; // 間隔

    // 削除予定 //
    Vector3 move = { 0.0f };
    // 移動地点はjson形式予定。 //

    bool isAvile_ = true; // 存在しているか
    bool isDead_ = false; // 死んでいるか
    bool isRanAway_ = false;
    float RanAwayOffset_ = 20.0f;

    std::array<Vector3, 3> muzzleOffsets_ = {
        Vector3 { 1.0f, 0.0f, 0.0f },
        Vector3 { 0.0f, 0.0f, 0.0f },
        Vector3 { -1.0f, 0.0f, 0.0f }
    };
};
