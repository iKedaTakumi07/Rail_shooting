#include "stageSelectUI.h"
#include "../../Engine/2d/Sprite.h"
#include "../../Engine/3d/Camera.h"
#include "../../Engine/3d/CameraManager.h"
#include "../../Engine/3d/ModelManager.h"
#include "../../Engine/base/TextureManager.h"
#include "../../Engine/base/WinApp.h"

void stageSelectUI::Initialize()
{
    TextureManager::getInstance()->LoadTexture("resources/skydone/sky_sphere.png");
    TextureManager::getInstance()->LoadTexture("resources/player/1x1white.png");

    TextureManager::getInstance()->LoadTexture("resources/UI/SelectUIStage1.png"); // ステージ名
    TextureManager::getInstance()->LoadTexture("resources/UI/SelectUIStage2.png");
    TextureManager::getInstance()->LoadTexture("resources/UI/SelectUI1.png");

    ModelManager::GetInstance()->LoadModel("player/Player.obj");
    ModelManager::GetInstance()->LoadModel("skydone/Selectskydome.obj");

    // 惑星の座標
    stagePos[0] = { 0.0f, 0.0f, 10.0f };
    stagePos[1] = { 15.0f, 0.0f, 10.0f };

    ObjectModel = std::make_unique<Model>();
    ObjectModel->Initialize("resources/skydone", "Selectskydome.obj"); // 指定したパターンに
    for (int i = 0; i < maxStage; i++) {
        Object3d_[i] = std::make_unique<Object3d>();
        Object3d_[i]->Initialize();
        Object3d_[i]->SetModel(ObjectModel.get());

        Object3d_[i]->SetTranslate(stagePos[i]);
        Object3d_[i]->SetScale(Vector3(3.0f, 3.0f, 3.0f));
        Object3d_[i]->SetRotate(Vector3(0.0f, 0.0f, 0.0f));
    }

    // プレイヤーの機体
    playerObject3d_ = std::make_unique<Object3d>();
    playerObject3d_->Initialize();

    // 最初の惑星
    currentPlanetPos_ = Vector3(stagePos[0].x, stagePos[0].y, 0.0f);
    currentPlanetPos_ = Vector3(stagePos[0].x, stagePos[0].y, 0.0f);
    targetPlanetPos_ = currentPlanetPos_;

    playerObjectModel = std::make_unique<Model>();
    playerObjectModel->Initialize("resources/player", "Player.obj"); // 指定したパターンに
    playerObject3d_->SetModel(playerObjectModel.get());

    playerObject3d_->SetTranslate(currentPlanetPos_);
    playerObject3d_->SetScale(Vector3(1.0f, 1.0f, 1.0f));
    playerObject3d_->SetRotate(Vector3(0.0f, 0.0f, 0.0f));

    // スプライトを表示↓後で
    std::array<std::string, maxStage> stageTexturePaths = {
        "resources/UI/SelectUIStage1.png",
        "resources/UI/SelectUIStage2.png"
    };

    for (int i = 0; i < maxStage; i++) {
        stageSprite[i] = std::make_unique<Sprite>();
        stageSprite[i]->Initialize(stageTexturePaths[i]);
        stageSprite[i]->SetPosition(Vector2(WinApp::KClientWidth / 2.0f, WinApp::KClientHeight / 8.0f)); // 中心位置
        stageSprite[i]->SetAnchorPoint(Vector2(0.5f, 0.5f));
        stageSpriteBaseSize_[i] = stageSprite[i]->GetSize();
    }

    UI = std::make_unique<Sprite>();
    UI->Initialize("resources/UI/SelectUI1.png");
    UI->SetPosition(Vector2(0.0f, WinApp::KClientHeight / 8.0f * 6.0f)); // 中心位置
    UI->SetAnchorPoint(Vector2(0.0f, 0.0f));

    stageNumber_ = 1;
    uiAnimTimer_ = 0.0f;
}

