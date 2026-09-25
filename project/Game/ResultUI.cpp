#include "ResultUI.h"
#include "../Engine/base/TextureManager.h"
#include "../Engine/base/WinApp.h"
#include "../Game/stage/stageDataLoad.h"
#include "../resources/nlohmann/json.hpp"
#include "Loder/GameScoreManager.h"
#include <algorithm>
#include <fstream>
#include <iostream>
using json = nlohmann::json;
#include "../Engine/io/Input.h"
#include "../Engine/scene/SceneManager.h"
#include <random>

void ResultUI::Initialize()
{
    // ランダムエンジン初期化
    std::random_device seedGenerator;
    randomEngine = std::mt19937(seedGenerator());

    // 数字
    for (int i = 0; i < 10; ++i) {
        std::string path = "resources/UI/number/number" + std::to_string(i) + ".png";
        TextureManager::getInstance()->LoadTexture(path);
        numberList_[i] = path;
    }

    // 文字
    TextureManager::getInstance()->LoadTexture("resources/UI/ResultUI1.png");
    TextureManager::getInstance()->LoadTexture("resources/UI/ResultUI2.png");
    TextureManager::getInstance()->LoadTexture("resources/UI/ResultUI3.png");

    // ステージの敵の数を調べる
    PopEnemyFilePath_ = stageDataLoad::GetInstance()->GetEnemyPopData();
    maxEnemies();

    // 撃破数を読み込む
    NumberOfDefeats = GameScoreManager::GetInstance()->GetDefeatCount();

    ResultUI = std::make_unique<Sprite>();
    ResultUI->Initialize("resources/UI/ResultUI1.png");
    ResultUI->SetPosition(Vector2(WinApp::KClientWidth / 2.0f, WinApp::KClientHeight / 8.0f * 1.0f));
    ResultUI->SetAnchorPoint(Vector2(0.5f, 0.5f));

    Defeats = std::make_unique<Sprite>();
    Defeats->Initialize("resources/UI/ResultUI2.png");
    Defeats->SetPosition(Vector2(WinApp::KClientWidth / 3.0f, WinApp::KClientHeight / 2.0f)); // 中心位置
    Defeats->SetAnchorPoint(Vector2(0.5f, 0.5f));

    SelectUI = std::make_unique<Sprite>();
    SelectUI->Initialize("resources/UI/ResultUI3.png");
    SelectUI->SetPosition(Vector2(WinApp::KClientWidth / 2.0f, WinApp::KClientHeight / 8.0f * 7.0f)); // 中心位置
    SelectUI->SetAnchorPoint(Vector2(0.5f, 0.5f));

    ResultUIBaseSize_ = ResultUI->GetSize();
    DefeatsBaseSize_ = Defeats->GetSize();

    isUIDraw = false;

    SetNumber();

    nowChoice = State::kSceneChange;
}

void ResultUI::Update()
{
    auto* input = Input::getInstance();
    float deltaTime = SceneManager::GetInstance()->GetDeltaTime();

    switch (nowChoice) {
    case ResultUI::State::kSceneChange:
        if (SpriteDrawTimer <= 0.0f) {
            isUIDraw = true;
            nowChoice = State::kResultUpdateNow;
        } else {
            SpriteDrawTimer -= deltaTime;
        }
        break;
    case ResultUI::State::kResultUpdateNow:
        ResultUIUpdate(); // 文字をモニターみたいな起動の更新
        NumberUpdate(); // 数字変動
        skipUpdate(); // 演出スキップ

        if (isAllAnimationEnd) {
            nowChoice = State::kResult;
        }
        break;
    case ResultUI::State::kResult:

        if (input->TriggerKey(DIK_RETURN)) {
            isSelectOrder = true;
        }

        break;
    case ResultUI::State::kReturnSelect:
        break;
    }

    ResultUI->Update();
    Defeats->Update();
    SelectUI->Update();

    // 撃破数スプライトの更新
    for (auto& sprite : DefeatsNumber_) {
        if (sprite) {
            sprite->Update();
        }
    }
    for (auto& sprite : animationDefeatsNumber_) {
        if (sprite) {
            sprite->Update();
        }
    }
}

void ResultUI::Draw()
{
}

void ResultUI::SpritDraw()
{
    if (isUIDraw) {
        ResultUI->Draw();
        Defeats->Draw();

        if (isNumberAnimationEnd) {
            for (auto& sprite : DefeatsNumber_) {
                if (sprite) {
                    sprite->Draw();
                }
            }

            SelectUI->Draw(); // 案内ステージ選択UI
        } else {
            // ダミー数字を描画
            for (auto& sprite : animationDefeatsNumber_) {
                if (sprite) {
                    sprite->Draw();
                }
            }
        }
    }
}

