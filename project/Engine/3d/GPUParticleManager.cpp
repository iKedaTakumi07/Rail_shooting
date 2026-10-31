#include "GPUParticleManager.h"
#include "../3d/Camera.h"
#include "../base/DirectXCommon.h"
#include "../base/Logger.h"
#include "../base/Math.h"
#include "../base/SrvManager.h"
#include "../base/TextureManager.h"
#include "../base/WinApp.h"
#include "../scene/SceneManager.h"
#include "CameraManager.h"
#include <memory>
#include <numbers>

std::unique_ptr<GPUParticleManager> GPUParticleManager::instance = nullptr;

GPUParticleManager* GPUParticleManager::getInstance()
{
    if (instance == nullptr) {
        instance = std::make_unique<GPUParticleManager>(ConstructorKey());
    }
    return instance.get();
}

void GPUParticleManager::Initialize(DirectXCommon* DirectXCollision, SrvManager* srvManager)
{
    dxCommon = DirectXCollision;
    this->srvManager = srvManager;
    this->winApp_ = WinApp::GetInstance();

    // VS/PS初期化
    graphicsPipelineInitialize(dxCommon);

    // CS初期化
    CSPipelineInitialize(dxCommon);
    CSEmitPipelineInitialize(dxCommon);
    CSUpdatePipelineInitialize(dxCommon);

    // リソース類初期化
    ResourceInitialize();
}

void GPUParticleManager::Update()
{
    Camera_ = CameraManager::GetInstance()->GetActiveCamera();
    if (!Camera_) {
        return;
    }

    float deltaTime = SceneManager::GetInstance()->GetDeltaTime();

    PreFrameData_->deltaTime = deltaTime;
    PreFrameData_->time += deltaTime;

    for (auto& emitter : emitters_) {
        if (!emitter.active || !emitter.mappedData)
            continue;

        // 1. 任意時限射出 (TriggerEmitForDuration)
        if (emitter.emitDurationTimer > 0.0f) {
            emitter.emitDurationTimer -= deltaTime;

            emitter.mappedData->frequencyTime += deltaTime;
            if (emitter.mappedData->frequency <= emitter.mappedData->frequencyTime) {
                emitter.mappedData->frequencyTime -= emitter.mappedData->frequency;
                emitter.mappedData->emit = 1; // 射出許可
            } else {
                emitter.mappedData->emit = 0;
            }

            // 指定時間が経過した瞬間
            if (emitter.emitDurationTimer <= 0.0f) {
                // 次フレームで安全にフラグを消去するため -1.0f に移行させる
                emitter.emitDurationTimer = -1.0f;
            }
        }
        // 2. 常時自動射出 (autoEmit)
        else if (emitter.autoEmit) {
            emitter.mappedData->frequencyTime += deltaTime;
            if (emitter.mappedData->frequency <= emitter.mappedData->frequencyTime) {
                emitter.mappedData->frequencyTime -= emitter.mappedData->frequency;
                emitter.mappedData->emit = 1;
            } else {
                emitter.mappedData->emit = 0;
            }
        }
        // 3. 単発射出 (TriggerOneShot) と消去の遅延処理
        else {
            // 外部から emit = 1, timer = 0 に設定された「トリガー直後」のフレーム
            if (emitter.emitDurationTimer == 0.0f && emitter.mappedData->emit == 1) {
                // 今フレームは emit = 1 を維持してCSEmitへ渡し、次フレームで消去させる状態へ移行
                emitter.emitDurationTimer = -1.0f;
            }
            // 射出コマンド発行から1フレーム経過した状態 (GPU側での前フレーム処理が開始/完了している見込み)
            else if (emitter.emitDurationTimer < 0.0f) {
                emitter.mappedData->emit = 0; // ここで初めて安全にリセット
                emitter.emitDurationTimer = 0.0f; // 待機状態に戻す
            }
        }
    }

    const Matrix4x4& viewProjection = Camera_->GetViewProjectionMatrix();

    // ビルボード行列の計算
    Matrix4x4 backToFrontMatrix = MakeRotateYMatrix(std::numbers::pi_v<float>);
    Matrix4x4 billboardMatrix = Multiply(backToFrontMatrix, Camera_->GetWorldMatrix());
    billboardMatrix.m[3][0] = 0.0f;
    billboardMatrix.m[3][1] = 0.0f;
    billboardMatrix.m[3][2] = 0.0f;

    // 構文エラー修正: PerView* へのポインタ宣言
    PerView* perViewData = nullptr;
    perViewResource->Map(0, nullptr, reinterpret_cast<void**>(&perViewData));
    perViewData->viewProjection = viewProjection;
    perViewData->billboardMatrix = billboardMatrix;
    perViewResource->Unmap(0, nullptr);

    // 射出CS
    PrepareCSEmit();
    CSEmit();

    // 更新CS
    PrepareCSUpdate();
    CSUpdate();
}

