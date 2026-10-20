#include "IParticleMesh.h"
#include <cassert>
#include <cmath>
#include <numbers>

// バッファリソース生成用
Microsoft::WRL::ComPtr<ID3D12Resource> CreateBuffer(ID3D12Device* device, size_t size)
{
    Microsoft::WRL::ComPtr<ID3D12Resource> resource;
    D3D12_HEAP_PROPERTIES heapProps { };
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC resDesc { };
    resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resDesc.Width = size;
    resDesc.Height = 1;
    resDesc.DepthOrArraySize = 1;
    resDesc.MipLevels = 1;
    resDesc.Format = DXGI_FORMAT_UNKNOWN;
    resDesc.SampleDesc.Count = 1;
    resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource));
    return resource;
}

// === PlaneMesh の実装 ===
PlaneMesh::PlaneMesh(ID3D12Device* device)
{
    size_t size = sizeof(VertexData) * 6;
    vertexResource_ = CreateBuffer(device, size);

    vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = static_cast<UINT>(size);
    vertexBufferView_.StrideInBytes = sizeof(VertexData);

    VertexData* vertexData = nullptr;
    vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

    vertexData[0] = { { 1.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } };
    vertexData[1] = { { -1.0f, 1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } };
    vertexData[2] = { { 1.0f, -1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } };
    vertexData[3] = { { 1.0f, -1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } };
    vertexData[4] = { { -1.0f, 1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } };
    vertexData[5] = { { -1.0f, -1.0f, 0.0f, 1.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } };

    vertexResource_->Unmap(0, nullptr);
}

// === RingMesh の実装 ===
RingMesh::RingMesh(ID3D12Device* device, uint32_t divide, float outerRadius, float innerRadius)
{
    // 1分割につき2つの三角形（6頂点）が必要
    vertexCount_ = divide * 6;
    size_t size = sizeof(VertexData) * vertexCount_;
    vertexResource_ = CreateBuffer(device, size);

    vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = static_cast<UINT>(size);
    vertexBufferView_.StrideInBytes = sizeof(VertexData);

    VertexData* vertexData = nullptr;
    vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

    float radianPreDivide = 2.0f * std::numbers::pi_v<float> / float(divide);

    for (uint32_t index = 0; index < divide; ++index) {
        float Rad = index * radianPreDivide;
        float nextRad = (index + 1) * radianPreDivide;

        // 元コードの cos の計算が sin になっていたバグと、
        // ルート内で同じ配列インデックス[0~3]を上書きしていたバグを修正
        float sin = std::sin(Rad);
        float cos = std::cos(Rad);
        float sinNext = std::sin(nextRad);
        float cosNext = std::cos(nextRad);

        float u = float(index) / float(divide);
        float uNext = float(index + 1) / float(divide);

        // 各位置の頂点座標を計算
        Vector4 pOuter = { -sin * outerRadius, cos * outerRadius, 0.0f, 1.0f };
        Vector4 pOuterNext = { -sinNext * outerRadius, cosNext * outerRadius, 0.0f, 1.0f };
        Vector4 pInner = { -sin * innerRadius, cos * innerRadius, 0.0f, 1.0f };
        Vector4 pInnerNext = { -sinNext * innerRadius, cosNext * innerRadius, 0.0f, 1.0f };

        uint32_t offset = index * 6;
        Vector3 normal = { 0.0f, 0.0f, 1.0f };

        // 三角形1
        vertexData[offset + 0] = { pOuter, { u, 0.0f }, normal };
        vertexData[offset + 1] = { pOuterNext, { uNext, 0.0f }, normal };
        vertexData[offset + 2] = { pInner, { u, 1.0f }, normal };
        // 三角形2
        vertexData[offset + 3] = { pInner, { u, 1.0f }, normal };
        vertexData[offset + 4] = { pOuterNext, { uNext, 0.0f }, normal };
        vertexData[offset + 5] = { pInnerNext, { uNext, 1.0f }, normal };
    }

    vertexResource_->Unmap(0, nullptr);
}

