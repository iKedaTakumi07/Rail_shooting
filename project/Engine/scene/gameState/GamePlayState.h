#pragma once
class GamePlayScene;

class IGamePlayState {
public:
    virtual ~IGamePlayState() = default;

    virtual void Enter(GamePlayScene& scene) { (void)scene; }
    virtual void Update(GamePlayScene& scene, float deltaTime) = 0;
    virtual void Exit(GamePlayScene& scene) { (void)scene; }

    // ポーズ割り込み・復帰時のフック
    virtual void OnPause(GamePlayScene& scene) { (void)scene; }
    virtual void OnResume(GamePlayScene& scene) { (void)scene; }

    // フェーズ固有の描画フック
    virtual void DrawDebug(GamePlayScene& scene) { (void)scene; }
    virtual void DrawSprite(GamePlayScene& scene) { (void)scene; }
};

class IntroState : public IGamePlayState {
public:
    void Update(GamePlayScene& scene, float deltaTime) override;
    void OnPause(GamePlayScene& scene) override;
    void OnResume(GamePlayScene& scene) override;
};

class PlayState : public IGamePlayState {
public:
    void Update(GamePlayScene& scene, float deltaTime) override;
    void DrawDebug(GamePlayScene& scene) override;
};

class DeadState : public IGamePlayState {
public:
    void Enter(GamePlayScene& scene) override;
    void Update(GamePlayScene& scene, float deltaTime) override;
};

class GameOverState : public IGamePlayState {
public:
    explicit GameOverState(bool isImmediateExplosion = false)
        : isImmediateExplosion_(isImmediateExplosion)
    {
    }

    void Enter(GamePlayScene& scene) override;
    void Update(GamePlayScene& scene, float deltaTime) override;
    void DrawSprite(GamePlayScene& scene) override;

private:
    bool isImmediateExplosion_ = false;
};

class PauseState : public IGamePlayState {
public:
    void Update(GamePlayScene& scene, float deltaTime) override;

private:
    bool isChange_ = false;
    bool isReset_ = false;
    float changeTimer_ = 0.5f;
};

class ClearState : public IGamePlayState {
public:
    void Enter(GamePlayScene& scene) override;
    void Update(GamePlayScene& scene, float deltaTime) override;

private:
    float clearTimer_ = 0.0f;
};