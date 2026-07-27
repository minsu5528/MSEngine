#include "Timer.h"

Timer::Timer() : m_deltaTime(0.0f) {
    // 1. CPU의 고유 주파수(1초당 틱 횟수)를 가져와서 고정시킴
    QueryPerformanceFrequency(&m_frequency);

    // 2. 타이머의 최초 시작 시간을 기록 (스톱워치 버튼 ON)
    QueryPerformanceCounter(&m_prevTime);
}

void Timer::Tick() {
    // 1. 이번 프레임의 현재 시간 기록
    QueryPerformanceCounter(&m_currentTime);

    // 2. 델타 타임 계산: (현재 시간 - 아까 시간) / 주파수
    // (참고: QuadPart는 LARGE_INTEGER 구조체 안에서 실제 64비트 정수값을 꺼내오는 윈도우 전용 변수명이야)
    m_deltaTime = static_cast<float>(m_currentTime.QuadPart - m_prevTime.QuadPart) / static_cast<float>(m_frequency.QuadPart);

    // 3. 다음 프레임 계산을 위해 '현재 시간'을 '이전 시간'으로 덮어씌움 (랩타임 갱신)
    m_prevTime = m_currentTime;
}

float Timer::GetDeltaTime() const {
    return m_deltaTime;
}