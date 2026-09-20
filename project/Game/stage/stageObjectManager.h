#pragma once
#include "../../Engine/3d/Camera.h"
#include "../../Engine/3d/Model.h"
#include "../../Engine/3d/Object3d.h"
#include "../../Engine/base/Math.h"
#include "stageObject.h"

#include <memory>
#include <vector>
class Player;

struct stageObjectPopData {
    std::string ObjectPatan; // 敵の種類
    float spawn_Z; // 出現するタイミング
    Vector3 popPosition; // 出現場所
    Vector3 sizeScale; // 大きさ
};

class stageObjectManager {
public:
    void Initialize(const std::string& filePath, Player* player);

    void Update();
    void ClearUpdate();

    void Draw();

public:
    void LoadJsonPopData(const std::string& filePath);

    // get
    const std::vector<std::unique_ptr<stageObject>>& GetstageObjects() const { return stageObjects_; };

private:
    void PopEnemyCheck(const stageObjectPopData& data);

    void GrauondInitialize();
    void GrauondUpdate();

private:
    std::vector<stageObjectPopData> popDatas_; // 読み込んだデータ
    std::vector<std::unique_ptr<stageObject>> stageObjects_; // 生きている敵のリスト

    size_t currentSpawnIndex_ = 0; // 次に出現させる敵のインデックス
    std::string PopObjFilePath_; // 現在読み込んでいるファイル

    Transform grauondtransform_ = { { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } }; // 座標
    Transform grauondtransform2_ = { { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
    Transform grauondtransform3_ = { { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
    const float grauondSize = 200.0f;
    int section = 0;

    Player* player_ = nullptr; // ポインタ

    std::unique_ptr<Model> grauondModel;
    std::unique_ptr<Object3d> grauond3d;
    std::unique_ptr<Object3d> grauond3d2;
    std::unique_ptr<Object3d> grauond3d3;
};
