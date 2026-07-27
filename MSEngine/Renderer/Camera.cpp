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
    // XMVector3Cross로 오른쪽 벡터 구하고, MoveForward랑 같은 패턴으로 position 갱신
    XMVECTOR pos = XMLoadFloat3(&position);
    XMVECTOR fwd = XMLoadFloat3(&forward);
    XMVECTOR use_up = XMLoadFloat3(&up);

    XMVECTOR right = XMVector3Normalize(XMVector3Cross(use_up, fwd));
    pos = pos + right * speed;

    XMStoreFloat3(&position, pos);
}

XMMATRIX Camera::GetViewMatrix() const
{
    // at = position + forward, 그리고 XMMatrixLookAtLH 호출
    // XMLoadFloat3로 position/forward/up 전부 변환해야 함
    XMVECTOR pos = XMLoadFloat3(&position);
    XMVECTOR fwd = XMLoadFloat3(&forward);
    XMVECTOR use_up = XMLoadFloat3(&up);

    fwd = XMVector3Normalize(fwd); // 정규화

    XMVECTOR at = pos + fwd;

    return XMMatrixLookAtLH(pos, at, use_up);
}