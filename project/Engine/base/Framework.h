#pragma once
#include <memory>

#include "../3d/Camera.h"
#include "../3d/CameraManager.h"
#include "../3d/LightManager.h"
#include "../io/Input.h"
#include "../scene/base/AbstractSceneFactory.h"
#include "../scene/base/BaseScene.h"
#include "DirectXCommon.h"
#include "ImGuiManager.h"
#include "OffscreenSurface.h"
#include "PostProcess.h"
#include "SrvManager.h"
class WinApp;
class SpriteCommon;
class Object3dCommon;
class ModelCommon;

class Framework {
public:
    virtual ~Framework() = default;

    // 実行
    void Run();

    // 初期化
    virtual void Initialize();

    // 更新
    virtual void Update();

    // 描画
    virtual void Draw() = 0;

    // 終了
    virtual void Finalize();

    virtual bool IsEndRequst();
    void RequestEnd() { endRequst_ = true; }

    // get
    BaseScene* GetBaseScene() { return baseScene.get(); }
    DirectXCommon* GetDirectXCommon() { return dxCommon.get(); }
    SrvManager* GetSrvManager() { return srvManager.get(); }
    ImGuiManager* GetImGuiManager() { return imguiManager.get(); }
    OffscreenSurface* GetOffScreenSurface() { return offscreenSurface.get(); } // そのほか
    OffscreenSurface* GetOffScreenSurfaceB() { return offscreenSurfaceB.get(); } // 分離フィルターの中間バッフア

private:
    // ゲーム終了クラス
    bool endRequst_ = false;

    // dxCommon
    std::unique_ptr<DirectXCommon> dxCommon = nullptr;
    // srvマネージャー
    std::unique_ptr<SrvManager> srvManager = nullptr;
    // IMGUIマネージャー
    std::unique_ptr<ImGuiManager> imguiManager = nullptr;

    // ベースシーン
    std::unique_ptr<BaseScene> baseScene = nullptr;
    // シーンファクトリー
    std::unique_ptr<AbstractSceneFactory> sceneFactory_ = nullptr;
    // offscreen()
    std::unique_ptr<OffscreenSurface> offscreenSurface = nullptr;
    std::unique_ptr<OffscreenSurface> offscreenSurfaceB = nullptr; // 分離フィルターの中間バッフア
    // postProcess
    std::unique_ptr<PostProcess> postProcess = nullptr;
};
