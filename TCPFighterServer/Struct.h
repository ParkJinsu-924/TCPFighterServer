#pragma once

struct stSESSION
{
	SOCKET Socket;				//�� ������ TCP ����
	DWORD dwSessionID;			//�������� ���� ���� ID
	CRingBuffer RecvQ;			//���� ť
	CRingBuffer SendQ;			//�۽� ť
	DWORD dwLastRecvTime;		//�޽��� ���� üũ�� ���� �ð�(Ÿ�Ӿƿ���)
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

//���ο� Ŭ���̾�Ʈ�� ������ �� ���� ��ü�� ĳ���� ��ü�� �ű� �Ҵ��Ͽ� ����մϴ�.
struct stCHARACTER
{
	stSESSION* pSession;
	DWORD dwSessionID;

	DWORD dwAction;
	BYTE byDirection;
	BYTE byMoveDirection;

	short shX;
	short shY;

	//���� ����
	stSECTOR_POS CurSector;
	stSECTOR_POS OldSector;

	char chHP;
};