CylinderMesh::CylinderMesh(ID3D12Device* device, uint32_t divide, float kTopRadius, float kBottomRadius, float kHeight)
{
    // 1分割につき2つの三角形（6頂点）が必要
    vertexCount_ = divide * 6;
    size_t size = sizeof(VertexData) * vertexCount_;
    vertexResource_ = CreateBuffer(device, size);

    vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = static_cast<UINT>(size);
    vertexBufferView_.StrideInBytes = sizeof(VertexData);

    VertexData* vertexData = nullptr;
    vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

    float radianPreDivide = 2.0f * std::numbers::pi_v<float> / float(divide);

    for (uint32_t index = 0; index < divide; ++index) {
        float sin = std::sin(index * radianPreDivide);
        float cos = std::cos(index * radianPreDivide);
        float sinNext = std::sin((index + 1) * radianPreDivide);
        float cosNext = std::cos((index + 1) * radianPreDivide);
        float u = float(index) / float(divide);
        float uNext = float(index + 1) / float(divide);

        // pos,tex,normal
        uint32_t offset = index * 6;
        // 三角形1
        vertexData[offset + 0] = { { -sin * kTopRadius, kHeight, cos * kTopRadius, 1.0f }, { u, vTop }, { -sin, 0.0f, cos } };
        vertexData[offset + 1] = { { -sinNext * kTopRadius, kHeight, cosNext * kTopRadius, 1.0f }, { uNext, vTop }, { -sinNext, 0.0f, cosNext } };
        vertexData[offset + 2] = { { -sin * kBottomRadius, 0.0f, cos * kBottomRadius, 1.0f }, { u, vBottom }, { -sin, 0.0f, cos } };
        // 三角形2
        vertexData[offset + 3] = { { -sin * kBottomRadius, 0.0f, cos * kBottomRadius, 1.0f }, { u, vBottom }, { -sin, 0.0f, cos } };
        vertexData[offset + 4] = { { -sinNext * kTopRadius, kHeight, cosNext * kTopRadius, 1.0f }, { uNext, vTop }, { -sinNext, 0.0f, cosNext } };
        vertexData[offset + 5] = { { -sinNext * kBottomRadius, 0.0f, cosNext * kBottomRadius, 1.0f }, { uNext, vBottom }, { -sinNext, 0.0f, cosNext } };
    }

    vertexResource_->Unmap(0, nullptr);
}

TorusMesh::TorusMesh(ID3D12Device* device, uint32_t tubularDivide, uint32_t radialDivide, float majorRadius, float minorRadius)
{
    vertexCount_ = tubularDivide * radialDivide * 6;
    size_t size = sizeof(VertexData) * vertexCount_;
    vertexResource_ = CreateBuffer(device, size);

    vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = static_cast<UINT>(size);
    vertexBufferView_.StrideInBytes = sizeof(VertexData);

    VertexData* vertexData = nullptr;
    vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

    float tubularStep = 2.0f * std::numbers::pi_v<float> / float(tubularDivide);
    float radialStep = 2.0f * std::numbers::pi_v<float> / float(radialDivide);

    uint32_t vIndex = 0;

    for (uint32_t i = 0; i < tubularDivide; ++i) {
        float phi = i * tubularStep;
        float nextPhi = (i + 1) * tubularStep;

        float cosPhi = std::cos(phi);
        float sinPhi = std::sin(phi);
        float cosNextPhi = std::cos(nextPhi);
        float sinNextPhi = std::sin(nextPhi);

        float u = float(i) / float(tubularDivide);
        float uNext = float(i + 1) / float(tubularDivide);

        for (uint32_t j = 0; j < radialDivide; ++j) {
            float theta = j * radialStep;
            float nextTheta = (j + 1) * radialStep;

            float cosTheta = std::cos(theta);
            float sinTheta = std::sin(theta);
            float cosNextTheta = std::cos(nextTheta);
            float sinNextTheta = std::sin(nextTheta);

            float v = float(j) / float(radialDivide);
            float vNext = float(j + 1) / float(radialDivide);

            // 頂点位置計算 (XY平面上に大円、Z方向に厚みを持つ構造)
            auto CalculatePoint = [&](float cPhi, float sPhi, float cTheta, float sTheta) -> Vector4 {
                float r = majorRadius + minorRadius * cTheta;
                return Vector4 { r * cPhi, r * sPhi, minorRadius * sTheta, 1.0f };
            };

            auto CalculateNormal = [&](float cPhi, float sPhi, float cTheta, float sTheta) -> Vector3 {
                return Vector3 { cTheta * cPhi, cTheta * sPhi, sTheta };
            };

            Vector4 p00 = CalculatePoint(cosPhi, sinPhi, cosTheta, sinTheta);
            Vector4 p10 = CalculatePoint(cosNextPhi, sinNextPhi, cosTheta, sinTheta);
            Vector4 p01 = CalculatePoint(cosPhi, sinPhi, cosNextTheta, sinNextTheta);
            Vector4 p11 = CalculatePoint(cosNextPhi, sinNextPhi, cosNextTheta, sinNextTheta);

            Vector3 n00 = CalculateNormal(cosPhi, sinPhi, cosTheta, sinTheta);
            Vector3 n10 = CalculateNormal(cosNextPhi, sinNextPhi, cosTheta, sinTheta);
            Vector3 n01 = CalculateNormal(cosPhi, sinPhi, cosNextTheta, sinNextTheta);
            Vector3 n11 = CalculateNormal(cosNextPhi, sinNextPhi, cosNextTheta, sinNextTheta);

            // 三角形 1
            vertexData[vIndex++] = { p00, { u, v }, n00 };
            vertexData[vIndex++] = { p10, { uNext, v }, n10 };
            vertexData[vIndex++] = { p01, { u, vNext }, n01 };

            // 三角形 2
            vertexData[vIndex++] = { p01, { u, vNext }, n01 };
            vertexData[vIndex++] = { p10, { uNext, v }, n10 };
            vertexData[vIndex++] = { p11, { uNext, vNext }, n11 };
        }
    }

    vertexResource_->Unmap(0, nullptr);
}

