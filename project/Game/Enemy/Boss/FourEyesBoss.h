#pragma once
#include "../../OnCollison/Collider.h"
#include "../base/baseBossEnemy.h "
#include <memory>
#include <numbers>

class Player;
class Model;
class Object3d;
class Sprite;

struct BossPart {
    Vector3 muzzleOffset; // 弾発射位置オフセット
    Vector3 aabbMinOffset; // 当たり判定 (AABB min) オフセット
    Vector3 aabbMaxOffset; // 当たり判定 (AABB max) オフセット
    int hp = 200; // 部位耐久力
};

class FourEyesBoss : public baseBossEnemy {
public:
    void Initialize(Vector3 pos) override;
    void Update() override;
    void Draw() override;
    void SpriteDraw() override;

public:
    void SetTargetPlayer(Player* target) override { player_ = target; }
    void SetIsDead(bool num) { isDead_ = num; }
    void SetMove(Vector3 num) override { num; }
    void SetbasePos(Vector3 num) override { centerPos_ = num; } // 中心位置
    void SetHp(int num) override;

    float GetHpRate() const override { return static_cast<float>(currentHp_) / maxHp_; }
    int GetCurrentPhase() const override { return currentPhase_; }

    AllAABB GetAllAABB() const override;
    CollisionGroup GetCollisionGroup() const override { return CollisionGroup::kEnenmy; }
    std::vector<Vector3> GetTargetPositions() override; // ホーミング用の座標渡し
    void OnCollision(Collider* other) override;
    int GetDamage() const override { return dameg_; }
    bool GetIsAvile_() override { return isAvile_; }
    Vector3 GetTranslate() override { return transform_.translate; }

public:
    void StartAppearance() override;
    void UpdateAppearance(float deltaTime) override;
    bool IsAppearing() const override { return isAppearing_; }

    void StartDeathProduction() override;
    void UpdateDeathProduction(float deltaTime) override;

private:
    void FireFourWayBullets();
    void MoveUpdate();
    void UIUpdate();
    void partsDamage(Collider* other);

private:
    Camera* camera_ = nullptr; // カメラ(ポインタ)
    Player* player_ = nullptr;

    // 3dモデル
    std::unique_ptr<Model> model;
    std::unique_ptr<Object3d> object3d;

    // UI(スプライト)
    std::unique_ptr<Sprite> BossMaxHpUI;
    std::unique_ptr<Sprite> BossHpUI;
    std::unique_ptr<Sprite> BossWarning;

    float appearanceTimer_ = 0.0f;
    const float kAppearanceDuration = 3.0f;
    float deathTimer = 0.0f;
    const float kdeathTimer = 5.0f;
    Vector3 deadPos_ = { 0.0f, 0.0f, 0.0f };

    const float kStartOffsetY = 40.0f;
    const float kStartRotateY = std::numbers::pi_v<float>;

    Transform transform_ = { 0.0f }; // 座標系
    Vector3 centerPos_ = { 0.0f }; // 中心位置

    int dameg_ = 5;
    bool isAvile_ = true; // 存在しているか
    bool isDead_ = false; // 死んでいるか
    bool isDeadMoveCompletion_ = false;

    float interval = 3.0f; // 弾を発射する間隔
    static inline const float maxInterval = 3.0f; // 間隔
    const float offsetPosZ = 50.0f; // カメラと離す距離

    float moveTimer_ = 0.0f; // 移動計算用タイマー
    const float kMoveSpeed = 0.5f; // 8の字周回スピード
    const float kAmplitudeX = 10.0f; // 横幅（X軸方向の振幅）
    const float kAmplitudeY = 5.0f; // 縦幅（Y軸方向の振幅）

    // 発射位置,耐久力、当たり判定offset(各頂点の中心位置)
    std::array<BossPart, 4> parts_;
};