void GPUParticleManager::Draw()
{
    auto* commandList = dxCommon->GetCommandList();

    commandList->SetGraphicsRootSignature(rootSignature.Get());
    commandList->SetGraphicsRootConstantBufferView(1, perViewResource->GetGPUVirtualAddress());
    srvManager->SetGraphicsRootDescriptorTable(3, 0);

    for (const auto& [groupName, group] : groups_) {
        assert(group.textureIndex != 0 && "GPUParticleGroup textureIndex is 0! Texture loading failed or points to OffscreenSurface.");

        // 各グループのブレンドモードに応じた PSO を適用
        size_t blendIdx = static_cast<size_t>(group.blendMode);
        commandList->SetPipelineState(graphicsPipelineStates_[blendIdx].Get());

        // 各グループのマテリアル CBV を適用（b0）
        commandList->SetGraphicsRootConstantBufferView(0, group.materialResource->GetGPUVirtualAddress());

        // 各グループ専用の Particle StructuredBuffer SRV を適用（t0）
        srvManager->SetGraphicsRootDescriptorTable(2, group.instancingSrvIndex);

        // 各グループのメッシュ VBV をバインドして描画呼び出し
        auto meshIt = meshes_.find(group.meshType);
        if (meshIt != meshes_.end()) {
            const auto& mesh = meshIt->second;
            commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            commandList->IASetVertexBuffers(0, 1, &mesh->GetVertexBufferView());

            commandList->DrawInstanced(mesh->GetVertexCount(), group.maxParticles, 0, 0);
        }
    }
}

void GPUParticleManager::CreateParticleGroup(const std::string& name, const std::string& textureFilePath, ParticleMeshType meshType, BlendMode blendMode, const Vector4& color)
{
    if (groups_.find(name) != groups_.end()) {
        return; // 登録済み
    }

    TextureManager::getInstance()->LoadTexture(textureFilePath);
    uint32_t texIndex = TextureManager::getInstance()->GetSrvIndex(textureFilePath);
    assert(texIndex != 0 && "Texture SrvIndex returned 0. Ensure the file path is correct and loaded.");

    GPUParticleGroup group;
    group.name = name;
    group.textureFilePath = textureFilePath;
    group.textureIndex = texIndex;
    group.blendMode = blendMode;
    group.meshType = meshType;
    group.color = color;
    group.useClampSampler = (meshType == ParticleMeshType::kRing) ? 1 : 0;
    group.maxParticles = 1024;

    // グループごとのマテリアル定数バッファ生成
    group.materialResource = dxCommon->CreateBufferResource(sizeof(ParticleMaterial));
    group.materialResource->Map(0, nullptr, reinterpret_cast<void**>(&group.mappedMaterialData));
    group.mappedMaterialData->color = color;
    group.mappedMaterialData->enableLighting = false;
    group.mappedMaterialData->uvTransform = MakeIdentity4x4();
    group.mappedMaterialData->useClampSampler = group.useClampSampler;

    auto* device = dxCommon->GetDevice();

    size_t particleBufferSize = sizeof(Particle) * group.maxParticles;
    D3D12_HEAP_PROPERTIES heapProps { D3D12_HEAP_TYPE_DEFAULT };
    D3D12_RESOURCE_DESC resDesc { };
    resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resDesc.Width = particleBufferSize;
    resDesc.Height = 1;
    resDesc.DepthOrArraySize = 1;
    resDesc.MipLevels = 1;
    resDesc.Format = DXGI_FORMAT_UNKNOWN;
    resDesc.SampleDesc.Count = 1;
    resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&group.particleResource));

    group.instancingSrvIndex = srvManager->Allocate();
    srvManager->CreateSRVforStructuredBuffer(group.instancingSrvIndex, group.particleResource.Get(), group.maxParticles, sizeof(Particle));

    D3D12_RESOURCE_DESC counterResDesc = resDesc;
    counterResDesc.Width = sizeof(int32_t);
    device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &counterResDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&group.freeCounterResource));

    D3D12_RESOURCE_DESC listResDesc = resDesc;
    listResDesc.Width = sizeof(int32_t) * group.maxParticles;
    device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &listResDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&group.freeListResource));

    PrepareCSInitialize();
    CSInitialize(group);

    groups_[name] = std::move(group);
}

