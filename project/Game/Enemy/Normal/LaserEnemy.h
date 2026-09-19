#pragma once
#include "../base/baseEnemy.h"
#include <memory>

#include "../../../Engine/3d/Object3d.h"
#include "../../../Engine/base/Math.h"

#include "../../OnCollison/Collider.h"
#include "../../Player/Player.h"

class Model;

class LaserEnemy : public baseEnemy {
public:
    void Initialize(Vector3 pos) override;

    void Update() override;

    void Draw() override;

public:
    // Get関数
    AllAABB GetAllAABB() const override;
    AllOBB GetAllOBB() const override;
    CollisionGroup GetCollisionGroup() const override { return CollisionGroup::kEnenmy; }
    void OnCollision(Collider* other) override;
    int GetDamage() const override { return dameg_; }
    bool GetIsAvile_() override { return AllisAvile_; }
    Vector3 GetTranslate() override { return startingPointtransform_.translate; }
    std::vector<Vector3> GetTargetPositions() override; // ホーミング用の座標渡し

    // set関数
    void SetTargetPlayer(Player* target) override { player_ = target; }
    void SetLastStopPos(Vector3 num) override { lastStoptransform_.translate = num; }
    void SetIsDead(bool num) { AllisAvile_ = num; }
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
    std::unique_ptr<Model> model;
    std::unique_ptr<Object3d> startingPointObject3d; // 始点側のオブジェ
    std::unique_ptr<Object3d> lastStopObject3d; // 終点側のオブジェ

    // 当たり判定
    float size = 0.5f; // OBBに移植後は知らん。
    int useBullet = 0; // 使う弾
    float homingPower = 0.0f;

    Transform startingPointtransform_ = { 0.0f }; // レーザビーム始点側座標系
    Transform lastStoptransform_ = { 0.0f }; // レーザビーム終点がわ座標系

    int starthealth_ = 6; // 体力(jsonで設定予定)
    int lasthealth_ = 6; // 体力(jsonで設定予定)
    int dameg_ = 5;

    // 削除予定 //
    Vector3 move = { 0.0f };

    float interval = 5.0f; // 弾を発射する間隔
    static inline const float maxInterval = 5.0f; // 間隔

    bool startisAvile_ = true; // 存在しているか
    bool startisDead_ = false; // 死んでいるか

    bool lastisAvile_ = true; // 存在しているか
    bool lastisDead_ = false; // 死んでいるか

    bool AllisAvile_ = true; // 両方存在しているか
    bool AllisDead_ = false; // 両方死んでいるか

    float RanAwayOffset_ = 30.0f;
};