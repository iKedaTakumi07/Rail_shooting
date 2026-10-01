#pragma comment(lib, "mfplat.lib")

#include "Audio.h"
#include "Sound.h"
#include <algorithm>
#include <cassert>
#include <mfapi.h>

void VoiceCallback::OnBufferEnd(void* pBufferContext)
{
    if (pBufferContext) {
        auto* voice = static_cast<IXAudio2SourceVoice*>(pBufferContext);
        audio_->MarkVoiceForDeletion(voice);
    }
}

Audio::Audio()
    : voiceCallback_(this)
{
}

Audio* Audio::GetInstance()
{
    static Audio instance;
    return &instance;
}

bool Audio::Initialize()
{
    HRESULT result;

    result = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
    assert(SUCCEEDED(result));

    HRESULT hr = XAudio2Create(&xAudio2_, 0);
    if (FAILED(hr))
        return false;

    hr = xAudio2_->CreateMasteringVoice(&masterVoice_);
    return SUCCEEDED(hr);
}

void Audio::Finalize()
{
    std::lock_guard<std::mutex> lock(voiceMutex_);

    for (auto* voice : activeVoices_) {
        if (voice) {
            voice->Stop(0);
            voice->DestroyVoice();
        }
    }
    activeVoices_.clear();
    finishedVoices_.clear();

    if (masterVoice_) {
        masterVoice_->DestroyVoice();
        masterVoice_ = nullptr;
    }
    xAudio2_.Reset();

    MFShutdown();
}

void Audio::Update()
{
    std::vector<IXAudio2SourceVoice*> toDelete;
    {
        std::lock_guard<std::mutex> lock(voiceMutex_);
        toDelete.swap(finishedVoices_);
    }

    for (auto* voice : toDelete) {
        if (voice) {
            voice->Stop(0);
            voice->FlushSourceBuffers();
            voice->DestroyVoice();
            activeVoices_.erase(voice);
        }
    }
}

bool Audio::Play(const Sound& sound, float volume, bool isLoop)
{
    const auto& soundData = sound.GetSoundData();
    if (soundData.buffer.empty()) {
        return false; // データが読み込まれていない場合は安全に復帰
    }

    IXAudio2SourceVoice* sourceVoice = nullptr;

    HRESULT hr = xAudio2_->CreateSourceVoice(
        &sourceVoice,
        &soundData.wfex,
        0,
        XAUDIO2_DEFAULT_FREQ_RATIO,
        &voiceCallback_);
    if (FAILED(hr))
        return false;

    XAUDIO2_BUFFER buffer { };
    buffer.pAudioData = soundData.buffer.data();
    buffer.AudioBytes = static_cast<UINT32>(soundData.buffer.size());
    buffer.Flags = XAUDIO2_END_OF_STREAM;
    buffer.LoopCount = isLoop ? XAUDIO2_LOOP_INFINITE : 0;
    buffer.pContext = sourceVoice;

    hr = sourceVoice->SubmitSourceBuffer(&buffer);
    if (FAILED(hr)) {
        sourceVoice->DestroyVoice();
        return false;
    }

    // 音量設定
    sourceVoice->SetVolume(std::clamp(volume, 0.0f, 1.0f));

    hr = sourceVoice->Start();
    if (FAILED(hr)) {
        sourceVoice->DestroyVoice();
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(voiceMutex_);
        activeVoices_.insert(sourceVoice);
    }

    return true;
}

void Audio::MarkVoiceForDeletion(IXAudio2SourceVoice* voice)
{
    std::lock_guard<std::mutex> lock(voiceMutex_);
    finishedVoices_.push_back(voice);
}