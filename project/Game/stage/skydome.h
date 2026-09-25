#pragma once
#include "../../Engine/3d/Object3d.h"
#include "../../Engine/base/Math.h"
#include <memory>
#include <string>

class Model;
class Camera;

class skydome {
public:
    void Initialize();

    void Update();

    void Draw();

private:
private:
    Camera* camera_ = nullptr; // カメラポインタ
    std::string objPatan_; // オブジェクトバターン

    Transform transform_;

    std::unique_ptr<Model> ObjectModel;
    std::unique_ptr<Object3d> Object3d_;
};
