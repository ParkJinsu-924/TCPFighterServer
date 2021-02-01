#pragma once

struct stSESSION
{
	SOCKET Socket;				//현 접속의 TCP 소켓
	DWORD dwSessionID;			//접속자의 고유 세션 ID
	CRingBuffer RecvQ;			//수신 큐
	CRingBuffer SendQ;			//송신 큐
	DWORD dwLastRecvTime;		//메시지 수신 체크를 위한 시간(타임아웃용)
};

struct stSECTOR_POS
{
	int iX;
	int iY;
};

struct stSECTOR_AROUND
{
	int iCount;
	stSECTOR_POS Around[9];
};

//새로운 클라이언트가 접속할 시 세션 객체와 캐릭터 객체를 신규 할당하여 등록합니다.
struct stCHARACTER
{
	stSESSION* pSession;
	DWORD dwSessionID;

	DWORD dwAction;
	BYTE byDirection;
	BYTE byMoveDirection;

	short shX;
	short shY;

	//섹터 정보
	stSECTOR_POS CurSector;
	stSECTOR_POS OldSector;

	char chHP;
};