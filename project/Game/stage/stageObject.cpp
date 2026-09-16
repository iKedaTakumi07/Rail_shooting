#include "stageObject.h"

#include "../../Engine/3d/CameraManager.h"
#include "../../Engine/3d/ModelManager.h"
#include "../../Engine/3d/Object3d.h"
#include "../../Engine/base/Math.h"
#include "../../Engine/base/TextureManager.h"

void stageObject::Initialize(const std::string& patan, const Vector3& pos, const Vector3& scale)
{
    TextureManager::getInstance()->LoadTexture("resources/stage/uvChecker.png");
    ModelManager::GetInstance()->LoadModel("stage/stageCube1.obj");
    ModelManager::GetInstance()->LoadModel("stage/stageObjectCube.obj");

    camera_ = CameraManager::GetInstance()->GetActiveCamera();
    objPatan_ = patan;

    transform_.translate = pos;
    transform_.scale = scale;
    transform_.rotate = { 0.0f, 0.0f, 0.0f };

    Object3d_ = std::make_unique<Object3d>();
    Object3d_->Initialize();

    ObjectModel = std::make_unique<Model>();
    ObjectModel->Initialize("resources/stage", objPatan_ + ".obj"); // 指定したパターンに
    Object3d_->SetModel(ObjectModel.get());

    Object3d_->SetTranslate(transform_.translate);
    Object3d_->SetScale(transform_.scale);
    Object3d_->SetRotate(transform_.rotate);

    if (objPatan_ == "stageCube1") {
        parts_[0] = { { -1.0f, -1.0f, -1.0f }, { -0.5f, 0.5f, 1.0f } }; // 右
        parts_[1] = { { 0.5f, -1.0f, -1.0f }, { 1.0f, 0.5f, 1.0f } }; // 左
        parts_[2] = { { -1.0f, 0.5f, -1.0f }, { 1.0f, 1.0f, 1.0f } }; // 屋根
    } else {
        parts_[0] = { { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f } }; // エラー対策
        parts_[1] = { { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f } }; // エラー対策
        parts_[2] = { { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f } }; // エラー対策
    }
}

void stageObject::Update()
{
    camera_ = CameraManager::GetInstance()->GetActiveCamera();

    Object3d_->SetTranslate(transform_.translate);
    Object3d_->SetScale(transform_.scale);
    Object3d_->SetRotate(transform_.rotate);
    Object3d_->Update();
}

void stageObject::Draw()
{
    Object3d_->Draw();
}

AllAABB stageObject::GetAllAABB() const
{
    AABB aabb;
    AllAABB compound;
    float sizeX = baseSize_ * transform_.scale.x;
    float sizeY = baseSize_ * transform_.scale.y;
    float sizeZ = baseSize_ * transform_.scale.z;

    float halfSizeX = sizeX * 0.5f;
    float halfSizeY = sizeY * 0.5f;

    // 全体の大きさ(早期リターン用)
    aabb.min = { transform_.translate.x - sizeX, transform_.translate.y - sizeY, transform_.translate.z - sizeZ };
    aabb.max = { transform_.translate.x + sizeX, transform_.translate.y + sizeY, transform_.translate.z + sizeZ };

    compound.wholeBox = aabb;
    if (objPatan_ == "stageObjectCube") {
        compound.dividBoxes.push_back(aabb);
    } else if (objPatan_ == "stageCube1") {
        // 3分割した当たり判定にする

        AABB leftWall; // 左壁
        leftWall.min = { transform_.translate.x - sizeX, transform_.translate.y - sizeY, transform_.translate.z - sizeZ };
        leftWall.max = { transform_.translate.x - halfSizeX, transform_.translate.y + halfSizeY, transform_.translate.z + sizeZ };
        compound.dividBoxes.push_back(leftWall);

        AABB rightWall; // 右壁
        rightWall.min = { transform_.translate.x + halfSizeX, transform_.translate.y - sizeY, transform_.translate.z - sizeZ };
        rightWall.max = { transform_.translate.x + sizeX, transform_.translate.y + halfSizeY, transform_.translate.z + sizeZ };
        compound.dividBoxes.push_back(rightWall);

        AABB roof;
        roof.min = { transform_.translate.x - sizeX, transform_.translate.y + halfSizeY, transform_.translate.z - sizeZ };
        roof.max = { transform_.translate.x + sizeX, transform_.translate.y + sizeY, transform_.translate.z + sizeZ };
        compound.dividBoxes.push_back(roof);

    } else {
        // エラー対策
        compound.dividBoxes.push_back(aabb);
    }

    return compound;
}

