#pragma once
#include <Windows.h>
#define QUEUE_SIZE 10000

// TODO LIST
//

// ===========================================================
// CRingBuffer ( ver 1.0.2 )
//
// ȯ��ť�� ������� RingBuffer Class
//
// Update
// 1. ��� �Լ� front + 1, rear + 1 �̶� ���� ���°� ����.
//
// Last Update = 2020 - 09 - 06 
// ===========================================================
class CRingBuffer
{
private:
	// RingBuffer ���� ����
	char* m_RingBuffer;

	// ������ RingBuffer ũ��
	DWORD m_BufferSize = 0;

	int	  m_front = 0;
	int	  m_rear = 0;

public:
	// ===========================================================
	// �⺻ ������
	//
	// QUEUE_SIZE ����Ʈ ¥�� RingBuffer�� �����Ѵ�. ( �⺻�� 10000 Byte )
	// ===========================================================
	CRingBuffer(void)
	{
		m_RingBuffer = (char*)malloc(QUEUE_SIZE);
		m_BufferSize = QUEUE_SIZE;
	}

	// ===========================================================
	// �����ε� ������
	//
	// ���ڷ� ���� ũ�� RingBuffer �����Ѵ�.
	//
	// Parameter : (int) ������ ������ ũ��.
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
	// ������ �������� ������ ���.
	//
	// Parameter : (void) ����.
	// Return    : (int) �������� ũ��.
	// ===========================================================
	int GetBufferSize(void)
	{
		return m_BufferSize;
	}

	// ===========================================================
	// GetUseSize
	//
	// ���� ������� �뷮 ���.
	//
	// Parameter : (void) ����.
	// Return    : (int) ������� �뷮.
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
	// ���� ���ۿ� ���� �뷮 ���.
	//
	// Parameter : (void) ����.
	// Return	 : (int) �����뷮. 
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
	// ���� �����ͷ� �ܺο��� �ѹ濡 �� �� �ִ� ����.
	// (������ ���� ����)
	//
	// ���� ť�� ������ ������ ���� �ִ� �����ʹ� �ε��� ó������ ���ư��� �����͸� �ִ´�.
	// �׷��Ƿ� recv�� ���� �����͸� �ֱ� ���ؼ� ��Ȳ�� ���� 2���� �۾��� �ؾ��Ѵ�.
	//
	// Parameter : (void) ����.
	// Return	 : (int) ��밡�� �뷮.
	// ===========================================================
	int GetDirectEnqueueSize(void)
	{
		if (GetFreeSize() == 0)
			return 0;

		if (m_front > m_rear)
			return m_front - m_rear - 1;
		else
		{
			if (m_BufferSize - m_rear - 1 == 0)//������ �� á��.
				return GetFreeSize();
			return m_BufferSize - m_rear - 1;
		}
	}

	// ===========================================================
	// GetDirectDequeueSize
	//
	// ���� �����ͷ� �ܺο��� �ѹ濡 ���� �� �ִ� ����.
	// (������ ���� ����)
	//
	// ���� ť�� ������ ������ ���� �ִ� �����ʹ� �ε��� ó������ ���ư��� �����͸� �ִ´�.
	//
	// Parameter : (void) ����.
	// Return	 : (int) �б� ���� �뷮.
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
	// for������ �����͸� ���� �ʰ� memcpy�� �� ������ ��Ƽ ������ ����ȭ ������
	// �߻��� �� �� �ֱ� �����̴�.
	//
	// Parameters : (char *) ������ ������, (int) ũ��.
	// Return     : (int) ���� ũ��.
	// ===========================================================
	int Enqueue(char* chpData, int iSize)
	{
		// EnQueue ������ ũ��
		int retval = 0;

		// RingBUffer�� ��á�ٸ� �ٷ� return
		if (GetFreeSize() == 0)
			return 0;

		// �ѹ��� EnQueue �� �� �ִ� ũ��
		int directSize = GetDirectEnqueueSize();

		// EnQueue�� ������ ũ��
		int inputSize = iSize;

		// input �� ������ ũ�Ⱑ �ѹ��� ���� �� �ִ� ������ ũ�� ���� ũ�ٸ� directSize ��ŭ �ֵ��� �Ѵ�.
		if (inputSize > directSize)
		{
			inputSize = directSize;
		}

		// �����۷� �ѹ��� ���� �� �ִ� ũ�⸸ŭ memcpy
		memcpy_s(&m_RingBuffer[(m_rear + 1) % m_BufferSize], inputSize, chpData, inputSize);
		retval = inputSize;

		// iSize == inputSize �̶�� ���������� �����͸� �� �����Ŵ�.
		// iSize != inputSize �̶�� 2���� ��찡 �����Ѵ�.
		// ������ ������ ��ã�� ���� ������ ������ �������� ���
		// ������ ������ ��ã�� ���� ���̻� �����͸� ������ �ȵȴ�.
		// ������ ������ �����ִٸ� ���� ������ŭ �����͸� �־��ش�.
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

		// ������ ���� ��ŭ Rear �̵�
		MoveRear(retval);

		return retval;
	}