ConeMesh::ConeMesh(ID3D12Device* device, uint32_t divide, float bottomRadius, float height)
{
    // 1分割につき2つの三角形（6頂点：両面描画用）
    vertexCount_ = divide * 6;
    size_t size = sizeof(VertexData) * vertexCount_;
    vertexResource_ = CreateBuffer(device, size);

    vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = static_cast<UINT>(size);
    vertexBufferView_.StrideInBytes = sizeof(VertexData);

    VertexData* vertexData = nullptr;
    vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

    float radianPreDivide = 2.0f * std::numbers::pi_v<float> / float(divide);

    for (uint32_t index = 0; index < divide; ++index) {
        float sin = std::sin(index * radianPreDivide);
        float cos = std::cos(index * radianPreDivide);
        float sinNext = std::sin((index + 1) * radianPreDivide);
        float cosNext = std::cos((index + 1) * radianPreDivide);

        float u = float(index) / float(divide);
        float uNext = float(index + 1) / float(divide);

        uint32_t offset = index * 6;

        // Z=0 を底面、Z=height を頂点（Apex）とする円錐
        Vector4 apex = { 0.0f, 0.0f, height, 1.0f };
        Vector4 pCurrent = { -sin * bottomRadius, cos * bottomRadius, 0.0f, 1.0f };
        Vector4 pNext = { -sinNext * bottomRadius, cosNext * bottomRadius, 0.0f, 1.0f };

        Vector3 n0 = Normalize(Vector3 { -sin, cos, bottomRadius / height });
        Vector3 n1 = Normalize(Vector3 { -sinNext, cosNext, bottomRadius / height });

        // 表面（時計回り）
        vertexData[offset + 0] = { pCurrent, { u, 0.0f }, n0 };
        vertexData[offset + 1] = { apex, { u + 0.5f / divide, 1.0f }, { 0.0f, 0.0f, 1.0f } };
        vertexData[offset + 2] = { pNext, { uNext, 0.0f }, n1 };

        // 裏面（反時計回り：カリングOFF時や内部視点用）
        vertexData[offset + 3] = { pCurrent, { u, 0.0f }, n0 };
        vertexData[offset + 4] = { pNext, { uNext, 0.0f }, n1 };
        vertexData[offset + 5] = { apex, { u + 0.5f / divide, 1.0f }, { 0.0f, 0.0f, 1.0f } };
    }

    vertexResource_->Unmap(0, nullptr);
}

HalfConeMesh::HalfConeMesh(ID3D12Device* device, uint32_t divide, float bottomRadius, float height)
{
    // 1分割につき表面の三角形1つ（3頂点）のみ生成して軽量化＆加算重複防止
    vertexCount_ = divide * 3;
    size_t size = sizeof(VertexData) * vertexCount_;
    vertexResource_ = CreateBuffer(device, size);

    vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = static_cast<UINT>(size);
    vertexBufferView_.StrideInBytes = sizeof(VertexData);

    VertexData* vertexData = nullptr;
    vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

    float radianPreDivide = 2.0f * std::numbers::pi_v<float> / float(divide);

    for (uint32_t index = 0; index < divide; ++index) {
        float sin = std::sin(index * radianPreDivide);
        float cos = std::cos(index * radianPreDivide);
        float sinNext = std::sin((index + 1) * radianPreDivide);
        float cosNext = std::cos((index + 1) * radianPreDivide);

        float u = float(index) / float(divide);
        float uNext = float(index + 1) / float(divide);

        uint32_t offset = index * 3;

        Vector4 apex = { 0.0f, 0.0f, height, 1.0f };
        Vector4 pCurrent = { -sin * bottomRadius, cos * bottomRadius, 0.0f, 1.0f };
        Vector4 pNext = { -sinNext * bottomRadius, cosNext * bottomRadius, 0.0f, 1.0f };

        Vector3 n0 = Normalize(Vector3 { -sin, cos, bottomRadius / height });
        Vector3 n1 = Normalize(Vector3 { -sinNext, cosNext, bottomRadius / height });

        // 表面（時計回り）のみ登録
        vertexData[offset + 0] = { pCurrent, { u, 0.0f }, n0 };
        vertexData[offset + 1] = { apex, { u + 0.5f / divide, 1.0f }, { 0.0f, 0.0f, 1.0f } };
        vertexData[offset + 2] = { pNext, { uNext, 0.0f }, n1 };
    }

    vertexResource_->Unmap(0, nullptr);
}