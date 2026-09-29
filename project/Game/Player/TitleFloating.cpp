#include "TitleFloating.h"

#include "../../Engine/3d/ModelManager.h"
#include "../../Engine/3d/Object3d.h"
#include "../../Engine/base/Math.h"
#include "../../Engine/base/TextureManager.h"
#include "../../Engine/scene/SceneManager.h"
#include <algorithm>

void TitleFloating::Initialize()
{
    TextureManager::getInstance()->LoadTexture("resources/player/1x1white.png");
    ModelManager::GetInstance()->LoadModel("player/Player.obj");

    pattern_ = State::kIntro;
    MovePattern_ = MoveState::knull;

    std::random_device seedGenerator;
    randomEngine = std::mt19937(seedGenerator());

    playerObject3d = std::make_unique<Object3d>();
    playerObject3d->Initialize();

    playerModel = std::make_unique<Model>();
    playerModel->Initialize("resources/player", "Player.obj");
    playerObject3d->SetModel(playerModel.get());

    transform_.rotate.y = std::numbers::pi_v<float> * 0.5f;
    playerObject3d->SetScale(transform_.scale);
    playerObject3d->SetRotate(transform_.rotate);

    IntroTimer = kIntroTimer;
}

void TitleFloating::Update()
{
    float deltaTime = SceneManager::GetInstance()->GetDeltaTime();
    switch (pattern_) {
    case TitleFloating::State::kStay:
        MoveUpdate(deltaTime);
        BulletUpdate(deltaTime);
        if (isSortie) {
            pattern_ = State::kSortie;
            StartPos = transform_.translate;
        }
        break;
    case TitleFloating::State::kIntro:
        IntroUpdate(deltaTime);
        if (isSortie) {
            pattern_ = State::kSortie;
            StartPos = transform_.translate;
        }
        break;
    case TitleFloating::State::kSortie:
        SortieUpdate(deltaTime);
        break;
    }

    playerObject3d->SetTranslate(transform_.translate);
    playerObject3d->SetRotate(transform_.rotate);
    playerObject3d->Update();
}

void TitleFloating::Draw()
{
    playerObject3d->Draw();
}

void TitleFloating::IntroUpdate(float deltaTime)
{
    IntroTimer -= deltaTime;
    if (IntroTimer <= 0.0f) {
        pattern_ = State::kStay;
    }

    float progress = 1.0f - (IntroTimer / kIntroTimer);
    progress = std::clamp(progress, 0.0f, 1.0f);

    float easeT = EaseOutCubic(progress);

    transform_.translate = Lerp(StartPos, EndPos, easeT);
}

void TitleFloating::SortieUpdate(float deltaTime)
{
    SortieTimer -= deltaTime;

    float progress = 1.0f - (IntroTimer / kIntroTimer);
    progress = std::clamp(progress, 0.0f, 1.0f);

    float easeT = EaseOutCubic(progress);

    transform_.translate = Lerp(StartPos, EndPos, easeT);
}

