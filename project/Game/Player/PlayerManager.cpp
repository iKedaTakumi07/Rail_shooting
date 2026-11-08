#include "PlayerManager.h"
#include "../OnCollison/CollisionManager.h"

void PlayerDodgeCollider::OnCollision(Collider* other)
{
    if (!ownerPlayer_ || ownerPlayer_->IsDead())
        return;

    if (ownerPlayer_->IsInvincible()) {
        return;
    }

    // 敵弾または敵本体と擦れ違った（回避成功）場合
    if (other->GetCollisionGroup() == CollisionGroup::kEnemyBullet || other->GetCollisionGroup() == CollisionGroup::kEnemy) {
        // すでに保持していない場合のみ付与 (上限1回)
        if (!ownerPlayer_->HasInstantCharge()) {
            ownerPlayer_->SetInstantCharge(true);
            // エフェクトやSE再生をここに追加可能
        }
    }
}

void PlayerManager::Initialize()
{
    player_ = std::make_unique<Player>();
    player_->Initialize();

    dodgeCollider_ = std::make_unique<PlayerDodgeCollider>(player_.get());
    dodgeCollider_->ResetPrevPosition(player_->GetTranslate());
}

void PlayerManager::Update()
{
    if (player_) {
        player_->Update();
        dodgeCollider_->UpdatePrevPosition(player_->GetTranslate());
    }
}

void PlayerManager::UpdateIntro(float progress)
{
    if (player_) {
        player_->UpdateIntro(progress);
        dodgeCollider_->ResetPrevPosition(player_->GetTranslate());
    }
}

void PlayerManager::UpdateClear()
{
    if (player_) {
        player_->UpdateClear();
        dodgeCollider_->UpdatePrevPosition(player_->GetTranslate());
    }
}

void PlayerManager::Draw()
{
    if (player_)
        player_->Draw();
}

void PlayerManager::SpritDraw()
{
    if (player_)
        player_->SpritDraw();
}

void PlayerManager::RegisterColliders(CollisionManager* collisionManager)
{
    if (!collisionManager || !player_)
        return;

    // 自機本尊と回避判定の両方を登録
    collisionManager->AddCollider(player_.get());
    for (auto& bullet : player_->GetBullets()) {
        collisionManager->AddCollider(bullet.get());
    }

    collisionManager->AddCollider(dodgeCollider_.get());
}