AllOBB stageObject::GetAllOBB() const
{
    AllOBB compound;

    // 方向ベクトル
    Matrix4x4 rotX = MakeRotateXMatrix(transform_.rotate.x);
    Matrix4x4 rotY = MakeRotateYMatrix(transform_.rotate.y);
    Matrix4x4 rotZ = MakeRotateZMatrix(transform_.rotate.z);
    Matrix4x4 rotMat = Multiply(rotX, Multiply(rotY, rotZ));

    Vector3 orientations[3] = {
        Normalize({ rotMat.m[0][0], rotMat.m[0][1], rotMat.m[0][2] }),
        Normalize({ rotMat.m[1][0], rotMat.m[1][1], rotMat.m[1][2] }),
        Normalize({ rotMat.m[2][0], rotMat.m[2][1], rotMat.m[2][2] })
    };

    // 透けるに応じたsize変更
    Vector3 extents = {
        baseSize_ * transform_.scale.x,
        baseSize_ * transform_.scale.y,
        baseSize_ * transform_.scale.z
    };

    // 全体のOBB
    OBB wholeObb;
    wholeObb.center = transform_.translate;
    wholeObb.orientations[0] = orientations[0];
    wholeObb.orientations[1] = orientations[1];
    wholeObb.orientations[2] = orientations[2];
    wholeObb.size = extents;

    compound.wholeBox = wholeObb;

    if (objPatan_ == "stageObjectCube") {
        // 穴が開いているわけではないのでそのまま
        OBB box;
        box.center = transform_.translate;
        box.orientations[0] = orientations[0];
        box.orientations[1] = orientations[1];
        box.orientations[2] = orientations[2];
        box.size = extents;

        compound.dividBoxes.push_back(box);
    } else if (objPatan_ == "stageCube1") {
        for (const auto& part : parts_) {

            // ローカルの中心点とサイズを計算
            Vector3 localCenter = {
                (part.aabbMinOffset.x + part.aabbMaxOffset.x) / 2.0f * transform_.scale.x,
                (part.aabbMinOffset.y + part.aabbMaxOffset.y) / 2.0f * transform_.scale.y,
                (part.aabbMinOffset.z + part.aabbMaxOffset.z) / 2.0f * transform_.scale.z
            };
            Vector3 halfSize = {
                (part.aabbMaxOffset.x - part.aabbMinOffset.x) / 2.0f * transform_.scale.x,
                (part.aabbMaxOffset.y - part.aabbMinOffset.y) / 2.0f * transform_.scale.y,
                (part.aabbMaxOffset.z - part.aabbMinOffset.z) / 2.0f * transform_.scale.z
            };

            // ワールド座標の中心位置への変換
            Vector3 worldCenter = {
                transform_.translate.x + (localCenter.x * rotMat.m[0][0] + localCenter.y * rotMat.m[1][0] + localCenter.z * rotMat.m[2][0]),
                transform_.translate.y + (localCenter.x * rotMat.m[0][1] + localCenter.y * rotMat.m[1][1] + localCenter.z * rotMat.m[2][1]),
                transform_.translate.z + (localCenter.x * rotMat.m[0][2] + localCenter.y * rotMat.m[1][2] + localCenter.z * rotMat.m[2][2])
            };

            OBB box;
            box.center = worldCenter;
            box.orientations[0] = orientations[0];
            box.orientations[1] = orientations[1];
            box.orientations[2] = orientations[2];
            box.size = halfSize;

            compound.dividBoxes.push_back(box);
        }
    }
    return compound;
}

void stageObject::OnCollision(Collider* other)
{
    // 当たったもの次第で分岐
    if (other->GetCollisionGroup() == CollisionGroup::kEnemyBullet || other->GetCollisionGroup() == CollisionGroup::kEnenmy) {

        // 無敵時間のフラグ実行
    }
}
