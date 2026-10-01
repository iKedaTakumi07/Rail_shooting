#pragma once
#include "../base/baseEnemy.h"
#include <memory>

#include "../../../Engine/3d/Object3d.h"
#include "../../../Engine/base/Math.h"

#include "../../OnCollison/Collider.h"
#include "../../Player/Player.h"

class Model;
class Camera;

class LaserEnemy : public baseEnemy {
public:
    void Initialize(Vector3 pos) override;

    void Update() override;

    void Draw() override;

public:
    // Get関数
    AllAABB GetAllAABB() const override;
    AllOBB GetAllOBB() const override;
    CollisionGroup GetCollisionGroup() const override { return CollisionGroup::kEnemy; }
    void OnCollision(Collider* other) override;
    int GetDamage() const override { return dameg_; }
    bool GetIsAvile_() override { return allIsAvile_; }
    Vector3 GetTranslate() override { return FromPointTransform_.translate; }
    std::vector<Vector3> GetTargetPositions() override; // ホーミング用の座標渡し

    // set関数
    void SetTargetPlayer(Player* target) override { player_ = target; }
    void SetLastStopPos(Vector3 num) override { ToStopTransform_.translate = num; }
    void SetIsDead(bool num) { allIsAvile_ = num; }
    void SetUseBullet(int num) override { useBullet = num; }
    void SetHomingPower(float num) override { homingPower = num; }
    void SetHp(int num) override
    {
        // 両方同じに
        starthealth_ = num;
        lasthealth_ = num;
    }

private:
    void BulletUpdate();

    void partsDamage(Collider* other);

private:
    Camera* camera_ = nullptr; // カメラ(ポインタ)
    Player* player_ = nullptr;

    // 3dモデル
    std::unique_ptr<Model> laserModel;
    std::unique_ptr<Object3d> fromPointObject3d; // 始点側のオブジェ
    std::unique_ptr<Object3d> toStopObject3d; // 終点側のオブジェ

    bool isLaserCleared_ = false;

    // 当たり判定
    float size = 0.5f; // OBBに移植後は知らん。
    int useBullet = 0; // 使う弾
    float homingPower = 0.0f;

    Transform FromPointTransform_ = { 0.0f }; // レーザビーム始点側座標系
    Transform ToStopTransform_ = { 0.0f }; // レーザビーム終点がわ座標系

    int starthealth_ = 6; // 体力
    int lasthealth_ = 6; // 体力
    int dameg_ = 5;

    // 削除予定 //
    Vector3 move = { 0.0f };

    float interval = 5.0f; // 弾を発射する間隔
    static inline const float maxInterval = 5.0f; // 間隔

    bool fromIsAvile_ = true; // 存在しているか
    bool fromIsDead_ = false; // 死んでいるか

    bool toIsAvile_ = true; // 存在しているか
    bool toIsDead_ = false; // 死んでいるか

    bool allIsAvile_ = true; // 両方存在しているか
    bool allIsDead_ = false; // 両方死んでいるか

    float RanAwayOffset_ = 30.0f;
};