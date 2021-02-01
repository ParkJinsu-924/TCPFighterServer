#pragma once
#include <Windows.h>
#define QUEUE_SIZE 10000

// TODO LIST
//

// ===========================================================
// CRingBuffer ( ver 1.0.2 )
//
// 환영큐로 만들어진 RingBuffer Class
//
// Update
// 1. 모든 함수 front + 1, rear + 1 이때 오류 나는거 수정.
//
// Last Update = 2020 - 09 - 06 
// ===========================================================
class CRingBuffer
{
private:
	// RingBuffer 시작 지점
	char* m_RingBuffer;

	// 생성된 RingBuffer 크기
	DWORD m_BufferSize = 0;

	int	  m_front = 0;
	int	  m_rear = 0;

public:
	// ===========================================================
	// 기본 생성자
	//
	// QUEUE_SIZE 바이트 짜리 RingBuffer를 생성한다. ( 기본값 10000 Byte )
	// ===========================================================
	CRingBuffer(void)
	{
		m_RingBuffer = (char*)malloc(QUEUE_SIZE);
		m_BufferSize = QUEUE_SIZE;
	}

	// ===========================================================
	// 오버로딩 생성자
	//
	// 인자로 받은 크기 RingBuffer 생성한다.
	//
	// Parameter : (int) 생성할 링버퍼 크기.
	// ===========================================================
	CRingBuffer(int iBufferSize)
	{
		m_RingBuffer = (char*)malloc(iBufferSize);
		m_BufferSize = iBufferSize;
	}

	//void Initial(int iBufferSize);

	// ===========================================================
	// GetBufferSize
	//
	// 생성된 링버퍼의 사이즈 얻기.
	//
	// Parameter : (void) 없음.
	// Return    : (int) 링버퍼의 크기.
	// ===========================================================
	int GetBufferSize(void)
	{
		return m_BufferSize;
	}

	// ===========================================================
	// GetUseSize
	//
	// 현재 사용중인 용량 얻기.
	//
	// Parameter : (void) 없음.
	// Return    : (int) 사용중인 용량.
	// ===========================================================
	int GetUseSize(void)
	{
		if (m_front > m_rear)
			return m_BufferSize - m_front + m_rear;
		else
			return m_rear - m_front;
	}

	// ===========================================================
	// GetFreeSize
	//
	// 현재 버퍼에 남은 용량 얻기.
	//
	// Parameter : (void) 없음.
	// Return	 : (int) 남은용량. 
	// ===========================================================
	int GetFreeSize(void)
	{
		if (m_front > m_rear)
			return m_front - m_rear - 1;
		else
			return m_BufferSize - m_rear + m_front - 1;
	}

	// ===========================================================
	// GetDirectEnqueueSize
	//
	// 버퍼 포인터로 외부에서 한방에 쓸 수 있는 길이.
	// (끊기지 않은 길이)
	//
	// 원형 큐의 구조상 버퍼의 끝에 있는 데이터는 인덱스 처음으로 돌아가서 데이터를 넣는다.
	// 그러므로 recv를 통해 데이터를 넣기 위해선 상황에 따라서 2번에 작업을 해야한다.
	//
	// Parameter : (void) 없음.
	// Return	 : (int) 사용가능 용량.
	// ===========================================================
	int GetDirectEnqueueSize(void)
	{
		if (GetFreeSize() == 0)
			return 0;

		if (m_front > m_rear)
			return m_front - m_rear - 1;
		else
		{
			if (m_BufferSize - m_rear - 1 == 0)//완전히 꽉 찼다.
				return GetFreeSize();
			return m_BufferSize - m_rear - 1;
		}
	}

	// ===========================================================
	// GetDirectDequeueSize
	//
	// 버퍼 포인터로 외부에서 한방에 읽을 수 있는 길이.
	// (끊기지 않은 길이)
	//
	// 원형 큐의 구조상 버퍼의 끝에 있는 데이터는 인덱스 처음으로 돌아가서 데이터를 넣는다.
	//
	// Parameter : (void) 없음.
	// Return	 : (int) 읽기 가능 용량.
	// ===========================================================
	int GetDirectDequeueSize(void)
	{
		if (GetUseSize() == 0)
			return 0;

		if (m_front > m_rear)
			return m_BufferSize - m_front - 1;
		else
			return m_rear - m_front;
	}

