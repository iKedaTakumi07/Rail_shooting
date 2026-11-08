#pragma once
#include "../OnCollison/Collider.h"
#include "Player.h"
#include <memory>

// 自機回避判定用の独立コライダー
class PlayerDodgeCollider : public Collider {
public:
    PlayerDodgeCollider(Player* player)
        : ownerPlayer_(player)
    {
    }

    AllAABB GetAllAABB() const override { return ownerPlayer_->GetAllAABB(); }
    AllOBB GetAllOBB() const override { return ownerPlayer_->GetDodgeOBB(); }
    CollisionGroup GetCollisionGroup() const override { return CollisionGroup::kPlayerDodge; }

    void OnCollision(Collider* other) override;

private:
    Player* ownerPlayer_ = nullptr;
};

class PlayerManager {
public:
    void Initialize();
    void Update();
    void UpdateIntro(float progress);
    void UpdateClear();
    void Draw();
    void SpritDraw();

    // ゲッター & セッター
    Player* GetPlayer() const { return player_.get(); }
    Collider* GetDodgeCollider() const { return dodgeCollider_.get(); }

    void RegisterColliders(class CollisionManager* collisionManager);

private:
    std::unique_ptr<Player> player_;
    std::unique_ptr<PlayerDodgeCollider> dodgeCollider_;
};