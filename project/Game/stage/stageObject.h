#pragma once
#include "../../Engine/base/Math.h"
#include "../OnCollison/Collider.h"
#include <memory>
#include <string>

class Model;
class Object3d;
class Camera;

struct stageObjectPart {
    Vector3 aabbMinOffset; // 当たり判定 (AABB min) オフセット
    Vector3 aabbMaxOffset; // 当たり判定 (AABB max) オフセット
};

class stageObject : public Collider {
public:
    void Initialize(const std::string& patan, const Vector3& pos, const Vector3& scale);

    void Update();

    void Draw();

public:
    // Get
    AllAABB GetAllAABB() const override;
    AllOBB GetAllOBB() const override;
    CollisionGroup GetCollisionGroup() const override { return CollisionGroup::kStageObject; }
    void OnCollision(Collider* other) override;
    int GetDamage() const override { return dameg_; }

    // Set
    void setPatan(std::string patan) { objPatan_ = patan; }

private:
private:
    Camera* camera_ = nullptr; // カメラポインタ
    std::string objPatan_; // オブジェクトバターン

    Transform transform_;

    float baseSize_ = 1.0f; // sclaeが1.0fなら2mなのでsclaeと同じにする
    int dameg_ = 5; // 衝突ダメージ

    std::unique_ptr<Model> ObjectModel;
    std::unique_ptr<Object3d> Object3d_;
    std::array<stageObjectPart, 3> parts_;
};
