#pragma once
class resultScene;

class IResultSceneState {
public:
    virtual ~IResultSceneState() = default;

    virtual void Enter(resultScene& scene) { (void)scene; }
    virtual void Update(resultScene& scene, float deltaTime) = 0;
    virtual void Exit(resultScene& scene) { (void)scene; }
};

class SceneChange : public IResultSceneState {
public:
    void Enter(resultScene& scene) override;
    void Update(resultScene& scene, float deltaTime) override;

private:
    float stateTimer_ = 1.0f;
};

class PlayerView : public IResultSceneState {
public:
    void Update(resultScene& scene, float deltaTime) override;

private:
    float stateTimer_ = 1.0f;
};

class ResultUpdateNow : public IResultSceneState {
public:
    void Enter(resultScene& scene) override;
    void Update(resultScene& scene, float deltaTime) override;
};

class ResultWait : public IResultSceneState {
public:
    void Update(resultScene& scene, float deltaTime) override;
};

class PlanetEscape : public IResultSceneState {
public:
    void Enter(resultScene& scene) override;
    void Update(resultScene& scene, float deltaTime) override;
};

class ReturnSelect : public IResultSceneState {
public:
    void Enter(resultScene& scene) override;
    void Update(resultScene& scene, float deltaTime) override;

private:
    float stateTimer_ = 1.0f;
};