	// ===========================================================
	// Enqueue
	//
	// for문으로 데이터를 넣지 않고 memcpy로 한 이유는 멀티 스레드 동기화 문제가
	// 발생할 수 도 있기 때문이다.
	//
	// Parameters : (char *) 데이터 포인터, (int) 크기.
	// Return     : (int) 넣은 크기.
	// ===========================================================
	int Enqueue(char* chpData, int iSize)
	{
		// EnQueue 성공한 크기
		int retval = 0;

		// RingBUffer가 꽉찼다면 바로 return
		if (GetFreeSize() == 0)
			return 0;

		// 한번에 EnQueue 할 수 있는 크기
		int directSize = GetDirectEnqueueSize();

		// EnQueue할 데이터 크기
		int inputSize = iSize;

		// input 할 데이터 크기가 한번에 넣을 수 있는 데이터 크기 보다 크다면 directSize 만큼 넣도록 한다.
		if (inputSize > directSize)
		{
			inputSize = directSize;
		}

		// 링버퍼로 한번에 넣을 수 있는 크기만큼 memcpy
		memcpy_s(&m_RingBuffer[(m_rear + 1) % m_BufferSize], inputSize, chpData, inputSize);
		retval = inputSize;

		// iSize == inputSize 이라면 정상적으로 데이터를 잘 넣은거다.
		// iSize != inputSize 이라면 2가지 경우가 존재한다.
		// 링버퍼 공간이 꽉찾을 경우와 링버퍼 공간이 남아있을 경우
		// 링버퍼 공간이 꽉찾을 경우는 더이상 데이터를 넣으면 안된다.
		// 링버퍼 공간이 남아있다면 남은 공간만큼 데이터를 넣어준다.
		if (iSize != inputSize && GetFreeSize() != inputSize) //
		{
			directSize = iSize - inputSize;

			if (directSize > GetFreeSize() - inputSize)
			{
				directSize = GetFreeSize() - inputSize;
			}

			memcpy_s(&m_RingBuffer[0], directSize, &chpData[inputSize], directSize);
			retval = directSize + inputSize;
		}

		// 데이터 넣은 만큼 Rear 이동
		MoveRear(retval);

		return retval;
	}

	// ===========================================================
	// Dequeue
	//
	// for문으로 데이터를 넣지 않고 memcpy로 한 이유는 멀티 스레드 동기화 문제가
	// 발생할 수 도 있기 때문이다.
	//
	// Parameters : (char *) 데이터 포인터, (int) 크기.
	// Return     : (int) 가져온 크기.
	// ===========================================================
	int Dequeue(char* chpDest, int iSize)
	{
		// DeQueue 성공한 크기
		int retval = 0;

		if (GetUseSize() == 0)
			return 0;

		// 한번에 DeQueue 할 수 있는 크기
		int directSize = GetDirectDequeueSize();

		// DeQueue할 데이터 크기
		int outputSize = iSize;

		// output 할 데이터 크기가 한번에 뺄 수 있는 데이터 크기 보다 크다면 directSize 만큼 넣도록 한다.
		if (directSize < outputSize)
		{
			outputSize = directSize;
		}

		// 링버퍼로 한번에 뺄 수 있는 크기만큼 memcpy
		memcpy_s(chpDest, outputSize, &m_RingBuffer[(m_front + 1) % m_BufferSize], outputSize);
		retval = outputSize;

		// iSize == outputSize 이라면 잘뽑은 거니 더이상 뽑기를 시도하지 않는다.
		// iSize != outputSize 이라면 경우에 수가 2가지 존재한다.
		//
		// 더 이상 뽑을 데이터가 없을때와 있을때
		// 만약 GetUseSize == outputSize 이라면 더 이상 뽑을 데이터가 없다라는 뜻이고
		// GetUseSize != outputSize 다르다면 더 뽑을 데이터가 있다는 의미다.
		if (iSize != outputSize && GetUseSize() != outputSize)
		{
			directSize = iSize - outputSize;

			if (directSize > GetUseSize() - outputSize)
			{
				directSize = GetUseSize() - outputSize;
			}

			memcpy_s(&chpDest[outputSize], directSize, &m_RingBuffer[0], directSize);
			retval = directSize + outputSize;
		}

		// 데이터 뺀 만큼 front 이동
		MoveFront(retval);

		return retval;
	}

