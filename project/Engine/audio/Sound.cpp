#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "Mfreadwrite.lib")
#pragma comment(lib, "mfplat.lib")

#include <Windows.h>

#include <mfidl.h>

#include <mfapi.h>
#include <mferror.h>
#include <mfobjects.h>
#include <mfreadwrite.h>

#include <cassert>
#include <wrl.h>

#include "../base/StringUtility.h"
#include "Sound.h"

bool Sound::SoundLoadFile(const std::string& filename)
{
    Unload();

    std::wstring filePathW = StringUtility::ConvertString(filename);
    HRESULT result;

    Microsoft::WRL::ComPtr<IMFSourceReader> pReader;
    result = MFCreateSourceReaderFromURL(filePathW.c_str(), nullptr, &pReader);
    if (FAILED(result)) {
        return false; // ファイルが存在しない、または読み込み失敗時は安全に中断
    }

    Microsoft::WRL::ComPtr<IMFMediaType> pPCMType;
    result = MFCreateMediaType(&pPCMType);
    if (FAILED(result))
        return false;

    pPCMType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    pPCMType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
    result = pReader->SetCurrentMediaType(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), nullptr, pPCMType.Get());
    if (FAILED(result))
        return false;

    Microsoft::WRL::ComPtr<IMFMediaType> pOutType;
    result = pReader->GetCurrentMediaType(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), &pOutType);
    if (FAILED(result))
        return false;

    WAVEFORMATEX* waveFormat = nullptr;
    result = MFCreateWaveFormatExFromMFMediaType(pOutType.Get(), &waveFormat, nullptr);
    if (FAILED(result))
        return false;

    soundData.wfex = *waveFormat;
    CoTaskMemFree(waveFormat);

    while (true) {
        Microsoft::WRL::ComPtr<IMFSample> pSample;
        DWORD streamIndex = 0, flags = 0;
        LONGLONG llTimeStamp = 0;

        result = pReader->ReadSample(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), 0, &streamIndex, &flags, &llTimeStamp, &pSample);
        if (FAILED(result)) {
            Unload();
            return false;
        }

        if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
            break;

        if (pSample) {
            Microsoft::WRL::ComPtr<IMFMediaBuffer> pBuffer;
            result = pSample->ConvertToContiguousBuffer(&pBuffer);
            if (FAILED(result))
                continue;

            BYTE* pData = nullptr;
            DWORD maxLength = 0, currentLength = 0;
            result = pBuffer->Lock(&pData, &maxLength, &currentLength);
            if (SUCCEEDED(result)) {
                soundData.buffer.insert(soundData.buffer.end(), pData, pData + currentLength);
                pBuffer->Unlock();
            }
        }
    }

    return true;
}

void Sound::Unload()
{
    soundData.buffer.clear();
    soundData.wfex = { };
}
