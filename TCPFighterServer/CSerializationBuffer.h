#pragma once
#ifndef __PACKET__
#define __PACKET__
class CSerializationBuffer
{
public:
	class Exception
	{
#define dfMESSAGE_NOT_PICK	1

		int exceptionCode = 0;
	public:
		Exception() = default;
		void SetExceptionCode(int iCode) { exceptionCode = iCode; }
		
		int GetExceptionCode() { return exceptionCode; }
	};

	enum enPACKET
	{
		eBUFFER_DEFAULT = 1400
	};

	CSerializationBuffer();
	CSerializationBuffer(int iBufferSize);

	virtual ~CSerializationBuffer();

	//��Ŷ�ı�
	void Release();

	//��Ŷû��
	void Clear();

	//���ۻ�������
	int GetBufferSize();

	//���������;��
	char* GetBufferPtr();

	//����pos�̵�, �����̵��� �ȵ�
	int MoveWritePos(int iSize);
	int MoveReadPos(int iSize);

	CSerializationBuffer& operator=(CSerializationBuffer& clSrcPacket);

	//�� Ÿ�� ���� ���� ��� ����, �ֱ�
	CSerializationBuffer& operator<<(char chValue);
	CSerializationBuffer& operator<<(short shValue);
	CSerializationBuffer& operator<<(int iValue);
	CSerializationBuffer& operator<<(float fValue);
	CSerializationBuffer& operator<<(__int64 iValue);
	CSerializationBuffer& operator<<(double dValue);
	CSerializationBuffer& operator<<(wchar_t wchValue);

	CSerializationBuffer& operator<<(BYTE byValue);
	CSerializationBuffer& operator<<(WORD wValue);
	CSerializationBuffer& operator<<(DWORD dwValue);

	//����
	CSerializationBuffer& operator>>(char& chValue);
	CSerializationBuffer& operator>>(short& shValue);
	CSerializationBuffer& operator>>(int& iValue);
	CSerializationBuffer& operator>>(float& fValue);
	CSerializationBuffer& operator>>(__int64& iValue);
	CSerializationBuffer& operator>>(double& dValue);
	CSerializationBuffer& operator>>(wchar_t& dValue);

	CSerializationBuffer& operator>>(BYTE& byValue);
	CSerializationBuffer& operator>>(WORD& wValue);
	CSerializationBuffer& operator>>(DWORD& dwValue);


	int GetData(char* chpDest, int iSize);

	int PutData(char* chpSrc, int iSrcSize);

	int GetUseSize();

	int GetFreeSize();

protected:
	int iBufferSize;
	int iDataSize;
	char* chpReadPos;
	char* chpWritePos;

	//������ ������
	char* chpBufferPtr;
};
#endif
