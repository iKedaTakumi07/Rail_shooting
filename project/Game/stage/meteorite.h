#pragma once
#include "../../Engine/3d/Object3d.h"
#include "../../Engine/base/Math.h"
#include <array>
#include <memory>
#include <random>
#include <string>

class Model;
class Camera;
class meteorite {
public:
    void Initialize();

    void Update();

    void Draw();

private:
private:
    Camera* camera_ = nullptr; // カメラポインタ
    std::string objPatan_; // オブジェクトバターン
    std::mt19937 randomEngine;

    static inline const int maxMetrorite = 10;
    std::array<Transform, maxMetrorite> transform_;
    std::array<Vector3, maxMetrorite> Speed = { 0.0f, 0.0f, 0.0f }; // 隕石の速度
    Vector3 ChangePos = { -20.0f, 10.0f, -40.0f };

    std::unique_ptr<Model> ObjectModel;
    std::array<std::unique_ptr<Object3d>, maxMetrorite> Object3d_;
};
