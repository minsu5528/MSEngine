#pragma once
#include <windows.h>

class Timer {
private:
    LARGE_INTEGER m_frequency;    // CPU 주파수 (1초에 몇 번 째깍거리는가?)
    LARGE_INTEGER m_prevTime;     // 이전 프레임의 시간
    LARGE_INTEGER m_currentTime;  // 현재 프레임의 시간

    float m_deltaTime;            // 프레임 간 걸린 시간 (초 단위)

public:
    Timer();                      // 주파수와 초기 시간을 세팅

    void Tick();                  // 시간 계산 함수
    float GetDeltaTime() const;   // 시간 불러오기 함수
};