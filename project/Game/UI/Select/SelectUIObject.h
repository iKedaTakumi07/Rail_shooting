#pragma once
#include "../../../Engine/3d/Object3d.h"
#include "../../../Engine/base/Math.h"
#include <array>
#include <memory>

class Model;

class SelectUIObject {
public:
    static constexpr int maxStage = 2;

    void Initialize();
    void Update(float deltaTime, bool isSortie);
    void Draw();

    void ChangeStage(int stageIndex);
    void StartSortie(int stageIndex, float duration);

    // 状態判定・取得
    bool IsMoving() const { return isMoving_; }
    const Vector3& GetCurrentPlanetPos() const { return currentPlanetPos_; }

private:
    void MoveUpdate(float deltaTime, bool isSortie);

private:
    std::unique_ptr<Model> objectModel_;
    std::array<std::unique_ptr<Object3d>, maxStage> object3d_;

    std::unique_ptr<Model> playerObjectModel_;
    std::unique_ptr<Object3d> playerObject3d_;

    std::unique_ptr<Model> baseLabObjectModel_;
    std::unique_ptr<Object3d> baseLabObject3d_;

    std::unique_ptr<Model> lampModel_;
    std::unique_ptr<Object3d> lampObject3d_;

    std::unique_ptr<Model> lampCoverModel_;
    std::unique_ptr<Object3d> lampCoverObject3d_;

    std::array<Vector3, maxStage> stagePos_;

    Vector3 currentPlanetPos_ { 0.0f, 0.0f, 0.0f };
    Vector3 startPlanetPos_ { 0.0f, 0.0f, 0.0f };
    Vector3 targetPlanetPos_ { 0.0f, 0.0f, 0.0f };

    bool isMoving_ = false;
    float moveTimer_ = 0.0f;
    float moveDuration_ = 0.25f;
    static constexpr float kmoveDuration_ = 0.25f;
};
