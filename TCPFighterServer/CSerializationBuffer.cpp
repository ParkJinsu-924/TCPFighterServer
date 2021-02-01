#include <iostream>
#include <Windows.h>
#include "CSerializationBuffer.h"

CSerializationBuffer::CSerializationBuffer()
{
	chpBufferPtr = new char[eBUFFER_DEFAULT];
	iBufferSize = eBUFFER_DEFAULT;
	chpWritePos = chpBufferPtr;
	chpReadPos = chpWritePos;
}

CSerializationBuffer::CSerializationBuffer(int iBufferSize)
{
	chpBufferPtr = new char[iBufferSize];
	this->iBufferSize = iBufferSize;
	chpWritePos = chpBufferPtr;
	chpReadPos = chpBufferPtr;
}

CSerializationBuffer::~CSerializationBuffer()
{
	delete chpBufferPtr;
}

void CSerializationBuffer::Release()
{
	this->~CSerializationBuffer();
}

void CSerializationBuffer::Clear()
{
	chpWritePos = chpBufferPtr;
	chpReadPos = chpBufferPtr;
}

int CSerializationBuffer::GetBufferSize()
{
	return iBufferSize;
}

char* CSerializationBuffer::GetBufferPtr()
{
	return chpBufferPtr;
}

int CSerializationBuffer::MoveWritePos(int iSize)
{
	chpWritePos += iSize;
	return iSize;
}

int CSerializationBuffer::MoveReadPos(int iSize)
{
	chpReadPos += iSize;
	return iSize;
}

CSerializationBuffer& CSerializationBuffer::operator=(CSerializationBuffer& clSrcPacket)
{
	// TODO: 여기에 return 문을 삽입합니다.
	return *this;
}

CSerializationBuffer& CSerializationBuffer::operator<<(char chValue)
{
	if (GetFreeSize() < sizeof(char))
		return *this;

	*chpWritePos = chValue;
	chpWritePos += sizeof(char);
	return *this;
}

CSerializationBuffer& CSerializationBuffer::operator<<(short shValue)
{
	if (GetFreeSize() < sizeof(short))
		return *this;

	(*(short*)chpWritePos) = shValue;
	chpWritePos += sizeof(short);
	return *this;
}

CSerializationBuffer& CSerializationBuffer::operator<<(int iValue)
{
	if (GetFreeSize() < sizeof(int))
		return *this;

	(*(int*)chpWritePos) = iValue;
	chpWritePos += sizeof(int);
	return *this;
}

CSerializationBuffer& CSerializationBuffer::operator<<(float fValue)
{
	if (GetFreeSize() < sizeof(float))
		return *this;

	(*(float*)chpWritePos) = fValue;
	chpWritePos += sizeof(float);
	return *this;
}

CSerializationBuffer& CSerializationBuffer::operator<<(__int64 iValue)
{
	if (GetFreeSize() < sizeof(__int64))
		return *this;

	(*(__int64*)chpWritePos) = iValue;
	chpWritePos += sizeof(__int64);
	return *this;
}

CSerializationBuffer& CSerializationBuffer::operator<<(double dValue)
{
	if (GetFreeSize() < sizeof(double))
		return *this;

	(*(double*)chpWritePos) = dValue;
	chpWritePos += sizeof(double);
	return *this;
}

CSerializationBuffer& CSerializationBuffer::operator<<(wchar_t wchValue)
{
	if (GetFreeSize() < sizeof(wchar_t))
		return *this;

	(*(wchar_t*)chpWritePos) = wchValue;
	chpWritePos += sizeof(wchar_t);
	return *this;
}

CSerializationBuffer& CSerializationBuffer::operator<<(BYTE byValue)
{
	if (GetFreeSize() < sizeof(BYTE))
		return *this;

	(*(BYTE*)chpWritePos) = byValue;
	chpWritePos += sizeof(BYTE);
	return *this;
}

CSerializationBuffer& CSerializationBuffer::operator<<(WORD wValue)
{
	if (GetFreeSize() < sizeof(WORD))
		return *this;

	(*(WORD*)chpWritePos) = wValue;
	chpWritePos += sizeof(WORD);
	return *this;
}

CSerializationBuffer& CSerializationBuffer::operator<<(DWORD dwValue)
{
	if (GetFreeSize() < sizeof(DWORD))
		return *this;

	(*(DWORD*)chpWritePos) = dwValue;
	chpWritePos += sizeof(DWORD);
	return *this;
}

