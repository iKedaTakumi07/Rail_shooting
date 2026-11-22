#include "ResultUI.h"
#include "../../Loder/GameScoreManager.h"
#include "../Engine/base/TextureManager.h"
#include "../Engine/base/WinApp.h"
#include "../Engine/io/Input.h"
#include "../Engine/scene/SceneManager.h"
#include "../Game/Loder/stageDataLoad.h"
#include "../resources/nlohmann/json.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <random>
#include <string>

using json = nlohmann::json;

void ResultUI::Initialize()
{
    // ランダムエンジン初期化
    std::random_device seedGenerator;
    randomEngine = std::mt19937(seedGenerator());

    for (int i = 0; i < 10; ++i) {
        numberList_[i] = "resources/UI/number/GameNumber" + std::to_string(i) + ".png";
        TextureManager::getInstance()->LoadTexture(numberList_[i]);
    }
    TextureManager::getInstance()->FlushUploads();

    TextureManager::getInstance()->LoadTexture("resources/UI/ResultUI1.png");
    TextureManager::getInstance()->LoadTexture("resources/UI/ResultUI2.png");
    TextureManager::getInstance()->LoadTexture("resources/UI/ResultUI3.png");
    TextureManager::getInstance()->LoadTexture("resources/UI/ResultWindow.png"); // 500*620のサイズ
    TextureManager::getInstance()->LoadTexture("resources/UI/medal.png"); // 64*64のサイズ

    TextureManager::getInstance()->FlushUploads();

    // ステージの敵の数を調べる
    PopEnemyFilePath_ = stageDataLoad::GetInstance()->GetEnemyPopData();
    maxEnemies();

    // 撃破点/最大Hit数を読み込む
    NumberOfDefeats = GameScoreManager::GetInstance()->GetTotalScore();
    maxHitCount_ = GameScoreManager::GetInstance()->GetMaxHitCount();

    ResultUI_ = std::make_unique<Sprite>();
    ResultUI_->Initialize("resources/UI/ResultUI1.png");
    ResultUI_->SetPosition(Vector2(WinApp::KClientWidth / 2.0f, WinApp::KClientHeight / 8.0f * 1.0f));
    ResultUI_->SetAnchorPoint(Vector2(0.5f, 0.5f));

    Defeats_ = std::make_unique<Sprite>();
    Defeats_->Initialize("resources/UI/ResultUI2.png");
    Defeats_->SetPosition(Vector2(WinApp::KClientWidth / 3.0f, WinApp::KClientHeight / 2.0f)); // 中心位置
    Defeats_->SetAnchorPoint(Vector2(0.5f, 0.5f));

    SelectUI_ = std::make_unique<Sprite>();
    SelectUI_->Initialize("resources/UI/ResultUI3.png");
    SelectUI_->SetPosition(Vector2(WinApp::KClientWidth / 2.0f, WinApp::KClientHeight / 8.0f * 7.0f)); // 中心位置
    SelectUI_->SetAnchorPoint(Vector2(0.5f, 0.5f));

    ResultUIBaseSize_ = ResultUI_->GetSize();
    DefeatsBaseSize_ = Defeats_->GetSize();

    isUIDraw_ = false;
    SetNumber();
}

void ResultUI::Update()
{
    auto* input = Input::GetInstance();
    float deltaTime = SceneManager::GetInstance()->GetDeltaTime();

    if (isUIDraw_ && !isAllAnimationEnd_) {
        ResultUIUpdate(deltaTime);
        NumberUpdate(deltaTime);
    }

    ResultUI_->Update();
    Defeats_->Update();
    SelectUI_->Update();

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
    for (auto& sprite : maxHitNumber_) {
        if (sprite) {
            sprite->Update();
        }
    }
    for (auto& sprite : animationMaxHitNumber_) {
        if (sprite) {
            sprite->Update();
        }
    }
}

void ResultUI::SpritDraw()
{
    if (!isUIDraw_) {
        return;
    }

    ResultUI_->Draw();
    Defeats_->Draw();

    if (isNumberAnimationEnd_) {
        for (auto& sprite : DefeatsNumber_) {
            if (sprite) {
                sprite->Draw();
            }
        }
        for (auto& sprite : maxHitNumber_) {
            if (sprite) {
                sprite->Draw();
            }
        }
        SelectUI_->Draw(); // ステージセレクト
    } else {
        for (auto& sprite : animationDefeatsNumber_) {
            if (sprite) {
                sprite->Draw(); // ダミー演出
            }
        }
        for (auto& sprite : animationMaxHitNumber_) {
            if (sprite) {
                sprite->Draw();
            }
        }
    }
}

void ResultUI::StartAnimation()
{
    isUIDraw_ = true;
    animatonUITimer_ = kAnimatonUIDuration_;
    numberAnimationTimer_ = 2.0f;
    isUIAnimationEnd_ = false;
    isNumberAnimationEnd_ = false;
    isAllAnimationEnd_ = false;
}

