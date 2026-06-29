#pragma once
#include <cmath>

struct Vector2 {
    float x;
    float y;

    // 1. 생성자
    Vector2() : x(0.0f), y(0.0f) {}
    Vector2(float _x, float _y) : x(_x), y(_y) {}

    // 2. 벡터의 덧셈 (operator+)
    Vector2 operator+(const Vector2& other) const {
        return Vector2(x + other.x, y + other.y);
    }

    // 3. 벡터의 내적 (Dot Product)
    float Dot(const Vector2& other) const {
        return (x * other.x) + (y * other.y);
    }

    // x값을 반올림한 뒤 안전하게 int로 캐스팅
    int GetRoundedX() const {
        return static_cast<int>(std::round(x));
    }

    // y값을 반올림한 뒤 안전하게 int로 캐스팅
    int GetRoundedY() const {
        return static_cast<int>(std::round(y));
    }
};