size_t GPUParticleManager::AddEmitter(const std::string& groupName,
    const Vector3& position,
    float radius,
    uint32_t count,
    float frequency,
    float particleLifeTime,
    const Vector3& gravity,
    const Vector3& acceleration,
    float airResistance)
{
    auto it = groups_.find(groupName);
    assert(it != groups_.end() && "Particle group not found!");

    GPUEmitterInstance newEmitter;
    newEmitter.groupName = groupName;
    newEmitter.autoEmit = true;
    newEmitter.bufferResource = dxCommon->CreateBufferResource(sizeof(EmitterSphereData));
    newEmitter.bufferResource->Map(0, nullptr, reinterpret_cast<void**>(&newEmitter.mappedData));

    newEmitter.mappedData->translate = position;
    newEmitter.mappedData->radius = radius;
    newEmitter.mappedData->count = count;
    newEmitter.mappedData->frequency = frequency;
    newEmitter.mappedData->frequencyTime = 0.0f;
    newEmitter.mappedData->emit = 0;
    newEmitter.mappedData->textureIndex = it->second.textureIndex;
    newEmitter.mappedData->gravity = gravity;
    newEmitter.mappedData->acceleration = acceleration;
    newEmitter.mappedData->airResistance = airResistance;
    newEmitter.mappedData->particleLifeTime = particleLifeTime;

    emitters_.push_back(std::move(newEmitter));
    return emitters_.size() - 1;
}

void GPUParticleManager::SetEmitterAutoEmit(size_t emitterIndex, bool autoEmit)
{
    if (emitterIndex < emitters_.size()) {
        emitters_[emitterIndex].autoEmit = autoEmit;
    }
}

void GPUParticleManager::TriggerOneShot(size_t emitterIndex, uint32_t count)
{
    // duration = 0.0f を指定して 1 フレーム限定即時射出を実行
    TriggerEmitForDuration(emitterIndex, 0.0f, count);
}

void GPUParticleManager::TriggerEmitForDuration(size_t emitterIndex, float duration, uint32_t count)
{
    if (emitterIndex < emitters_.size() && emitters_[emitterIndex].mappedData) {
        auto& emitter = emitters_[emitterIndex];
        emitter.emitDurationTimer = duration;

        // 呼び出したそのフレームで即時1回目の発射が行われるようインターバルタイマーを満了状態に初期化
        emitter.mappedData->frequencyTime = emitter.mappedData->frequency;

        if (count > 0) {
            emitter.mappedData->count = count;
        }

        if (duration > 0.0f) {
            // 指定時間継続して射出するケース
            emitter.emitDurationTimer = duration;
            emitter.mappedData->frequencyTime = emitter.mappedData->frequency; // 即時1回目を許可
        } else {
            // duration = 0.0f の場合：呼び出したこのフレーム限定で即時 1 回射出
            emitter.emitDurationTimer = 0.0f;
            emitter.mappedData->emit = 1; // 1フレーム限定で直接射出許可を出す
        }
    }
}

void GPUParticleManager::SetEmitterPosition(size_t emitterIndex, const Vector3& position)
{
    if (emitterIndex < emitters_.size() && emitters_[emitterIndex].mappedData) {
        emitters_[emitterIndex].mappedData->translate = position;
    }
}

void GPUParticleManager::SetEmitterPhysics(size_t emitterIndex, const Vector3& gravity, const Vector3& accel, float drag)
{
    if (emitterIndex < emitters_.size() && emitters_[emitterIndex].mappedData) {
        emitters_[emitterIndex].mappedData->gravity = gravity;
        emitters_[emitterIndex].mappedData->acceleration = accel;
        emitters_[emitterIndex].mappedData->airResistance = drag;
    }
}