void ResultUI::SkipAnimation()
{
    isAllAnimationEnd_ = true;
    isNumberAnimationEnd_ = true;
    isUIAnimationEnd_ = true;

    ResultUI_->SetSize(ResultUIBaseSize_);
    Defeats_->SetSize(DefeatsBaseSize_);
}

void ResultUI::SetNumber()
{
    // 1000以上かどうか(カンスト999)
    int tempDefeats = std::clamp(NumberOfDefeats, 0, 999);
    digits[0] = tempDefeats / 100; // 百の位
    digits[1] = (tempDefeats / 10) % 10; // 十の位
    digits[2] = tempDefeats % 10; // 一の位

    // 最大HIT数
    int tempMaxHit = std::clamp(maxHitCount_, 0, 999);
    maxHitDigits[0] = tempMaxHit / 100;
    maxHitDigits[1] = (tempMaxHit / 10) % 10;
    maxHitDigits[2] = tempMaxHit % 10;

    // 文字のスペース
    float startX = WinApp::KClientWidth / 2.0f;
    float startY = WinApp::KClientHeight / 2.0f;
    float maxHitStartY = startY + 100.0f;
    float spriteSpacing = 64.0f;

    // 撃破点
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

    // 最大HIT
    for (size_t i = 0; i < maxHitNumber_.size(); ++i) {
        maxHitNumber_[i] = std::make_unique<Sprite>();
        maxHitNumber_[i]->Initialize(numberList_[maxHitDigits[i]]);
        maxHitNumber_[i]->SetPosition(Vector2(startX + (i * spriteSpacing), maxHitStartY));
        maxHitNumber_[i]->SetAnchorPoint(Vector2(0.5f, 0.5f));
    }

    for (size_t i = 0; i < animationMaxHitNumber_.size(); ++i) {
        animationMaxHitNumber_[i] = std::make_unique<Sprite>();
        animationMaxHitNumber_[i]->Initialize(numberList_[maxHitDigits[i]]);
        animationMaxHitNumber_[i]->SetPosition(Vector2(startX + (i * spriteSpacing), maxHitStartY));
        animationMaxHitNumber_[i]->SetAnchorPoint(Vector2(0.5f, 0.5f));
    }
}

void ResultUI::maxEnemies()
{
    std::ifstream file(PopEnemyFilePath_);
    if (!file.is_open())
        return;

    json data;
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

void ResultUI::ResultUIUpdate(float deltaTime)
{
    // 早期リターン
    if (isUIAnimationEnd_) {
        return;
    }

    animatonUITimer_ -= deltaTime;
    float t = std::clamp(1.0f - (animatonUITimer_ / kAnimatonUIDuration_), 0.0f, 1.0f);
    if (t >= 1.0f) {
        isUIAnimationEnd_ = true;
    }

    ResultUI_->SetSize({ ResultUIBaseSize_.x, ResultUIBaseSize_.y * t });
    Defeats_->SetSize({ DefeatsBaseSize_.x, DefeatsBaseSize_.y * t });
}

void ResultUI::NumberUpdate(float deltaTime)
{
    // 早期リターン
    if (isNumberAnimationEnd_) {
        return;
    }

    numberAnimationTimer_ -= deltaTime;

    // 両方完了済みなら終わり
    if (numberAnimationTimer_ <= 0.0f) {
        isNumberAnimationEnd_ = true;
    }
    if (isNumberAnimationEnd_ && isUIAnimationEnd_) {
        isAllAnimationEnd_ = true;
    }

    // アニメーション用のスプライトをコロコロ変える
    if (!isNumberAnimationEnd_) {
        std::uniform_int_distribution<int> dist(0, 9);

        if (numberAnimationTimer_ <= 1.5f) {
            animationDefeatsNumber_[0]->SetTexture(numberList_[digits[0]], false);
        } else {
            animationDefeatsNumber_[0]->SetTexture(numberList_[dist(randomEngine)], false);
        }
        if (numberAnimationTimer_ <= 1.0f) {
            animationDefeatsNumber_[1]->SetTexture(numberList_[digits[1]], false);
        } else {
            animationDefeatsNumber_[1]->SetTexture(numberList_[dist(randomEngine)], false);
        }
        animationDefeatsNumber_[2]->SetTexture(numberList_[dist(randomEngine)], false); // 位置の位は終わるまでランダム

        // 最大HIT
        if (numberAnimationTimer_ <= 1.5f) {
            animationMaxHitNumber_[0]->SetTexture(numberList_[maxHitDigits[0]], false);
        } else {
            animationMaxHitNumber_[0]->SetTexture(numberList_[dist(randomEngine)], false);
        }
        if (numberAnimationTimer_ <= 1.0f) {
            animationMaxHitNumber_[1]->SetTexture(numberList_[maxHitDigits[1]], false);
        } else {
            animationMaxHitNumber_[1]->SetTexture(numberList_[dist(randomEngine)], false);
        }
        animationMaxHitNumber_[2]->SetTexture(numberList_[dist(randomEngine)], false);
    }
}
