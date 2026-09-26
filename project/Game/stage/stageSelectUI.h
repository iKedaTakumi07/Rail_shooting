#pragma once
#include "../../Engine/3d/Object3d.h"
#include "../../Engine/base/Math.h"
#include <memory>
#include <string>

class Model;
class Camera;
class Sprite;

class stageSelectUI {
public:
    // 初期化
    void Initialize();

    // 毎フレーム更新
    void Update(float deltaTime);

    // 描画
    void Draw();
    void SpriteDraw();

    void ChangeStage(int stageIndex);
    void StartSortie(int stageIndex, float duration);

public:
    // Set関数
    void SetStageNumber(int num) { stageNumber_ = num; }
    void SetisSortie(bool num) { isSortie = num; }
    bool GetisMoving() { return isMoving_; }

private:
    void MoveUpdate(float deltaTime);

private:
    static inline const int maxStage = 2; // ステージ文
    int stageNumber_ = 0;
    bool isSortie = false; // 出撃モーションに変えるかどうか

    std::unique_ptr<Model> ObjectModel; // 惑星テクすちゃ増やすならarray化か?
    std::array<std::unique_ptr<Object3d>, maxStage> Object3d_; // 惑星オブジェクト
    std::array<std::unique_ptr<Sprite>, maxStage> stageSprite; // 惑星名スプライトを表示

    std::array<Vector3, maxStage> stagePos; // ステージ座標

    std::unique_ptr<Model> playerObjectModel; // プレイヤー
    std::unique_ptr<Object3d> playerObject3d_; // プレイヤー

    Vector3 currentPlanetPos_ { 0.0f, 0.0f, 0.0f }; // 現在座標
    Vector3 startPlanetPos_ { 0.0f, 0.0f, 0.0f }; // 補間開始座標
    Vector3 targetPlanetPos_ { 0.0f, 0.0f, 0.0f }; // 補間目標座標

    bool isMoving_ = false; // 移動演出中フラグ
    float moveTimer_ = 0.0f; // 経過時間
    float moveDuration_ = 0.25f;
    const float kmoveDuration_ = 0.25f; // 移動にかかる時間(秒)
};
