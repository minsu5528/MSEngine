#pragma once
#include <DirectXMath.h>
using namespace DirectX;

class Camera {
private:
    XMFLOAT3 position = { 0.0f, 1.0f, -5.0f };
    XMFLOAT3 forward = { 0.0f, 0.0f,  1.0f };
    XMFLOAT3 up = { 0.0f, 1.0f,  0.0f };

public:
    void MoveForward(float speed);
    void MoveRight(float speed);
    XMMATRIX GetViewMatrix() const;
};