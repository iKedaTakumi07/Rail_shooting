#include "Collider.h"

void Collider::UpdatePrevPosition(const Vector3& currentPos)
{
    if (!isPrevPositionInitialized_) {
        prevPosition_ = currentPos;
        currentPosition_ = currentPos;
        isPrevPositionInitialized_ = true;
        return;
    }
    prevPosition_ = currentPosition_;
    currentPosition_ = currentPos;
}

void Collider::ResetPrevPosition(const Vector3& pos)
{
    prevPosition_ = pos;
    currentPosition_ = pos;
    isPrevPositionInitialized_ = true;
}