	// ===========================================================
	// Dequeue
	//
	// for������ �����͸� ���� �ʰ� memcpy�� �� ������ ��Ƽ ������ ����ȭ ������
	// �߻��� �� �� �ֱ� �����̴�.
	//
	// Parameters : (char *) ������ ������, (int) ũ��.
	// Return     : (int) ������ ũ��.
	// ===========================================================
	int Dequeue(char* chpDest, int iSize)
	{
		// DeQueue ������ ũ��
		int retval = 0;

		if (GetUseSize() == 0)
			return 0;

		// �ѹ��� DeQueue �� �� �ִ� ũ��
		int directSize = GetDirectDequeueSize();

		// DeQueue�� ������ ũ��
		int outputSize = iSize;

		// output �� ������ ũ�Ⱑ �ѹ��� �� �� �ִ� ������ ũ�� ���� ũ�ٸ� directSize ��ŭ �ֵ��� �Ѵ�.
		if (directSize < outputSize)
		{
			outputSize = directSize;
		}

		// �����۷� �ѹ��� �� �� �ִ� ũ�⸸ŭ memcpy
		memcpy_s(chpDest, outputSize, &m_RingBuffer[(m_front + 1) % m_BufferSize], outputSize);
		retval = outputSize;

		// iSize == outputSize �̶�� �߻��� �Ŵ� ���̻� �̱⸦ �õ����� �ʴ´�.
		// iSize != outputSize �̶�� ��쿡 ���� 2���� �����Ѵ�.
		//
		// �� �̻� ���� �����Ͱ� �������� ������
		// ���� GetUseSize == outputSize �̶�� �� �̻� ���� �����Ͱ� ���ٶ�� ���̰�
		// GetUseSize != outputSize �ٸ��ٸ� �� ���� �����Ͱ� �ִٴ� �ǹ̴�.
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

		// ������ �� ��ŭ front �̵�
		MoveFront(retval);

		return retval;
	}

	// ===========================================================
	// Peek
	//
	// for������ �����͸� ���� �ʰ� memcpy�� �� ������ ��Ƽ ������ ����ȭ ������
	// �߻��� �� �� �ֱ� �����̴�.
	//
	// Parameters : (char *) ������ ������, (int) ũ��.
	// Return     : (int) �п� ũ��.
	// ===========================================================
	int Peek(char* chpDest, int iSize)
	{
		// Peek ������ ũ��
		int retval = 0;

		if (GetUseSize() == 0)
			return 0;

		// �ѹ��� Peek �� �� �ִ� ũ��
		int directSize = GetDirectDequeueSize();

		// Peek�� ������ ũ��
		int peekSize = iSize;

		// peek �� ������ ũ�Ⱑ �ѹ��� ���� �� �ִ� ������ ũ�� ���� ũ�ٸ� directSize ��ŭ �ֵ��� �Ѵ�.
		if (directSize < peekSize)
		{
			peekSize = directSize;
		}

		// �����۷� �ѹ��� ���� �� �ִ� ũ�⸸ŭ memcpy
		memcpy_s(chpDest, peekSize, &m_RingBuffer[(m_front + 1) % m_BufferSize], peekSize);
		retval = peekSize;

		// iSize == peekSize �̶�� �߻��� �Ŵ� ���̻� �б⸦ �õ����� �ʴ´�.
		// iSize != peekSize �̶�� ��쿡 ���� 2���� �����Ѵ�.
		//
		// �� �̻� ���� �����Ͱ� �������� ������
		// ���� GetUseSize == peekSize �̶�� �� �̻� ���� �����Ͱ� ���ٶ�� ���̰�
		// GetUseSize != peekSize �ٸ��ٸ� �� ���� �����Ͱ� �ִٴ� �ǹ̴�.
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
	// ���ϴ� ���̸�ŭ �б���ġ ���� ���� / ���� ��ġ �̵�
	//
	// Parameter : (int) �̵� �ϰ���� ũ��.
	// Return    : (void) ����.
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
	// ������ ��� ������ ����.
	//
	// Parameter : (void) ����.
	// Return	 : (void) ����.
	// ===========================================================
	void ClearBuffer(void)
	{
		m_front = 0;
		m_rear = 0;
	}

	// ===========================================================
	// GetFrontBufferPtr
	//
	// ������ Front ������ ����.
	//
	// Parameter : (void) ����.
	// Return	 : (char *) &RingBuffer[m_front + 1].
	// ===========================================================
	char* GetFrontBufferPtr(void)
	{
		return &m_RingBuffer[(m_front + 1) % m_BufferSize];
	}

	// ===========================================================
	// GetRearBufferPtr
	//
	// ������ RearPos ������ ����.
	//
	// Parameter : (void) ����.
	// Return	 : (char *) &RingBuffer[m_rear + 1].
	// ===========================================================
	char* GetRearBufferPtr(void)
	{
		return &m_RingBuffer[(m_rear + 1) % m_BufferSize];
	}
};