void TitleFloating::MoveUpdate(float deltaTime)
{
    // 行動変更
    patternInterval -= deltaTime;
    if (patternInterval <= 0.0f) {
        std::uniform_int_distribution<int> dist(0, 3);
        patternInterval = kpatternInterval;
        MovePattern_ = MoveState(dist(randomEngine));
    }

    Vector3 inputDir = { 0, 0, 0 };
    int isShift = false;

    switch (MovePattern_) {
    case TitleFloating::MoveState::knull:
        // なんもしない
        break;
    case TitleFloating::MoveState::kLeftRoll:
        inputDir.z -= 1.0f;
        break;
    case TitleFloating::MoveState::kRightRoll:
        inputDir.z += 1.0f;
        break;
    case TitleFloating::MoveState::kLeftShiftRoll:
        isShift = true;
        inputDir.z -= 1.0f;
        break;
    case TitleFloating::MoveState::kRightShiftRoll:
        isShift = true;
        inputDir.z += 1.0f;
        break;
    }

    // 上昇下降の処理
    UpDownSwitchTimer -= deltaTime;
    if (UpDownSwitchTimer <= 0.0f) {
        dirY = -1.0f * dirY;
        std::uniform_real_distribution<float> dist(2.0f, 3.0f);
        UpDownSwitchTimer = dist(randomEngine);
    }

    // x軸の更新
    MoveSwitchTimer -= deltaTime;
    if (MoveSwitchTimer <= 0.0f) {
        std::uniform_real_distribution<float> dist(2.0f, 3.0f);
        std::uniform_real_distribution<float> Dirdist(-1.0f, 0.5f);
        MoveSwitchTimer = dist(randomEngine);
        dirX = Dirdist(randomEngine);
    }

    inputDir.y = dirY;
    inputDir.x = dirX;

    float length = std::sqrt(inputDir.z * inputDir.z + inputDir.y * inputDir.y);
    if (length > 0.0f) {
        inputDir.z /= length;
        inputDir.y /= length;
    }

    float currentAccel = isShift ? kAcceleration * shiftUpSpeed : kAcceleration; // 加速度
    float currentMaxSpeed = isShift ? kCharacterSpeed * shiftUpSpeed : kCharacterSpeed; // 速度

    // 指定方向に加速
    velocity_.z += inputDir.z * currentAccel;
    velocity_.y += inputDir.y * currentAccel;
    velocity_.x += inputDir.x * currentAccel;

    // 摩擦による減速
    velocity_.z *= kFriction;
    velocity_.y *= kFriction;
    velocity_.x *= kFriction;

    // 最高速の制限
    float speed = std::sqrt(velocity_.z * velocity_.z + velocity_.y * velocity_.y + velocity_.x * velocity_.x);
    if (speed > currentMaxSpeed) {
        velocity_.z = (velocity_.z / speed) * currentMaxSpeed;
        velocity_.y = (velocity_.y / speed) * currentMaxSpeed;
        velocity_.x = (velocity_.x / speed) * currentMaxSpeed;
    }

    localPos_.x += velocity_.x;
    localPos_.y += velocity_.y;
    localPos_.z += velocity_.z;

    // オーバーしていたら戻す
    localPos_.x = std::clamp(localPos_.x, minPos.x, maxPos.x);
    localPos_.y = std::clamp(localPos_.y, minPos.y, maxPos.y);
    localPos_.z = std::clamp(localPos_.z, minPos.z, maxPos.z);

    transform_.translate.x =  localPos_.x;
    transform_.translate.y = localPos_.y;
    transform_.translate.z =  localPos_.z;

    RoateUpdate(deltaTime, currentAccel, isShift);
    BulletUpdate(deltaTime);
}

void TitleFloating::RoateUpdate(float deltaTime, float currentAccel, bool isShift)
{
    float maxTheoreticalSpeed = currentAccel / (1.0f - kFriction);

    float ratioX = std::clamp(velocity_.x / maxTheoreticalSpeed, -1.0f, 1.0f);
    float ratioY = std::clamp(velocity_.y / maxTheoreticalSpeed, -1.0f, 1.0f);
    float ratioZ = std::clamp(velocity_.z / maxTheoreticalSpeed, -1.0f, 1.0f);

    float targetRotateX = 0.0f;
    float targetRotateY = std::numbers::pi_v<float> * 0.5f;
    float targetRotateZ = 0.0f;

    if (isShift) {
        targetRotateZ = ratioZ * kMaxRollShift;
        targetRotateX = ratioY * kMaxPitchAngle;
        targetRotateY += (-ratioZ * kMaxYawAngle) * kShiftYawFactor;
    } else {
        targetRotateZ = ratioZ * kMaxRollNormal;
        targetRotateX = ratioY * kMaxPitchAngle;
        targetRotateY += -ratioZ * kMaxYawAngle;
    }

    // 補間処理 (フレームレート非依存)
    float lerpSpeed = 8.0f;
    float t = 1.0f - std::exp(-lerpSpeed * deltaTime);

    transform_.rotate.x += (targetRotateX - transform_.rotate.x) * t;
    transform_.rotate.y += (targetRotateY - transform_.rotate.y) * t;
    transform_.rotate.z += (targetRotateZ - transform_.rotate.z) * t;
}

void TitleFloating::BulletUpdate(float deltaTime)
{
}