void GPUParticleManager::SetEmitterParticleLifeTime(size_t emitterIndex, float lifeTime)
{
    if (emitterIndex < emitters_.size() && emitters_[emitterIndex].mappedData) {
        emitters_[emitterIndex].mappedData->particleLifeTime = lifeTime;
    }
}

void GPUParticleManager::PrepareCSInitialize()
{
    dxCommon->GetCommandList()->SetComputeRootSignature(csInitRootSignature.Get());
    dxCommon->GetCommandList()->SetPipelineState(csInitPipelineState.Get());
}

void GPUParticleManager::CSInitialize(GPUParticleGroup& group)
{
    auto* cmdList = dxCommon->GetCommandList();

    // u0
    cmdList->SetComputeRootUnorderedAccessView(0, group.particleResource->GetGPUVirtualAddress());
    // u1
    cmdList->SetComputeRootUnorderedAccessView(1, group.freeCounterResource->GetGPUVirtualAddress());
    // u2
    cmdList->SetComputeRootUnorderedAccessView(2, group.freeListResource->GetGPUVirtualAddress());

    cmdList->Dispatch(1, 1, 1);

    D3D12_RESOURCE_BARRIER barrier = { };
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.UAV.pResource = group.particleResource.Get();
    cmdList->ResourceBarrier(1, &barrier);
}

void GPUParticleManager::PrepareCSEmit()
{
    dxCommon->GetCommandList()->SetComputeRootSignature(csEmitRootSignature.Get());
    dxCommon->GetCommandList()->SetPipelineState(csEmitPipelineState.Get());
}

void GPUParticleManager::CSEmit()
{
    auto* cmdList = dxCommon->GetCommandList();

    for (const auto& emitter : emitters_) {
        if (!emitter.active || emitter.mappedData->emit == 0)
            continue;

        auto groupIt = groups_.find(emitter.groupName);
        if (groupIt == groups_.end())
            continue;

        auto& group = groupIt->second;

        D3D12_RESOURCE_BARRIER barrier = { };
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource = group.particleResource.Get();
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        cmdList->ResourceBarrier(1, &barrier);

        cmdList->SetComputeRootUnorderedAccessView(0, group.particleResource->GetGPUVirtualAddress());
        cmdList->SetComputeRootUnorderedAccessView(1, group.freeCounterResource->GetGPUVirtualAddress());
        cmdList->SetComputeRootUnorderedAccessView(2, group.freeListResource->GetGPUVirtualAddress());

        cmdList->SetComputeRootConstantBufferView(3, emitter.bufferResource->GetGPUVirtualAddress());
        cmdList->SetComputeRootConstantBufferView(4, randomResource->GetGPUVirtualAddress());

        UINT totalThreadsNeeded = (emitter.mappedData->count + 3) / 4;
        UINT threadGroupCountX = (totalThreadsNeeded + 63) / 64;
        cmdList->Dispatch(threadGroupCountX, 1, 1);

        D3D12_RESOURCE_BARRIER uavBarrier = { };
        uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        uavBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        uavBarrier.UAV.pResource = group.particleResource.Get();
        cmdList->ResourceBarrier(1, &uavBarrier);
    }
}

void GPUParticleManager::PrepareCSUpdate()
{
    dxCommon->GetCommandList()->SetComputeRootSignature(csUpdateRootSignature.Get());
    dxCommon->GetCommandList()->SetPipelineState(csUpdatePipelineState.Get());
}

void GPUParticleManager::CSUpdate()
{
    auto* cmdList = dxCommon->GetCommandList();

    for (auto& [groupName, group] : groups_) {
        // u0
        cmdList->SetComputeRootUnorderedAccessView(0, group.particleResource->GetGPUVirtualAddress());
        // u1
        cmdList->SetComputeRootUnorderedAccessView(1, group.freeCounterResource->GetGPUVirtualAddress());
        // u2
        cmdList->SetComputeRootUnorderedAccessView(2, group.freeListResource->GetGPUVirtualAddress());

        // b0
        cmdList->SetComputeRootConstantBufferView(3, randomResource->GetGPUVirtualAddress());

        UINT totalThreadsNeeded = (group.maxParticles + 3) / 4;
        UINT threadGroupCountX = (totalThreadsNeeded + 63) / 64;
        cmdList->Dispatch(threadGroupCountX, 1, 1);

        D3D12_RESOURCE_BARRIER transitionToSRV = { };
        transitionToSRV.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        transitionToSRV.Transition.pResource = group.particleResource.Get();
        transitionToSRV.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        transitionToSRV.Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        transitionToSRV.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        cmdList->ResourceBarrier(1, &transitionToSRV);
    }
}

