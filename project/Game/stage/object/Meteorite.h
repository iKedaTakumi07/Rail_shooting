#pragma once
#include "../../../Engine/3d/Object3d.h"
#include "../../../Engine/base/Math.h"
#include <array>
#include <memory>
#include <random>
#include <string>

class Model;
class Camera;
class Meteorite {
public:
    void Initialize();

    void Update();

    void Draw();

private:
    float randomFloat(const float min, const float max)
    {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(randomEngine);
    };

private:
    Camera* camera_ = nullptr; // カメラポインタ
    std::string objPatan_; // オブジェクトバターン
    std::mt19937 randomEngine;

    static inline const int maxFrontSideMetrorite = 5;
    static inline const int maxBackSideMetrorite = 15;

    std::unique_ptr<Model> ObjectModel;
    Vector3 ChangePos = { -20.0f, 10.0f, -40.0f };
    Vector3 speedMin = { -3.0f, -1.0f, -1.0f };
    Vector3 speedMax = { -0.5f, 1.0f, 1.0f };

    Vector3 randomeFrontSidePosMax = { 40.0f, 10.0f, -11.0f };
    Vector3 randomeFrontSidePosMin = { 20.0f, -10.0f, -20.0f };
    std::array<Transform, maxFrontSideMetrorite> frontSidetransform_;
    std::array<std::unique_ptr<Object3d>, maxFrontSideMetrorite> frontSideMetroriteObject_;
    std::array<Vector3, maxFrontSideMetrorite> frontSideSpeed = { 0.0f, 0.0f, 0.0f }; // 隕石の速度

    Vector3 randomeBackSidePosMax = { 40.0f, 10.0f, 22.0f };
    Vector3 randomeBackSidePosMin = { 20.0f, -10.0f, 11.0f };
    std::array<Transform, maxBackSideMetrorite> backSidetransform_;
    std::array<std::unique_ptr<Object3d>, maxBackSideMetrorite> backSideMetroriteObject_;
    std::array<Vector3, maxBackSideMetrorite> backSideSpeed = { 0.0f, 0.0f, 0.0f }; // 隕石の速度
};
