#pragma once
#include <fstream>
#include <mutex>
#include <unordered_set>
#include <vector>
#include <wrl.h>
#include <xaudio2.h>

#pragma comment(lib, "xaudio2.lib")

class Sound;

class VoiceCallback : public IXAudio2VoiceCallback {
public:
    VoiceCallback(class Audio* audio)
        : audio_(audio)
    {
    }
    ~VoiceCallback() = default;

    STDMETHOD_(void, OnVoiceProcessingPassStart)(UINT32) override { }
    STDMETHOD_(void, OnVoiceProcessingPassEnd)() override { }
    STDMETHOD_(void, OnStreamEnd)() override { }
    STDMETHOD_(void, OnBufferStart)(void*) override { }
    // バッファ再生終了時に呼び出される
    STDMETHOD_(void, OnBufferEnd)(void* pBufferContext) override;
    STDMETHOD_(void, OnLoopEnd)(void*) override { }
    STDMETHOD_(void, OnVoiceError)(void*, HRESULT) override { }

private:
    Audio* audio_ = nullptr;
};

class Audio {
public:
    // Singleton 取得
    static Audio* GetInstance();

    Audio();
    ~Audio() { Finalize(); }

    /// <summary>
    /// 初期化
    /// </summary>
    /// <returns></returns>
    bool Initialize();
    void Finalize();

    void Update();

    bool Play(const Sound& sound, float volume = 1.0f, bool isLoop = false);

    void MarkVoiceForDeletion(IXAudio2SourceVoice* voice);

    Audio(const Audio&) = delete;
    Audio& operator=(const Audio&) = delete;

private:
    Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;
    IXAudio2MasteringVoice* masterVoice_ = nullptr;

    VoiceCallback voiceCallback_;

    std::unordered_set<IXAudio2SourceVoice*> activeVoices_;
    std::vector<IXAudio2SourceVoice*> finishedVoices_;
    std::mutex voiceMutex_;
};