	// ===========================================================
	// Peek
	//
	// for문으로 데이터를 넣지 않고 memcpy로 한 이유는 멀티 스레드 동기화 문제가
	// 발생할 수 도 있기 때문이다.
	//
	// Parameters : (char *) 데이터 포인터, (int) 크기.
	// Return     : (int) 읽온 크기.
	// ===========================================================
	int Peek(char* chpDest, int iSize)
	{
		// Peek 성공한 크기
		int retval = 0;

		if (GetUseSize() == 0)
			return 0;

		// 한번에 Peek 할 수 있는 크기
		int directSize = GetDirectDequeueSize();

		// Peek할 데이터 크기
		int peekSize = iSize;

		// peek 할 데이터 크기가 한번에 읽을 수 있는 데이터 크기 보다 크다면 directSize 만큼 넣도록 한다.
		if (directSize < peekSize)
		{
			peekSize = directSize;
		}

		// 링버퍼로 한번에 읽을 수 있는 크기만큼 memcpy
		memcpy_s(chpDest, peekSize, &m_RingBuffer[(m_front + 1) % m_BufferSize], peekSize);
		retval = peekSize;

		// iSize == peekSize 이라면 잘뽑은 거니 더이상 읽기를 시도하지 않는다.
		// iSize != peekSize 이라면 경우에 수가 2가지 존재한다.
		//
		// 더 이상 읽을 데이터가 없을때와 있을때
		// 만약 GetUseSize == peekSize 이라면 더 이상 읽을 데이터가 없다라는 뜻이고
		// GetUseSize != peekSize 다르다면 더 읽을 데이터가 있다는 의미다.
		if (iSize != peekSize && GetUseSize() != peekSize)
		{
			directSize = iSize - peekSize;

			if (directSize > GetUseSize() - peekSize)
			{
				directSize = GetUseSize() - peekSize;
			}

			memcpy_s(&chpDest[peekSize], directSize, &m_RingBuffer[0], directSize);
			retval = directSize + peekSize;
		}

		return retval;
	}

	// ===========================================================
	// MoveRear
	//
	// 원하는 길이만큼 읽기위치 에서 삭제 / 쓰기 위치 이동
	//
	// Parameter : (int) 이동 하고싶은 크기.
	// Return    : (void) 없음.
	// ===========================================================
	void MoveRear(int iSize)
	{
		m_rear = (m_rear + iSize) % m_BufferSize;
	}
	void MoveFront(int iSize)
	{
		m_front = (m_front + iSize) % m_BufferSize;
	}

	// ===========================================================
	// ClearBuffer
	//
	// 버퍼의 모든 데이터 삭제.
	//
	// Parameter : (void) 없음.
	// Return	 : (void) 없음.
	// ===========================================================
	void ClearBuffer(void)
	{
		m_front = 0;
		m_rear = 0;
	}

	// ===========================================================
	// GetFrontBufferPtr
	//
	// 버퍼의 Front 포인터 얻음.
	//
	// Parameter : (void) 없음.
	// Return	 : (char *) &RingBuffer[m_front + 1].
	// ===========================================================
	char* GetFrontBufferPtr(void)
	{
		return &m_RingBuffer[(m_front + 1) % m_BufferSize];
	}

	// ===========================================================
	// GetRearBufferPtr
	//
	// 버퍼의 RearPos 포인터 얻음.
	//
	// Parameter : (void) 없음.
	// Return	 : (char *) &RingBuffer[m_rear + 1].
	// ===========================================================
	char* GetRearBufferPtr(void)
	{
		return &m_RingBuffer[(m_rear + 1) % m_BufferSize];
	}
};