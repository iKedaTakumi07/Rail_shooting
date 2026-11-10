#include "shockWaveParticle.h"
#include "../../../Engine/3d/GPUParticleManager.h"
#include "../../../Engine/base/TextureManager.h"

void shockWaveParticle::Initialize()
{
    TextureManager::getInstance()->LoadTexture("resources/testSnockWave.png");
    TextureManager::getInstance()->LoadTexture("resources/gradationLine.png");

    auto* gpuParticleMgr = GPUParticleManager::getInstance();
    // 新規登録
    gpuParticleMgr->CreateParticleGroup(
        "testSnockWave", "resources/testSnockWave.png", ParticleMeshType::kSphere, kBlendModeAdd, Vector4(1.0f, 1.0f, 1.0f, 1.0f));

    // 生成
    testSphereEmit_ = gpuParticleMgr->AddEmitter(
        "testSnockWave", ParticlePos, 0.5f, emitCount_, 0.05f, emitTimer_, gravity_, acceleration_, airResistance_);
    gpuParticleMgr->SetEmitterAutoEmit(testSphereEmit_, autoEmit_);

    // 超細かい設定
    EmitterSphereData* emitterData = gpuParticleMgr->GetEmitterData(testSphereEmit_);
    if (emitterData) {
        // [ランダム生成設定] スケール・回転・速度の幅をセット
        emitterData->minVelocity = Vector3(0.0f, 0.0f, 0.0f);
        emitterData->maxVelocity = Vector3(0.0f, 0.0f, 0.0f);
        emitterData->minSpeed = 0.0f;
        emitterData->maxSpeed = 0.0f;

        // 徐々に拡大
        emitterData->minStartScale = Vector3(0.2f, 0.2f, 0.2f);
        emitterData->maxStartScale = Vector3(0.2f, 0.2f, 0.2f);
        emitterData->minEndScale = Vector3(2.0f, 2.0f, 2.0f);
        emitterData->maxEndScale = Vector3(2.0f, 2.0f, 2.0f);

        // 回転はする
        emitterData->minStartRotation = Vector3(0.0f, 0.0f, 0.0f);
        emitterData->maxStartRotation = Vector3(0.0f, 0.0f, 3.14f);
        emitterData->minEndRotation = Vector3(0.0f, 0.0f, 3.14f);
        emitterData->maxEndRotation = Vector3(0.0f, 0.0f, 6.28f);

        // [時間経過グラデーション設定] 
        emitterData->minStartColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
        emitterData->maxStartColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
        emitterData->minEndColor = Vector4(1.0f, 1.0f, 1.0f, 0.0f);
        emitterData->maxEndColor = Vector4(1.0f, 1.0f, 1.0f, 0.0f);
    }

    gpuParticleMgr->CreateParticleGroup(
        "shockWaveRingGroup", "resources/gradationLine.png", ParticleMeshType::kRing, kBlendModeAdd, Vector4(1.0f, 1.0f, 1.0f, 1.0f));

    testRingEmit_ = gpuParticleMgr->AddEmitter(
        "shockWaveRingGroup", ParticlePos, 0.4f, emitCount_, 0.05f, emitTimer_, gravity_, acceleration_, airResistance_);
    gpuParticleMgr->SetEmitterAutoEmit(testRingEmit_, autoEmit_);

    if (EmitterSphereData* ringData = gpuParticleMgr->GetEmitterData(testRingEmit_)) {
        ringData->minVelocity = Vector3(0.0f, 0.0f, 0.0f);
        ringData->maxVelocity = Vector3(0.0f, 0.0f, 0.0f);
        ringData->minVelocity = Vector3(0.0f, 0.0f, 0.0f);
        ringData->maxVelocity = Vector3(0.0f, 0.0f, 0.0f);

        // 平面リング状に急速追従・拡散
        ringData->minStartScale = Vector3(0.2f, 0.2f, 0.2f);
        ringData->maxStartScale = Vector3(0.2f, 0.2f, 0.2f);
        ringData->minEndScale = Vector3(4.0f, 4.0f, 4.0f);
        ringData->maxEndScale = Vector3(4.0f, 4.0f, 4.0f);

        // 　色
        ringData->minStartColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
        ringData->maxStartColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
        ringData->minEndColor = Vector4(0.2f, 0.6f, 1.0f, 0.0f);
        ringData->maxEndColor = Vector4(0.2f, 0.6f, 1.0f, 0.0f);
    }
}

void shockWaveParticle::Emit(Vector3 pos) const
{
    auto* gpuParticleMgr = GPUParticleManager::getInstance();

    gpuParticleMgr->SetEmitterPosition(testSphereEmit_, pos);
    gpuParticleMgr->SetEmitterPosition(testRingEmit_, pos);

    gpuParticleMgr->TriggerOneShot(testSphereEmit_, emitCount_);
    gpuParticleMgr->TriggerOneShot(testRingEmit_, emitCount_);
}
