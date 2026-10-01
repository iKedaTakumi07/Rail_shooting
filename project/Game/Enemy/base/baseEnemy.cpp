#include "baseEnemy.h"
#include "../Bullet/LaserBeamBullet.h"
#include <vector>
#include "baseEnemyBullet.h"

void baseEnemy::Initialize(Vector3 pos)
{
}

void baseEnemy::Update()
{
}

void baseEnemy::Draw()
{
}

void baseEnemy::AddBullet(std::unique_ptr<baseEnemyBullet> bullet)
{
    if (bullet) {
        enemyBullet_.push_back(std::move(bullet));
    }
}

void baseEnemy::RemoveBulletsIf(const std::function<bool(const baseEnemyBullet*)>& predicate)
{
    std::erase_if(enemyBullet_, [&predicate](const std::unique_ptr<baseEnemyBullet>& bullet) {
        return predicate(bullet.get());
    });
}

void baseEnemy::UpdateBullets(float deltaTime, const Vector3& playerPos)
{
    for (auto& bullet : enemyBullet_) {
        bullet->SetPlayerPos(playerPos);
        bullet->Update(deltaTime);
    }
    std::erase_if(enemyBullet_, [](const std::unique_ptr<baseEnemyBullet>& bullet) {
        return bullet->GetIsDead();
    });
}

void baseEnemy::DrawBullets() const
{
    for (auto& bullet : enemyBullet_) {
        bullet->Draw();
    }
}