void ResultUI::SetNumber()
{

    // 100以上かどうか(カンスト999)
    int tempDefeats = std::clamp(NumberOfDefeats, 0, 999);
    digits[0] = tempDefeats / 100; // 百の位
    digits[1] = (tempDefeats / 10) % 10; // 十の位
    digits[2] = tempDefeats % 10; // 一の位

    // 文字のスペース
    float startX = WinApp::KClientWidth / 2.0f;
    float startY = WinApp::KClientHeight / 2.0f;
    float spriteSpacing = 64.0f;

    for (size_t i = 0; i < DefeatsNumber_.size(); ++i) {
        DefeatsNumber_[i] = std::make_unique<Sprite>();
        DefeatsNumber_[i]->Initialize(numberList_[digits[i]]);

        // 横並びに配置（X軸にオフセットを加算）
        Vector2 pos(startX + (i * spriteSpacing), startY);
        DefeatsNumber_[i]->SetPosition(pos);
        DefeatsNumber_[i]->SetAnchorPoint(Vector2(0.5f, 0.5f));
    }

    for (size_t i = 0; i < animationDefeatsNumber_.size(); ++i) {
        animationDefeatsNumber_[i] = std::make_unique<Sprite>();
        animationDefeatsNumber_[i]->Initialize(numberList_[digits[i]]);

        // 横並びに配置（X軸にオフセットを加算）
        Vector2 pos(startX + (i * spriteSpacing), startY);
        animationDefeatsNumber_[i]->SetPosition(pos);
        animationDefeatsNumber_[i]->SetAnchorPoint(Vector2(0.5f, 0.5f));
    }
}

void ResultUI::maxEnemies()
{
    std::ifstream file(PopEnemyFilePath_);
    if (!file.is_open())
        return;

    json data; // ← 注意: ここの変数名も data
    try {
        file >> data;
    } catch (const json::parse_error& e) {
        std::cerr << "JSON Parse Error: " << e.what() << std::endl;
        return;
    }

    maxNumberOfEnemies = 0;

    if (data.contains("enemies") && data["enemies"].is_array()) {
        maxNumberOfEnemies = static_cast<int>(data["enemies"].size());
    }
}

void ResultUI::ResultUIUpdate()
{
    // 早期リターン
    if (isUIAnimationEnd) {
        return;
    }

    float deltaTime = SceneManager::GetInstance()->GetDeltaTime();
    AnimatonUI -= deltaTime;
    float t = std::clamp(1.0f - (AnimatonUI / kAnimatonUI), 0.0f, 1.0f);
    if (t >= 1.0f) {
        isUIAnimationEnd = true;
    }

    float ResultUISizeY = ResultUIBaseSize_.y * t;
    float DefeatsSizeY = DefeatsBaseSize_.y * t;

    ResultUI->SetSize({ ResultUIBaseSize_.x, ResultUISizeY });
    Defeats->SetSize({ DefeatsBaseSize_.x, DefeatsSizeY });
}

void ResultUI::NumberUpdate()
{
    // 早期リターン
    if (isNumberAnimationEnd) {
        return;
    }

    float deltaTime = SceneManager::GetInstance()->GetDeltaTime();
    numberAnimationTimer -= deltaTime;

    // 両方完了済みなら終わり
    if (numberAnimationTimer <= 0.0f) {
        isNumberAnimationEnd = true;
    }
    if (isNumberAnimationEnd && isUIAnimationEnd) {
        isAllAnimationEnd = true;
    }

    // アニメーション用のスプライトをコロコロ変える
    if (!isNumberAnimationEnd) {
        std::uniform_int_distribution<int> dist(0, 9);

        if (numberAnimationTimer <= 1.5f) {
            animationDefeatsNumber_[0]->SetTexture(numberList_[digits[0]], false);
        } else {
            animationDefeatsNumber_[0]->SetTexture(numberList_[dist(randomEngine)], false);
        }
        if (numberAnimationTimer <= 1.0f) {
            animationDefeatsNumber_[1]->SetTexture(numberList_[digits[1]], false);
        } else {
            animationDefeatsNumber_[1]->SetTexture(numberList_[dist(randomEngine)], false);
        }
        animationDefeatsNumber_[2]->SetTexture(numberList_[dist(randomEngine)], false); // 位置の位は終わるまでランダム
    }
}

void ResultUI::skipUpdate()
{
    // 押した場合即刻リザルト表記
    auto* input = Input::getInstance();
    if (input->TriggerKey(DIK_RETURN)) {
        nowChoice = State::kResult;
        isAllAnimationEnd = true;
        isNumberAnimationEnd = true;
        isUIAnimationEnd = true;

        // スプライトを戻す
        ResultUI->SetSize(ResultUIBaseSize_);
        Defeats->SetSize(DefeatsBaseSize_);
        // ランダムを描画しないフラグ設定
    }
}
