#pragma once
#include "../../../Engine/3d/LightManager.h"
#include "../../../Engine/3d/Object3d.h"
#include "../../../Engine/base/Math.h"
#include <array>
#include <memory>

class Model;

class SelectUIObject {
public:
    enum class SequenceState {
        kLanding, // 着陸演出
        kIdle, // ステージ選択中
        kSortieRise, // 上昇
        kSortieCharge, // 溜め
        kSortieLaunch, // 高速発進
        kTitleBack // タイトルバック
    };

    SelectUIObject();
    ~SelectUIObject();

    static constexpr int maxStage = 2;

    void Initialize();
    void Update(float deltaTime, int uiState);
    void Draw();

    void ChangeStage(int stageIndex);
    void StartSortie(int stageIndex);
    void StartTitleBack();

    // 状態判定・取得
    bool IsMoving() const { return sequenceState_ != SequenceState::kIdle; }
    const Vector3& GetCurrentPlanetPos() const { return currentPlanetPos_; }
    SequenceState GetSequenceState() const { return sequenceState_; }

private:
    void MoveUpdate(float deltaTime);
    void UpdateLightEffect(float deltaTime);

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

    std::array<Vector3, maxStage> stagePos_ { };
    PointLigth pointLight_ { }; // 固有のPointLightパラメータ

    Vector3 currentPlanetPos_ { 0.0f, 0.0f, 0.0f };
    Vector3 startPlanetPos_ { 0.0f, 0.0f, 0.0f };
    Vector3 targetPlanetPos_ { 0.0f, 0.0f, 0.0f };
    Vector3 lanpPos_ = { 0.0f, 4.5f, 4.4f };
    Vector3 lampCover = { 0.0f, 4.5f, 4.0f };

    SequenceState sequenceState_ = SequenceState::kLanding;
    float sequenceTimer_ = 0.0f;

    bool isMoving_ = false;
    float moveTimer_ = 0.0f;
    float moveDuration_ = 0.25f;
    static constexpr float kmoveDuration_ = 0.25f;

    // 出撃タイマー
    static constexpr float kRiseDuration = 0.3f;
    static constexpr float kChargeDuration = 0.5f;
    static constexpr float kLaunchDuration = 0.4f;
    static constexpr float kLandingDuration = 0.8f;
    static constexpr float kTitleBackDuration = 1.0f;

    static constexpr float kRotateDuration = 0.4f; // 旋回時間
    static constexpr float kMoveDuration = 0.6f; // 発進時間
};