void GPUParticleManager::RootSignatureInitialize(DirectXCommon* dxcommon)
{
    // RootSignature作成
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature { };
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    D3D12_DESCRIPTOR_RANGE descriptorRangeForInstancing[1] = { };
    descriptorRangeForInstancing[0].BaseShaderRegister = 0;
    descriptorRangeForInstancing[0].NumDescriptors = 1;
    descriptorRangeForInstancing[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRangeForInstancing[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_DESCRIPTOR_RANGE descriptorRange[1] = { };
    descriptorRange[0].BaseShaderRegister = 0;
    descriptorRange[0].NumDescriptors = 256;
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_ROOT_PARAMETER rootParameters[4] = { };
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[0].Descriptor.ShaderRegister = 0;

    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[1].Descriptor.ShaderRegister = 0;

    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRangeForInstancing;
    rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeForInstancing);

    rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[3].DescriptorTable.pDescriptorRanges = descriptorRange;
    rootParameters[3].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);

    D3D12_STATIC_SAMPLER_DESC staticSamplers[2] = { };
    // [0] WRAP用 (主にPlane用) -> register(s0)
    staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
    staticSamplers[0].ShaderRegister = 0;
    staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    // [1] CLAMP用 (主にRing用) -> register(s1)
    staticSamplers[1].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    staticSamplers[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    staticSamplers[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[1].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    staticSamplers[1].MaxLOD = D3D12_FLOAT32_MAX;
    staticSamplers[1].ShaderRegister = 1;
    staticSamplers[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    descriptionRootSignature.pStaticSamplers = staticSamplers;
    descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

    descriptionRootSignature.pParameters = rootParameters;
    descriptionRootSignature.NumParameters = _countof(rootParameters);

    HRESULT hr;

    // シリアスライズしてバイナリにする
    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
    hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr)) {
        Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }

    // バイナリを元に生成
    rootSignature = nullptr;
    hr = dxcommon->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
    assert(SUCCEEDED(hr));
}

void GPUParticleManager::graphicsPipelineInitialize(DirectXCommon* dxcommon)
{
    // ルートシグネチャ作成
    RootSignatureInitialize(dxcommon);

    // InputLayout
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = { };
    inputElementDescs[0].SemanticName = "POSITION";
    inputElementDescs[0].SemanticIndex = 0;
    inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    inputElementDescs[1].SemanticName = "TEXCOORD";
    inputElementDescs[1].SemanticIndex = 0;
    inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    inputElementDescs[2].SemanticName = "COLOR";
    inputElementDescs[2].SemanticIndex = 0;
    inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc { };
    inputLayoutDesc.pInputElementDescs = inputElementDescs;
    inputLayoutDesc.NumElements = _countof(inputElementDescs);

    // RasterizerStateの設定
    D3D12_RASTERIZER_DESC rasterizerDesc { };
    // 裏面(時計回り)を表示しない
    rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
    // 三角形の中を塗りつぶす
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

    // Shaderをコンパイルする
    Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = dxcommon->CompileShader(L"resources/shaders/Particle.VS.hlsl", L"vs_6_0");
    assert(vertexShaderBlob != nullptr);

    Microsoft::WRL::ComPtr<IDxcBlob> pixeShaderBlob = dxcommon->CompileShader(L"resources/shaders/Particle.PS.hlsl", L"ps_6_0");
    assert(pixeShaderBlob != nullptr);

    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc { };
    graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();
    graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
    graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
    graphicsPipelineStateDesc.PS = { pixeShaderBlob->GetBufferPointer(), pixeShaderBlob->GetBufferSize() };

    graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
    // 書き込むRTVの情報
    graphicsPipelineStateDesc.NumRenderTargets = 1;
    graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    // 利用するとぽろじのタイプ
    graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    // どのように画面に色を打ち込むかの設定
    graphicsPipelineStateDesc.SampleDesc.Count = 1;
    graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

    // depthStencilStateの設定
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc { };
    // depthを有効化
    depthStencilDesc.DepthEnable = true;
    // 書き込み
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    // 比較関数はLessEqual
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

    // DepthStencilの設定
    graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
    graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    for (uint32_t i = 0; i < static_cast<uint32_t>(BlendMode::kCount); ++i) {
        D3D12_BLEND_DESC blendDesc { };
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        blendDesc.RenderTarget[0].BlendEnable = true;

        switch (static_cast<BlendMode>(i)) {
        case kBlendModeNone:
            blendDesc.RenderTarget[0].BlendEnable = false;
            break;
        case kBlendModeNormal: // 通常αブレンド
            blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
            blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
            blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
            blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
            blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
            blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
            break;
        case kBlendModeAdd: // 加算合成
            blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
            blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
            blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
            blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
            blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
            blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
            break;
        case kBlendModeSubtract: // 減算合成
            blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
            blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
            blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
            blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
            blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
            blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
            break;
        case kBlendModeMultily: // 乗算合成
            blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
            blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
            blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;
            blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_DEST_ALPHA;
            blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
            blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
            break;
        case kBlendModeScreen: // スクリーン合成
            blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
            blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
            blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
            blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
            blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
            blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
            break;
        default:
            break;
        }

        // 実際に生成
        graphicsPipelineStateDesc.BlendState = blendDesc;
        HRESULT hr = dxcommon->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineStates_[i]));
        assert(SUCCEEDED(hr));
    }
}

