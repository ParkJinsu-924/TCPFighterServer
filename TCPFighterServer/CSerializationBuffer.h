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

	//패킷파괴
	void Release();

	//패킷청소
	void Clear();

	//버퍼사이즈얻기
	int GetBufferSize();

	//버퍼포인터얻기
	char* GetBufferPtr();

	//버퍼pos이동, 음수이동은 안됨
	int MoveWritePos(int iSize);
	int MoveReadPos(int iSize);

	CSerializationBuffer& operator=(CSerializationBuffer& clSrcPacket);

	//각 타입 마다 변수 모두 만듦, 넣기
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

	//빼기
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

	//버퍼의 시작점
	char* chpBufferPtr;
};
#endif
