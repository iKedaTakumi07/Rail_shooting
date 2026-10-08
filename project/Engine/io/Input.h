#pragma once
#define DIRECTINPUT_VERSION 0x0800

#pragma comment(lib, "xinput.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#include "../base/Math.h"
#include "../base/WinApp.h"
#include <Windows.h>
#include <Xinput.h>
#include <dinput.h>
#include <wrl.h>
#include <wrl/client.h>

class Input {
public:
    // namespace省略
    template <class T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;

    // コンストラクタに渡すための鍵
    class ConstructorKey {
    private:
        ConstructorKey() = default;
        friend class Input;
    };
    explicit Input(ConstructorKey) { }

    static Input* GetInstance();

public:
    // 初期化
    void Initialize();
    // 更新
    void Update();

    // キーボード入力
    bool PushKey(BYTE keyNumber) const;
    bool TriggerKey(BYTE keyNumber) const;

    // コントローラー入力
    bool PushButton(WORD button) const;
    bool TriggerButton(WORD button) const;
    bool ReleaseButton(WORD button) const;

    Vector2 GetLeftStick() const;
    Vector2 GetRightStick() const;

    float GetLeftTrigger() const;
    float GetRightTrigger() const;

    // コントローラー接続状態取得
    bool IsControllerConnected() const { return isConnected_; }

    Input(Input&) = delete;
    Input& operator=(Input&) = delete;

private:
    friend struct std::default_delete<Input>;

    Input() = default;
    ~Input() = default;

    // スティック値を-1.0～1.0に正規化
    Vector2 NormalizeStick(SHORT x, SHORT y, SHORT deadZone) const;

private:
    static constexpr float kThumbStickPositiveMax = 32767.0f;
    static constexpr float kThumbStickNegativeMax = 32768.0f;
    static constexpr float kTriggerMax = 255.0f;

private:
    // 全キーの入力状態を取得する
    BYTE key_[256] = { };
    BYTE prevKey[256] = { };

    ComPtr<IDirectInputDevice8> keyboard_;
    // DirectInputの初期化
    ComPtr<IDirectInput8> directInput;

    // コントローラーの状態
    XINPUT_STATE padState_ { };
    XINPUT_STATE prevPadState_ { };
    bool isConnected_ = false;
    DWORD playerIndex_ = 0;

    // windowsAPI
    WinApp* winApp_ = nullptr;
};