void GPUParticleManager::CSRootSignatureInitialize(DirectXCommon* dxcommon)
{
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature { };
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE; // CS用はインプットアセンブラ不要

    D3D12_ROOT_PARAMETER rootParameters[3] = { };

    // u0: gOutputVertices (UAV)
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
    rootParameters[0].Descriptor.ShaderRegister = 0;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    // u1
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
    rootParameters[1].Descriptor.ShaderRegister = 1;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    // u2
    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
    rootParameters[2].Descriptor.ShaderRegister = 2;
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    descriptionRootSignature.pParameters = rootParameters;
    descriptionRootSignature.NumParameters = _countof(rootParameters);

    HRESULT hr;
    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
    hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr)) {
        Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }

    csInitRootSignature = nullptr;
    hr = dxcommon->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&csInitRootSignature));
}

void GPUParticleManager::CSPipelineInitialize(DirectXCommon* dxcommon)
{
    CSRootSignatureInitialize(dxcommon);

    HRESULT hr;

    // CSをコンパイル
    Microsoft::WRL::ComPtr<IDxcBlob> computeShaderBlob = dxcommon->CompileShader(L"resources/shaders/InitializeParticle.CS.hlsl", L"cs_6_0");
    assert(computeShaderBlob != nullptr);
    // PSOの設定
    D3D12_COMPUTE_PIPELINE_STATE_DESC computePipelineStateDesc { };
    computePipelineStateDesc.CS = {
        .pShaderBytecode = computeShaderBlob->GetBufferPointer(),
        .BytecodeLength = computeShaderBlob->GetBufferSize()
    };
    computePipelineStateDesc.pRootSignature = csInitRootSignature.Get();

    csInitPipelineState = nullptr;
    hr = dxcommon->GetDevice()->CreateComputePipelineState(&computePipelineStateDesc, IID_PPV_ARGS(&csInitPipelineState));
}

void GPUParticleManager::CSEmitRootSignatureInitialize(DirectXCommon* dxcommon)
{
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature { };
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE; // CS用はインプットアセンブラ不要

    D3D12_ROOT_PARAMETER rootParameters[5] = { };

    // u0: gOutputVertices (UAV)
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
    rootParameters[0].Descriptor.ShaderRegister = 0;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    // u1
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
    rootParameters[1].Descriptor.ShaderRegister = 1;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    // u2
    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
    rootParameters[2].Descriptor.ShaderRegister = 2;
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    // b0
    rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[3].Descriptor.ShaderRegister = 0;
    rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    // b1
    rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[4].Descriptor.ShaderRegister = 1;
    rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    descriptionRootSignature.pParameters = rootParameters;
    descriptionRootSignature.NumParameters = _countof(rootParameters);

    HRESULT hr;
    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
    hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr)) {
        Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }

    csEmitRootSignature = nullptr;
    hr = dxcommon->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&csEmitRootSignature));
}

