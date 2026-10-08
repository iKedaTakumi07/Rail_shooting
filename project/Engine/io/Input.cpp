#include "Input.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <memory>

Input* Input::GetInstance()
{
    static std::unique_ptr<Input> instance = std::make_unique<Input>(ConstructorKey());

    return instance.get();
}

void Input::Initialize()
{
    winApp_ = WinApp::GetInstance();

    HRESULT result = DirectInput8Create(winApp_->GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
    assert(SUCCEEDED(result));

    // キーボードデバイスの生成
    result = directInput->CreateDevice(GUID_SysKeyboard, &keyboard_, NULL);
    assert(SUCCEEDED(result));

    // 入力データ形式のセット
    result = keyboard_->SetDataFormat(&c_dfDIKeyboard);
    assert(SUCCEEDED(result));

    // 排他制御レベルのリセット
    result = keyboard_->SetCooperativeLevel(winApp_->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
    assert(SUCCEEDED(result));

    // コントローラーの初期状態を取得
    ZeroMemory(&padState_, sizeof(padState_));
    ZeroMemory(&prevPadState_, sizeof(prevPadState_));
    isConnected_ = XInputGetState(playerIndex_, &padState_) == ERROR_SUCCESS;
}

void Input::Update()
{
    // 前回のキーを保存
    memcpy(prevKey, key_, sizeof(key_));

    // キーボード入力を取得
    HRESULT result = keyboard_->Acquire();
    if (FAILED(result)) {
        return;
    }
    result = keyboard_->GetDeviceState(sizeof(key_), key_);
    if (FAILED(result)) {
        return;
    }

    // コントローラー
    // 前回の状態を保存
    prevPadState_ = padState_;

    DWORD resultXInput = XInputGetState(playerIndex_, &padState_);

    isConnected_ = (resultXInput == ERROR_SUCCESS);
    if (!isConnected_) {
        ZeroMemory(&padState_, sizeof(padState_));
    }
}

bool Input::PushKey(BYTE keyNumber) const
{
    if (key_[keyNumber]) {
        return true;
    }

    return false;
}

bool Input::TriggerKey(BYTE keyNumber) const
{
    if (!prevKey[keyNumber] && key_[keyNumber]) {
        return true;
    }

    return false;
}

bool Input::PushButton(WORD button) const
{
    if (!isConnected_) {
        return false;
    }
    return (padState_.Gamepad.wButtons & button) != 0;
}

bool Input::TriggerButton(WORD button) const
{
    if (!isConnected_) {
        return false;
    }
    const bool current = (padState_.Gamepad.wButtons & button) != 0;
    const bool previous = (prevPadState_.Gamepad.wButtons & button) != 0;
    return !previous && current;
}

bool Input::ReleaseButton(WORD button) const
{
    if (!isConnected_) {
        return false;
    }
    const bool current = (padState_.Gamepad.wButtons & button) != 0;
    const bool previous = (prevPadState_.Gamepad.wButtons & button) != 0;
    return previous && !current;
}

Vector2 Input::GetLeftStick() const
{
    return NormalizeStick(padState_.Gamepad.sThumbLX, padState_.Gamepad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
}

Vector2 Input::GetRightStick() const
{
    return NormalizeStick(padState_.Gamepad.sThumbRX, padState_.Gamepad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
}

Vector2 Input::NormalizeStick(SHORT x, SHORT y, SHORT deadZone) const
{
    if (!isConnected_) {
        return { 0.0f, 0.0f };
    }
    float normalizedX = static_cast<float>(x);
    float normalizedY = static_cast<float>(y);

    // X軸
    if (std::abs(normalizedX) < deadZone) {
        normalizedX = 0.0f;
    } else if (normalizedX > 0.0f) {
        normalizedX = (normalizedX - deadZone) / (kThumbStickPositiveMax - deadZone);
    } else {
        normalizedX = (normalizedX + deadZone) / (kThumbStickNegativeMax - deadZone);
    }

    // Y軸
    if (std::abs(normalizedY) < deadZone) {
        normalizedY = 0.0f;
    } else if (normalizedY > 0.0f) {
        normalizedY = (normalizedY - deadZone) / (kThumbStickPositiveMax - deadZone);
    } else {
        normalizedY = (normalizedY + deadZone) / (kThumbStickNegativeMax - deadZone);
    }
    return { std::clamp(normalizedX, -1.0f, 1.0f), std::clamp(normalizedY, -1.0f, 1.0f) };
}

float Input::GetLeftTrigger() const
{
    if (!isConnected_) {
        return 0.0f;
    }
    const BYTE trigger = padState_.Gamepad.bLeftTrigger;
    if (trigger < XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {
        return 0.0f;
    }
    return static_cast<float>(trigger - XINPUT_GAMEPAD_TRIGGER_THRESHOLD) / (kTriggerMax - XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
}

float Input::GetRightTrigger() const
{
    if (!isConnected_) {
        return 0.0f;
    }
    const BYTE trigger = padState_.Gamepad.bRightTrigger;
    if (trigger < XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {
        return 0.0f;
    }
    return static_cast<float>(trigger - XINPUT_GAMEPAD_TRIGGER_THRESHOLD) / (kTriggerMax - XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
}