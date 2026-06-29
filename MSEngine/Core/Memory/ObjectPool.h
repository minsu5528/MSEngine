#pragma once
#include <iostream>

template <typename T, int PoolSize = 256>
class ObjectPool {
private:
    // 1. 실제 객체들이 생성되어 머물 거대한 빈 땅 (256칸)
    T m_pool[PoolSize];

    // 2. 현재 비어있는 '인덱스 번호'들을 담아둘 스택
    int m_freeIndices[PoolSize];

    // 3. m_freeIndices 스택의 최상단을 가리키는 지시자
    int m_top;

public:
    ObjectPool() {
        // 생성자: 처음에 0번부터 255번까지 모든 인덱스가 비어있다고 스택에 채워 넣음
        m_top = 0;
        for (int i = 0; i < PoolSize; ++i) {
            m_freeIndices[i] = i;
        }
    }

    // 객체 할당
    T* Allocate() {
        // 1. 방어 코드: 금고가 다 비었으면 포인터 없음을 뜻하는 nullptr 반환
        if (m_top >= PoolSize) {
            std::cout << "[Warning] 풀이 꽉 찼습니다! 할당 거부.\n";
            return nullptr;
        }

        // 2. 스택(m_freeIndices)에서 현재 비어있는 '인덱스 번호'를 하나 뽑음
        int index = m_freeIndices[m_top];
        m_top++; // 다음 빈자리를 가리키도록 스택 포인터 1 증가

        // 3. 해당 인덱스에 위치한 m_pool의 "주소(&)"를 반환
        return &m_pool[index];
    }

    // 객체 반납
    void Free(T* ptr) {
        // 1. 방어 코드: 이상한 주소가 들어오면 무시
        if (ptr == nullptr) {
            std::cout << "[Warning] 비정상 반납 시도 (nullptr).\n";
            return;
        }

        if (ptr < m_pool || ptr >= (m_pool + PoolSize)) {
            std::cout << "[Error] 이 풀에서 빌려준 주소가 아닙니다!\n";
            return;
        }

        // 2. 포인터 연산: 반납된 주소가 몇 번 인덱스인지 계산
        int index = ptr - m_pool;

        // 3. 다시 스택(m_freeIndices)에 반납된 인덱스를 기록
        m_top--;                  // 스택 포인터를 한 칸 내리고
        m_freeIndices[m_top] = index; // 그 자리에 방금 반납받은 빈 방 번호를 적어둠
    }

    // --- 테스트 확인용 도우미 함수들 ---
    int GetAvailableCount() const { return PoolSize - m_top; } // 남은 방 개수
    int GetTopIndex() const { return m_top; }
};