#pragma once
#include "../../Engine/2d/Sprite.h"
#include "../../Engine/3d/Camera.h"
#include "../../Engine/3d/Model.h"
#include "../../Engine/3d/Object3d.h"
#include "../../Engine/base/Math.h"
#include <list>
#include <memory>

#include "../OnCollison/Collider.h"

#include "PlayerBullet.h"
class EnemyManager;
class baseEnemy;

class Player : public Collider {
public:
    void Initialize();

    void Update();

    void Draw();

    void SpritDraw();

public:
    // Get関数
    Vector3 GetTranslate() const { return basetransform_.translate; } // 座標の取得
    Transform GetTransform() const { return basetransform_; } // 回転角含む
    const std::list<std::unique_ptr<PlayerBullet>>& GetBullets() const { return playerBullets_; } // 弾の入手
    float GetLimitX() const { return kMoveLimitX; }
    float GetLimitY() const { return kMoveLimitY; }

    AllAABB GetAllAABB() const override;
    CollisionGroup GetCollisionGroup() const override { return CollisionGroup::kPlayer; }
    void OnCollision(Collider* other) override;
    int GetDamage() const override { return dameg_; }

    // set
    void SetBasePosition(const Vector3& pos) { railBasePos_ = pos; }
    void SetEnemyManager(EnemyManager* enemyManager) { enemyManager_ = enemyManager; }

private:
    // 更新系列
    void MoveUpdate();
    void HoverUpdate();
    void ReticleUpdate();

    // 弾の制御
    void BulletUpdate();
    void BulletCharge();

    // 体力UIの制御
    void UIUpdate();
    Vector2 WorldToScreen(const Vector3& worldPos, Camera* camera);

    // 押し出し処理
    void ColliderUpdate(Collider* other);

private:
    Transform transform_ = { { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } }; // モデル座標
    Transform basetransform_ = { { 0.5f, 0.5f, 0.5f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } }; // 揺れ成分を含まない座標他
    Vector3 localPos_ = { 0.0f, 0.0f, 0.0f }; // レール中心位置からの座標
    Vector3 railBasePos_ = { 0.0f, 0.0f, 0.0f }; // レール座標

    // 移動速度
    Vector3 velocity_ = { 0.0f, 0.0f, 0.0f };

    // 当たり判定
    float size = 1.0f; // OBBに移植後は知らん。

    // 体力
    int hp_ = 100; // 現体力
    int Maxhp_ = 100; // 最大体力

    // 被弾時の無敵時間
    const float KinvincibleTime = 1.0f;
    float invincibleTime = 1.0f;
    bool isinvincible = false;

    // 移動系パラメータ
    const float kCharacterSpeed = 0.4f; // 最高速度
    const float kAcceleration = 0.02f; // 加速度
    const float shiftUpSpeed = 1.25f; // シフト(高速旋回)乗算倍率
    const float kFriction = 0.87f; // 摩擦抵抗

    // 移動限界座標(仮定)
    const float kMoveLimitX = 8.0f;
    const float kMoveLimitY = 5.0f;

    // 機体の傾き
    float rollFactor = 0.8f;
    float shiftRollFactor = 1.4f;

    // 静止時の揺れ
    const float kHoverSpeed = 2.5f; // 浮遊の速さ（周波数）
    const float kHoverAmount = 0.015f; // 浮遊の揺れ幅（上下移動量）
    const float kSwaySpeed = 4.0f; // 揺れる速さ（周波数）
    const float kSwayAmountZ = 0.025f; // Roll（左右の傾き）の揺れ幅
    const float kSwayAmountX = 0.015f; // Pitch（前後の傾き）の揺れ幅
    float idleTimer_ = 0.0f; // 揺れタイマー

    // 弾の詳細設定(チャージショット一連の操作、チュートリアルを作成後作成)
    const float kCoolTime = 0.20f;
    float coolTime = 0.0f;
    int dameg_ = 10;
    int chargeDameg_ = 15;

    // チャージショット
    float chargeTimer_ = 0.0f; // チャージ時間
    const float kChargeTime = 1.0f; // チャージ完了までの時間
    const float kLockonAngleThreshold = 0.99f; // ロックオン範囲(円錐)<0.0fが90°,0.99fが約11°>
    uint32_t lockonTargetId_ = 0; // ロックオン対象
    int lockonTargetIndex_ = 0; // ロックオン対象のパーツ番号
    uint32_t ChageLookId_ = 0; // ロックオン対象
    int ChageLookIndex_ = 0; // ロックオン対象のパーツ番号

    // 3d照準の距離
    const float kLongDistancePlayerTo3DReticle = 50.0f; // 最長射程
    const float kShortDistancePlayerTo3DReticle = 25.0f; // 半分ぐらいの距離
    bool ChageLook_ = false;

    // 3dモデル
    std::unique_ptr<Model> playerModel;
    std::unique_ptr<Object3d> playerObject3d;

    std::unique_ptr<Model> ShortReticleModel;
    std::unique_ptr<Object3d> ShortReticleObject3d;

    std::unique_ptr<Model> LongReticleModel;
    std::unique_ptr<Object3d> LongReticleObject3d;

    std::unique_ptr<Sprite> ChargeReticleSprite;

    // UI(スプライト)
    std::unique_ptr<Sprite> PlayerMaxHpUI;
    std::unique_ptr<Sprite> PlayerHpUI;

    // カメラ
    Camera* camera_;
    EnemyManager* enemyManager_ = nullptr;

    // 弾
    std::list<std::unique_ptr<PlayerBullet>> playerBullets_;
};