CSerializationBuffer& CSerializationBuffer::operator>>(char& chValue)
{
	if (GetUseSize() < sizeof(char))
	{
		Exception e;
		e.SetExceptionCode(dfMESSAGE_NOT_PICK);
		throw e;
	}

	chValue = *chpReadPos;
	chpReadPos += sizeof(char);
	return *this;
}

CSerializationBuffer& CSerializationBuffer::operator>>(short& shValue)
{
	if (GetUseSize() < sizeof(short))
	{
		Exception e;
		e.SetExceptionCode(dfMESSAGE_NOT_PICK);
		throw e;
	}

	shValue = (*(short*)chpReadPos);
	chpReadPos += sizeof(short);
	return *this;
}

CSerializationBuffer& CSerializationBuffer::operator>>(int& iValue)
{
	if (GetUseSize() < sizeof(int))
	{
		Exception e;
		e.SetExceptionCode(dfMESSAGE_NOT_PICK);
		throw e;
	}

	iValue = (*(int*)chpReadPos);
	chpReadPos += sizeof(int);
	return *this;
}

CSerializationBuffer& CSerializationBuffer::operator>>(float& fValue)
{
	if (GetUseSize() < sizeof(float))
	{
		Exception e;
		e.SetExceptionCode(dfMESSAGE_NOT_PICK);
		throw e;
	}

	fValue = (*(float*)chpReadPos);
	chpReadPos += sizeof(float);
	return *this;
}

CSerializationBuffer& CSerializationBuffer::operator>>(__int64& iValue)
{
	if (GetUseSize() < sizeof(__int64))
	{
		Exception e;
		e.SetExceptionCode(dfMESSAGE_NOT_PICK);
		throw e;
	}

	iValue = (*(__int64*)chpReadPos);
	chpReadPos += sizeof(__int64);
	return *this;
}

CSerializationBuffer& CSerializationBuffer::operator>>(double& dValue)
{
	if (GetUseSize() < sizeof(double))
	{
		Exception e;
		e.SetExceptionCode(dfMESSAGE_NOT_PICK);
		throw e;
	}

	dValue = (*(double*)chpReadPos);
	chpReadPos += sizeof(double);
	return *this;
}

CSerializationBuffer& CSerializationBuffer::operator>>(wchar_t& dValue)
{
	if (GetUseSize() < sizeof(wchar_t))
	{
		Exception e;
		e.SetExceptionCode(dfMESSAGE_NOT_PICK);
		throw e;
	}

	dValue = (*(wchar_t*)chpReadPos);
	chpReadPos += sizeof(wchar_t);
	return *this;
}

CSerializationBuffer& CSerializationBuffer::operator>>(BYTE& byValue)
{
	if (GetUseSize() < sizeof(BYTE))
	{
		Exception e;
		e.SetExceptionCode(dfMESSAGE_NOT_PICK);
		throw e;
	}

	byValue = (*(BYTE*)chpReadPos);
	chpReadPos += sizeof(BYTE);
	return *this;
}

CSerializationBuffer& CSerializationBuffer::operator>>(WORD& wValue)
{
	if (GetUseSize() < sizeof(WORD))
	{
		Exception e;
		e.SetExceptionCode(dfMESSAGE_NOT_PICK);
		throw e;
	}

	wValue = (*(WORD*)chpReadPos);
	chpReadPos += sizeof(BYTE);
	return *this;
}

CSerializationBuffer& CSerializationBuffer::operator>>(DWORD& dwValue)
{
	if (GetUseSize() < sizeof(DWORD))
	{
		Exception e;
		e.SetExceptionCode(dfMESSAGE_NOT_PICK);
		throw e;
	}

	dwValue = (*(DWORD*)chpReadPos);
	chpReadPos += sizeof(DWORD);
	return *this;
}

int CSerializationBuffer::GetData(char* chpDest, int iSize)
{
	memcpy(chpDest, chpReadPos, iSize);
	chpReadPos += iSize;
	return *chpDest;
}

int CSerializationBuffer::PutData(char* chpSrc, int iSrcSize)
{
	memcpy(chpWritePos, chpSrc, iSrcSize);
	chpWritePos += iSrcSize;
	return *chpSrc;
}

int CSerializationBuffer::GetUseSize()
{
	return (int)chpWritePos - (int)chpReadPos;
}

int CSerializationBuffer::GetFreeSize()
{
	return chpBufferPtr + iBufferSize - chpWritePos;
}
