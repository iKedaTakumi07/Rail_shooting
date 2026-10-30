#pragma once
#include "../base/Math.h"
#include "IParticleMesh.h"
#include <d3d12.h>
#include <list>
#include <memory>
#include <random>
#include <string>
#include <unordered_map>
#include <wrl.h>

class DirectXCommon;
class SrvManager;
class Camera;
class WinApp;

struct GPUParticleGroup {
    std::string name;
    std::string textureFilePath;
    uint32_t textureIndex = 0;
    BlendMode blendMode = kBlendModeAdd;
    ParticleMeshType meshType = ParticleMeshType::kPlane;
    Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
    int32_t useClampSampler = 0;

    Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
    ParticleMaterial* mappedMaterialData = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> particleResource;
    Microsoft::WRL::ComPtr<ID3D12Resource> freeCounterResource;
    Microsoft::WRL::ComPtr<ID3D12Resource> freeListResource;
    uint32_t instancingSrvIndex = 0;
    uint32_t maxParticles = 1024;
};

class GPUParticleManager {
public:
    // コンストラクタに渡すための鍵
    class ConstructorKey {
    private:
        ConstructorKey() = default;
        friend class GPUParticleManager;
    };

    // passkeyを受け取るコンストラクタ
    explicit GPUParticleManager(ConstructorKey) { }

    static GPUParticleManager* getInstance();

    // 初期化
    void Initialize(DirectXCommon* DirectXCollision, SrvManager* srvManager);

    void Update();

    void Draw();

    // set
    void SetDefaultCamera(Camera* camera) { this->Camera_ = camera; }

    // パーティクルグループの作成
    void CreateParticleGroup(
        const std::string& groupName,
        const std::string& textureFilePath,
        ParticleMeshType meshType = ParticleMeshType::kPlane,
        BlendMode blendMode = kBlendModeAdd,
        const Vector4& color = { 1.0f, 1.0f, 1.0f, 1.0f });

    // 指定したグループからエミッターを追加。
    size_t AddEmitter(const std::string& groupName, const Vector3& position, float radius, uint32_t count, float frequency);

    // CSInitialize
    void PrepareCSInitialize();
    void CSInitialize(GPUParticleGroup& group);

    // CSEmit
    void PrepareCSEmit();
    void CSEmit();

    // CSUpdate
    void PrepareCSUpdate();
    void CSUpdate();

    GPUParticleManager(GPUParticleManager&) = delete;
    GPUParticleManager& operator=(GPUParticleManager&) = delete;

private:
    friend struct std::default_delete<GPUParticleManager>;
    ~GPUParticleManager() = default;

private:
    void RootSignatureInitialize(DirectXCommon* dxcommon);
    void graphicsPipelineInitialize(DirectXCommon* dxcommon);

    // CS初期化用パイプライン
    void CSRootSignatureInitialize(DirectXCommon* dxcommon);
    void CSPipelineInitialize(DirectXCommon* dxcommon);

    // CS射出用パイプライン
    void CSEmitRootSignatureInitialize(DirectXCommon* dxcommon);
    void CSEmitPipelineInitialize(DirectXCommon* dxcommon);

    // CS更新用パイプライン
    void CSUpdateRootSignatureInitialize(DirectXCommon* dxcommon);
    void CSUpdatePipelineInitialize(DirectXCommon* dxcommon);

    void ResourceInitialize();

private:
    SrvManager* srvManager = nullptr;
    DirectXCommon* dxCommon = nullptr;
    WinApp* winApp_ = nullptr;
    Camera* Camera_ = nullptr;

    // 各リソース
    Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
    Microsoft::WRL::ComPtr<ID3D12Resource> perViewResource;

    uint32_t instancingSrvIndex = 0;
    uint32_t textureSrvIndex = 0;

    // IA
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView { };
    VertexData* vertexData = nullptr;

    // コンテナ
    std::unordered_map<std::string, GPUParticleGroup> groups_;
    std::vector<GPUEmitterInstance> emitters_;
    std::unordered_map<ParticleMeshType, std::unique_ptr<IParticleMesh>> meshes_; // メッシュコンテナ (Plane, Ring, Cylinder)

    // ブレンドモード
    std::array<Microsoft::WRL::ComPtr<ID3D12PipelineState>, static_cast<size_t>(BlendMode::kCount)> graphicsPipelineStates_;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;

    // 初期化用CS
    Microsoft::WRL::ComPtr<ID3D12RootSignature> csInitRootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> csInitPipelineState;

    // 射出用CS
    Microsoft::WRL::ComPtr<ID3D12RootSignature> csEmitRootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> csEmitPipelineState;

    // 更新用CS
    Microsoft::WRL::ComPtr<ID3D12RootSignature> csUpdateRootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> csUpdatePipelineState;

    // random
    Microsoft::WRL::ComPtr<ID3D12Resource> randomResource;
    PreFrame* PreFrameData_ = nullptr;

    static std::unique_ptr<GPUParticleManager> instance;
};