void GPUParticleManager::CSEmitPipelineInitialize(DirectXCommon* dxcommon)
{
    CSEmitRootSignatureInitialize(dxcommon);

    HRESULT hr;

    // CSをコンパイル
    Microsoft::WRL::ComPtr<IDxcBlob> computeShaderBlob = dxcommon->CompileShader(L"resources/shaders/EmitParticle.CS.hlsl", L"cs_6_0");
    assert(computeShaderBlob != nullptr);
    // PSOの設定
    D3D12_COMPUTE_PIPELINE_STATE_DESC computePipelineStateDesc { };
    computePipelineStateDesc.CS = {
        .pShaderBytecode = computeShaderBlob->GetBufferPointer(),
        .BytecodeLength = computeShaderBlob->GetBufferSize()
    };
    computePipelineStateDesc.pRootSignature = csEmitRootSignature.Get();

    csEmitPipelineState = nullptr;
    hr = dxcommon->GetDevice()->CreateComputePipelineState(&computePipelineStateDesc, IID_PPV_ARGS(&csEmitPipelineState));
}

void GPUParticleManager::CSUpdateRootSignatureInitialize(DirectXCommon* dxcommon)
{
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature { };
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE; // CS用はインプットアセンブラ不要

    D3D12_ROOT_PARAMETER rootParameters[4] = { };

    // u0: gOutputVertices (UAV)
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
    rootParameters[0].Descriptor.ShaderRegister = 0;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    // u1
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
    rootParameters[1].Descriptor.ShaderRegister = 1;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    // u2
    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
    rootParameters[2].Descriptor.ShaderRegister = 2;
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    // b0 PerFrame
    rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[3].Descriptor.ShaderRegister = 0;
    rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    descriptionRootSignature.pParameters = rootParameters;
    descriptionRootSignature.NumParameters = _countof(rootParameters);

    HRESULT hr;
    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
    hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr)) {
        Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }

    csUpdateRootSignature = nullptr;
    hr = dxcommon->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&csUpdateRootSignature));
}

void GPUParticleManager::CSUpdatePipelineInitialize(DirectXCommon* dxcommon)
{
    CSUpdateRootSignatureInitialize(dxcommon);

    HRESULT hr;

    // CSをコンパイル
    Microsoft::WRL::ComPtr<IDxcBlob> computeShaderBlob = dxcommon->CompileShader(L"resources/shaders/UpdateParticle.CS.hlsl", L"cs_6_0");
    assert(computeShaderBlob != nullptr);
    // PSOの設定
    D3D12_COMPUTE_PIPELINE_STATE_DESC computePipelineStateDesc { };
    computePipelineStateDesc.CS = {
        .pShaderBytecode = computeShaderBlob->GetBufferPointer(),
        .BytecodeLength = computeShaderBlob->GetBufferSize()
    };
    computePipelineStateDesc.pRootSignature = csUpdateRootSignature.Get();

    csUpdatePipelineState = nullptr;
    hr = dxcommon->GetDevice()->CreateComputePipelineState(&computePipelineStateDesc, IID_PPV_ARGS(&csUpdatePipelineState));
}

void GPUParticleManager::ResourceInitialize()
{
    auto* device = dxCommon->GetDevice();

    meshes_[ParticleMeshType::kPlane] = std::make_unique<PlaneMesh>(device);
    meshes_[ParticleMeshType::kRing] = std::make_unique<RingMesh>(device, 32, 1.0f, 0.2f);
    meshes_[ParticleMeshType::kCylinder] = std::make_unique<CylinderMesh>(device, 32, 1.0f, 1.0f, 3.0f);
    meshes_[ParticleMeshType::kTorus] = std::make_unique<TorusMesh>(device, 24, 12, 0.5f, 0.2f);
    meshes_[ParticleMeshType::kCone] = std::make_unique<ConeMesh>(device, 32, 1.0f, 2.0f);
    meshes_[ParticleMeshType::kHalfCone] = std::make_unique<HalfConeMesh>(device, 32, 1.0f, 2.0f);

    // カメラ
    perViewResource = dxCommon->CreateBufferResource(sizeof(PerView));

    // random
    randomResource = dxCommon->CreateBufferResource(sizeof(PreFrame));
    randomResource->Map(0, nullptr, reinterpret_cast<void**>(&PreFrameData_));
    PreFrameData_->time = 0;
    PreFrameData_->deltaTime = 0;
}
