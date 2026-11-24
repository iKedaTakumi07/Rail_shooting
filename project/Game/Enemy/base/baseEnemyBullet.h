#pragma once
#include "../../../Engine/base/Math.h"
class Camera;

#include "../../OnCollison/Collider.h"

class baseEnemyBullet : public Collider {
public:
    virtual void Initialize(Vector3 pos, const Vector3& rotation);

    virtual void Update(float deltaTime);
    virtual void UpdatePause() = 0;

    virtual void Draw();

    // Get関数
    virtual bool GetIsDead() const = 0;
    // set関数
    virtual void SetPlayerPos(Vector3 pos) { };
};
