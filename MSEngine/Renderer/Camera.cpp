#include "Camera.h"

void Camera::MoveForward(float speed)
{
    XMVECTOR pos = XMLoadFloat3(&position);
    XMVECTOR fwd = XMLoadFloat3(&forward);
    pos = pos + fwd * speed;
    XMStoreFloat3(&position, pos);
}

void Camera::MoveRight(float speed)
{
    XMVECTOR pos = XMLoadFloat3(&position);
    XMVECTOR fwd = XMLoadFloat3(&forward);
    XMVECTOR use_up = XMLoadFloat3(&up);

    XMVECTOR right = XMVector3Normalize(XMVector3Cross(use_up, fwd));
    pos = pos + right * speed;

    XMStoreFloat3(&position, pos);
}

XMMATRIX Camera::GetViewMatrix() const
{
    XMVECTOR pos = XMLoadFloat3(&position);
    XMVECTOR fwd = XMLoadFloat3(&forward);
    XMVECTOR use_up = XMLoadFloat3(&up);

    fwd = XMVector3Normalize(fwd);

    XMVECTOR at = pos + fwd;

    return XMMatrixLookAtLH(pos, at, use_up);
}