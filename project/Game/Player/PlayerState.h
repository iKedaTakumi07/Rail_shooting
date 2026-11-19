#pragma once
class Player;
class Collider;

/// <summary>
/// Player 行動状態の共通基底インターフェース
/// </summary>
class IPlayerState {
public:
    virtual ~IPlayerState() = default;

    virtual void Enter(Player& player) { (void)player; }
    virtual void Update(Player& player, float deltaTime) = 0;
    virtual void Exit(Player& player) { (void)player; }
    virtual void OnCollision(Player& player, Collider* other)
    {
        (void)player;
        (void)other;
    }

    // 状態クエリ（外部および描画制御用）
    virtual bool IsDeadState() const { return false; }
    virtual bool IsExplosionState() const { return false; }
    virtual bool IsInvincibleState() const { return false; }
    virtual bool ShouldDrawModel() const { return true; }
    virtual bool ShouldDrawHud() const { return true; }
};

/// <summary>
/// イントロ演出状態（操作不能・レール追従・UI非表示）
/// </summary>
class PlayerIntroState : public IPlayerState {
public:
    void Update(Player& player, float deltaTime) override;
    void UpdateWithProgress(Player& player, float progress);
    bool ShouldDrawHud() const override { return false; }
};

/// <summary>
/// 通常操作状態
/// </summary>
class PlayerNormalState : public IPlayerState {
public:
    void Update(Player& player, float deltaTime) override;
    void OnCollision(Player& player, Collider* other) override;
};

/// <summary>
/// 被弾無敵点滅状態
/// </summary>
class PlayerInvincibleState : public IPlayerState {
public:
    void Enter(Player& player) override;
    void Update(Player& player, float deltaTime) override;
    void Exit(Player& player) override;
    bool IsInvincibleState() const override { return true; }

private:
    float timer_ = 0.0f;
};

/// <summary>
/// 撃墜・墜落状態（2秒後に自動爆発、または地形激突で即爆発）
/// </summary>
class PlayerDeadState : public IPlayerState {
public:
    void Enter(Player& player) override;
    void Update(Player& player, float deltaTime) override;
    void OnCollision(Player& player, Collider* other) override;
    bool IsDeadState() const override { return true; }
};

/// <summary>
/// 機体爆破状態（自機非表示・黒煙パーティクル＋残弾更新のみ）
/// </summary>
class PlayerExplosionState : public IPlayerState {
public:
    void Enter(Player& player) override;
    void Update(Player& player, float deltaTime) override;
    bool IsDeadState() const override { return true; }
    bool IsExplosionState() const override { return true; }
    bool ShouldDrawModel() const override { return false; }
};

/// <summary>
/// ステージクリア演出状態（無敵・前方上昇・UI非表示）
/// </summary>
class PlayerClearState : public IPlayerState {
public:
    void Update(Player& player, float deltaTime) override;
    bool ShouldDrawHud() const override { return false; }
};