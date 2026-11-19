#include "PlayerState.h"
#include "../OnCollison/Collider.h"
#include "Player.h"
#include <cmath>
#include <memory>

// PlayerIntroState
void PlayerIntroState::Update(Player& player, float deltaTime)
{
    (void)player;
    (void)deltaTime;
}

void PlayerIntroState::UpdateWithProgress(Player& player, float progress)
{
    player.ExecuteIntroStep(progress);
    if (progress >= 1.0f) {
        player.ChangeState(std::make_unique<PlayerNormalState>());
    }
}

// PlayerNormalState
void PlayerNormalState::Update(Player& player, float deltaTime)
{
    player.ExecuteActiveControlStep(deltaTime);
    player.ExecuteCommonAliveStep(deltaTime);
}

void PlayerNormalState::OnCollision(Player& player, Collider* other)
{
    player.HandleActiveCollision(other);
}

// PlayerInvincibleState
void PlayerInvincibleState::Enter(Player& player)
{
    timer_ = player.GetInvincibleDuration();
}

void PlayerInvincibleState::Update(Player& player, float deltaTime)
{
    timer_ -= deltaTime;
    player.UpdateInvincibleBlink(timer_);

    if (timer_ <= 0.0f) {
        player.ChangeState(std::make_unique<PlayerNormalState>());
        player.ExecuteActiveControlStep(deltaTime);
        player.ExecuteCommonAliveStep(deltaTime);
        return;
    }

    player.ExecuteActiveControlStep(deltaTime);
    player.ExecuteCommonAliveStep(deltaTime);
}

void PlayerInvincibleState::Exit(Player& player)
{
    player.ResetMaterialColor();
}

// PlayerDeadState
void PlayerDeadState::Enter(Player& player)
{
    player.DeadInitialize();
}

void PlayerDeadState::Update(Player& player, float deltaTime)
{
    bool shouldExplode = player.UpdateDeadStep(deltaTime);
    if (shouldExplode) {
        player.ChangeState(std::make_unique<PlayerExplosionState>());
        return;
    }
    player.ExecuteCommonAliveStep(deltaTime);
}

void PlayerDeadState::OnCollision(Player& player, Collider* other)
{
    if (other->GetCollisionGroup() == CollisionGroup::kStageObject) {
        player.ChangeState(std::make_unique<PlayerExplosionState>());
    }
}

// PlayerExplosionState
void PlayerExplosionState::Enter(Player& player)
{
    player.explosionInitialize();
}

void PlayerExplosionState::Update(Player& player, float deltaTime)
{
    (void)deltaTime;
    player.UpdateExplosion();
}

// PlayerClearState
void PlayerClearState::Update(Player& player, float deltaTime)
{
    player.ExecuteClearStep(deltaTime);
}