void stageSelectUI::Update(float deltaTime)
{
    MoveUpdate(deltaTime);
    UIAnimationUpdate(deltaTime);

    playerObject3d_->Update();
    UI->Update();
    for (int i = 0; i < maxStage; i++) {
        Object3d_[i]->Update();
        stageSprite[i]->Update();
    }
}

void stageSelectUI::Draw()
{

    playerObject3d_->Draw();

    for (int i = 0; i < maxStage; i++) {
        Object3d_[i]->Draw();
    }
}

void stageSelectUI::SpriteDraw()
{

    int idx = std::clamp(stageNumber_ - 1, 0, maxStage - 1);
    stageSprite[idx]->Draw();

    UI->Draw();
}

void stageSelectUI::ChangeStage(int stageIndex)
{
    stageNumber_ = stageIndex;
    int idx = std::clamp(stageIndex - 1, 0, maxStage - 1);

    startPlanetPos_ = currentPlanetPos_;
    // 選択された惑星の手前に機体を移動させる
    targetPlanetPos_ = Vector3(stagePos[idx].x, stagePos[idx].y, -2.0f);

    uiAnimTimer_ = 0.0f;
    moveTimer_ = 0.0f;
    moveDuration_ = kmoveDuration_;
    isMoving_ = true;
    isSortie = false;
}

void stageSelectUI::StartSortie(int stageIndex, float duration)
{
    stageNumber_ = stageIndex;
    int idx = std::clamp(stageIndex - 1, 0, maxStage - 1);

    startPlanetPos_ = currentPlanetPos_;
    // 惑星の奥（内部を突き抜ける座標）を最終目標に設定
    targetPlanetPos_ = Vector3(stagePos[idx].x, stagePos[idx].y, stagePos[idx].z + 5.0f);

    moveTimer_ = 0.0f;
    moveDuration_ = duration; // シーン遷移時間と同期
    isSortie = true;
    isMoving_ = false;
}

void stageSelectUI::MoveUpdate(float deltaTime)
{
    auto* camere = CameraManager::GetInstance()->GetActiveCamera();

    // イージング移動処理
    if (isMoving_) {
        moveTimer_ += deltaTime;
        float t = std::clamp(moveTimer_ / moveDuration_, 0.0f, 1.0f);
        float easedT = EaseOutCubic(t);

        currentPlanetPos_ = Lerp(startPlanetPos_, targetPlanetPos_, easedT);
        playerObject3d_->SetTranslate(currentPlanetPos_);

        if (t >= 1.0f) {
            isMoving_ = false;
        }
    } else if (isSortie) {
        moveTimer_ += deltaTime;
        float t = std::clamp(moveTimer_ / moveDuration_, 0.0f, 1.0f);
        float easedT = EaseOutCubic(t); // 奥へ向かって加速

        currentPlanetPos_ = Lerp(startPlanetPos_, targetPlanetPos_, easedT);
        playerObject3d_->SetTranslate(currentPlanetPos_);
    }

    // 惑星の時点
    for (int i = 0; i < maxStage; i++) {
        Vector3 rot = Object3d_[i]->GetRotate();
        rot.y += 0.5f * deltaTime;
        Object3d_[i]->SetRotate(rot);
    }

    // 座標セット
    Vector3 cameraPos = currentPlanetPos_;
    cameraPos.z += -15.0f;
    cameraPos.y += 1.0f;
    camere->SetTranslate(cameraPos);
}

void stageSelectUI::UIAnimationUpdate(float deltaTime)
{
    if (uiAnimTimer_ < kUIAnimDuration_) {
        uiAnimTimer_ += deltaTime;
        float t = std::clamp(uiAnimTimer_ / kUIAnimDuration_, 0.0f, 1.0f);
        float easedT = EaseOutCubic(t);

        // 現在選択されているステージ名スプライトのYサイズ可変設定
        int activeIdx = std::clamp(stageNumber_ - 1, 0, maxStage - 1);
        Vector2 baseSize = stageSpriteBaseSize_[activeIdx];
        stageSprite[activeIdx]->SetSize({ baseSize.x, baseSize.y * easedT });
    }
}
