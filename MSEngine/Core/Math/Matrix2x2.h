#pragma once
#include "Vector2.h"
#include <cmath>

struct Matrix2x2 {
    // 2x2 행렬의 요소들 (m11, m12, m21, m22)
    float m[2][2];

    // 생성자 (기본적으로 단위 행렬로 초기화)
    Matrix2x2() {
        m[0][0] = 1.0f; m[0][1] = 0.0f;
        m[1][0] = 0.0f; m[1][1] = 1.0f;
    }

    // 벡터와 행렬의 곱셈 (회전 적용)
    Vector2 Multiply(const Vector2& vec) const {
        return Vector2(
            (m[0][0] * vec.x) + (m[0][1] * vec.y),
            (m[1][0] * vec.x) + (m[1][1] * vec.y)
        );
    }

    // 회전 행렬로 셋팅하는 함수 (입력값: 라디안 각도)
    void SetRotation(float radian) {
        m[0][0] = std::cos(radian); m[0][1] = -std::sin(radian);
        m[1][0] = std::sin(radian); m[1][1] = std::cos(radian);
    }
};