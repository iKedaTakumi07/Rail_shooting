#include "SceneFactory.h"
#include <memory>

#include "../gameScene/GamePlayScene.h"
#include "../gameScene/SelectScene.h"
#include "../gameScene/TestScene.h"
#include "../gameScene/TitleScene.h"
#include "../gameScene/resultScene.h"

SceneFactory::SceneFactory(Camera* camera)
    : camera_(camera)
{
}

std::unique_ptr<BaseScene> SceneFactory::CreateScene(const std::string& sceneName)
{
    // 次のシーンを生成。
    std::unique_ptr<BaseScene> newScene = nullptr;

    if (sceneName == "TITLE") {
        newScene = std::make_unique<TitleScene>();

    } else if (sceneName == "GAMEPLAY") {
        newScene = std::make_unique<GamePlayScene>();
    } else if (sceneName == "SELECT") {
        newScene = std::make_unique<SelectScene>();
    } else if (sceneName == "RESULT") {
        newScene = std::make_unique<resultScene>();
    } else if (sceneName == "TEST") {
        newScene = std::make_unique<TestScene>();
    }

    if (newScene) {
        newScene->SetCamera(camera_);
    }

    return newScene;
}
