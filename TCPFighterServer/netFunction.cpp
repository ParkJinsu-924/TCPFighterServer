#include "Include.h"
#include "netFunction.h"

//DWORD g_netIOProcessCallCount = 0;
//DWORD g_netIOProcessTime = 0;

void Log(WCHAR* szString, int iLogLevel)
{
	wprintf(L"%s, lv. %d\n", szString, iLogLevel);
}

stSESSION* FindSession(SOCKET sock)
{
	return g_SessionMap.find(sock)->second;
}

stCHARACTER* FindCharacter(DWORD id)
{
	return g_CharacterMap.find(id)->second;
}

stSESSION* CreateSession(SOCKET sock)
{
	stSESSION* newSession = (stSESSION*)malloc(sizeof(stSESSION));
	g_ClientID++;
	newSession->Socket = sock;
	newSession->dwSessionID = g_ClientID;
	g_SessionMap.insert(make_pair(sock, newSession));
	return newSession;
}

void DisconnectSession(SOCKET sock)
{
	stSESSION* sessionPtr = g_SessionMap.find(sock)->second;

	stCHARACTER* ptr = g_CharacterMap.find(sessionPtr->dwSessionID)->second;

	CSerializationBuffer packet;

	mpDeleteCharacter(&packet, sessionPtr->dwSessionID);

	SendPacketAround(sessionPtr, &packet);

	g_Sector[ptr->CurSector.iY][ptr->CurSector.iX].remove(ptr);

	g_SessionMap.erase(sock);

	g_CharacterMap.erase(sessionPtr->dwSessionID);

	delete sessionPtr;

	delete ptr;
}

void netIOProcess()
{
	//DWORD startTime = timeGetTime();
	//g_netIOProcessCallCount++;

	//�����ؾߵ� ���ǵ��� ���� Disconnect���ش�.
	for (auto i = g_DeleteSet.begin(); i != g_DeleteSet.end(); ++i)
	{
		DisconnectSession((*i)->Socket);
	}

	g_DeleteSet.clear();

	stSESSION* pSession;

	SOCKET UserTable_SOCKET[FD_SETSIZE] = { INVALID_SOCKET };
	int iSocketCount = 0;

	//--------------------------------------------------
	//FD_SET�� FD_SETSIZE ��ŭ���� ���� �˻簡 �����ϴ�
	//�׷��Ƿ� �� ������ŭ �־ �����
	//--------------------------------------------------

	FD_SET ReadSet;
	FD_SET WriteSet;

	FD_ZERO(&ReadSet);
	FD_ZERO(&WriteSet);

	FD_SET(g_ListenSocket, &ReadSet);
	UserTable_SOCKET[iSocketCount] = g_ListenSocket;

	iSocketCount++;

	//�������� �� �������� ��� Ŭ���̾�Ʈ�� ���� SOCKET�� üũ�Ѵ�.

	for (auto SessionIter = g_SessionMap.begin(); SessionIter != g_SessionMap.end();)
	{
		pSession = SessionIter->second;

		UserTable_SOCKET[iSocketCount] = pSession->Socket;

		FD_SET(pSession->Socket, &ReadSet);
		if (pSession->SendQ.GetUseSize() > 0)
			FD_SET(pSession->Socket, &WriteSet);

		iSocketCount++;

		//�̸� �غ���, ������ �ȹް��մϴ�.
		++SessionIter;

		//select�ִ�ġ�� ����, ������� ���̺� ������ selectȣ�� �� ����
		if (FD_SETSIZE <= iSocketCount)
		{
			netSelectSocket(UserTable_SOCKET, &ReadSet, &WriteSet);

			FD_ZERO(&ReadSet);
			FD_ZERO(&WriteSet);

			memset(UserTable_SOCKET, INVALID_SOCKET, sizeof(SOCKET) * FD_SETSIZE);

			//���� ���������̱� ������ ������ ������ ���ϰ� �ɸ��� ��� ������ ó���� ��û ��������.
			//�׷� ������ 64���� selectó���� �� �� �Ź� acceptó���� ���ֵ��� �Ѵ�.

			FD_SET(g_ListenSocket, &ReadSet);
			UserTable_SOCKET[0] = g_ListenSocket;

			iSocketCount = 1;
		}
	}

	//1. 64���� �������� �ʾ��� ��� : �������� + �Ϲݼ��� �˻��ϱ� ����
	//2. 64���� �������� ��� : �������ϸ� �˻��ϱ� ����
	if (iSocketCount > 0)
		netSelectSocket(UserTable_SOCKET, &ReadSet, &WriteSet);

	//g_netIOProcessTime += timeGetTime() - startTime;
}

//���ϵ��� select�� ȣ���մϴ�.
void netSelectSocket(SOCKET* pTableSocket, FD_SET* pReadSet, FD_SET* pWriteSet)
{
	timeval time;
	//���� ������ �� Ŭ���̾�Ʈ�� �����Ҽ����ֱ⶧���� �� bool������ �ʿ���
	//�� ������ WriteSet�� ReadSet�� ���ÿ� �ؾ��Ұ�� �ʿ�
	bool bProcFlag;

	//������ ������ ����
	int iResult;

	time.tv_sec = 0;
	time.tv_usec = 0;

	//64�� �̻��� ��Ĺ���� �� �Լ��� ���� �� ȣ���ϸ鼭 ������ Ȯ�� �ؾ� �ϹǷ�
	//timeout �� 0���� �ؾ��Ѵ�. �׷��� ������ ���� ���ϵ� �˻� Ÿ�̹��� ���� �ʾ�����.

	iResult = select(NULL, pReadSet, pWriteSet, NULL, &time);

	if (iResult > 0)
	{
		for (int iCnt = 0; iCnt < FD_SETSIZE; iCnt++)
		{
			bProcFlag = true;
			if (pTableSocket[iCnt] == INVALID_SOCKET)
				break;

			//Writeüũ
			if (FD_ISSET(pTableSocket[iCnt], pWriteSet))
			{
				--iResult;
				bProcFlag = netProcSend(pTableSocket[iCnt]);
			}
			//Readüũ
			if (FD_ISSET(pTableSocket[iCnt], pReadSet))
			{
				--iResult;
				//ProcSend�κп��� ����(���Ӳ���...��)��Ȳ����
				//�̹� �ش� Ŭ���̾�Ʈ�� �������Ḧ �� ��찡 �ֱ⶧����
				//bProcFlag �� Ȯ�� �� Recv����

				if (bProcFlag)
				{
					//Listen������ ���� ó��
					if (pTableSocket[iCnt] == g_ListenSocket)
						netProcAccept();
					else if (pTableSocket[iCnt] != g_ListenSocket)
						netProcRecv(pTableSocket[iCnt]);
				}
			}
		}
	}
	else if (iResult == SOCKET_ERROR)
		//Error(L"select() error"); <- �̰� ��������
		wprintf(L"select() error\n");
}

void netProcRecv(SOCKET sock)
{
	stSESSION* pSession;
	int iBuffSize;
	int enqueueSize;
	int iResult;

	//������ ���� ������ ã�´�.
	pSession = FindSession(sock);

	//���� ã�����ϴ� ������ ���ٸ�
	if (pSession == NULL)
		return;

	//���������� ���� �޽��� Ÿ��
	//pSession->dwLastRecvTime = timeGetTime();

	if (pSession->RecvQ.GetFreeSize() == 0)
	{
		DisconnectSession(sock);
		return;
	}

	iBuffSize = pSession->RecvQ.GetDirectEnqueueSize();

	//�ޱ� �۾�
	iResult = recv(sock, pSession->RecvQ.GetRearBufferPtr(), iBuffSize, NULL);

	//�������� || ������ ��� Ŭ�����
	if (iResult == SOCKET_ERROR || iResult == 0)
	{
		g_DeleteSet.insert(pSession);
		return;
	}

	//���� ���� �����Ͱ� �ִٸ�
	if (iResult > 0)
	{
		pSession->RecvQ.MoveRear(iResult);

		while (1)
		{
			iResult = CompleteRecvPacket(pSession);

			if (iResult == 1)//���̻� ó���� ��Ŷ�� ����
				break;

			if (iResult == -1)
			{
				//_LOG(dfLOG_LEVEL_ERROR, L"PRError SessionID:%d ", pSession->dwSessionID);
				return;
			}
		}
	}
}

bool netProcSend(SOCKET sock)
{
	int retVal;
	int directDequeueSize;
	stSESSION* sessionPtr = FindSession(sock);

	directDequeueSize = sessionPtr->SendQ.GetDirectDequeueSize();

	if (directDequeueSize != sessionPtr->SendQ.GetUseSize())//send�ι��ؾߵ�
	{
		// �̰� �ϳ�
		retVal = send(sock, sessionPtr->SendQ.GetFrontBufferPtr(), directDequeueSize, NULL);

		if (retVal == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				sessionPtr->SendQ.MoveFront(retVal);
				return true;
			}
			else
			{
				g_DeleteSet.insert(sessionPtr);
				return false;
			}
		}

		sessionPtr->SendQ.MoveFront(retVal);

		retVal = send(sock, sessionPtr->SendQ.GetFrontBufferPtr(), sessionPtr->SendQ.GetUseSize(), NULL);
		if (retVal == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				sessionPtr->SendQ.MoveFront(retVal);
				return true;
			}
			else
			{
				g_DeleteSet.insert(sessionPtr);
				return false;
			}
		}
		sessionPtr->SendQ.MoveFront(retVal);
		return true;
	}
	else//send �� �� �� ��
	{
		retVal = send(sock, sessionPtr->SendQ.GetFrontBufferPtr(), directDequeueSize, NULL);

		if (retVal == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				sessionPtr->SendQ.MoveFront(retVal);
				return true;
			}
			else
			{
				g_DeleteSet.insert(sessionPtr);
				return false;
			}
		}

		sessionPtr->SendQ.MoveFront(retVal);
		return true;
	}
}

bool netProcAccept()
{
	SOCKADDR_IN addr;
	int addrSize = sizeof(addr);
	SOCKET clientSock = accept(g_ListenSocket, (SOCKADDR*)&addr, &addrSize);

	if (clientSock == INVALID_SOCKET)
		return false;

	//���ǰ�ü �Ҵ�
	stSESSION* newSession = new stSESSION;
	g_ClientID++;
	newSession->Socket = clientSock;
	newSession->dwSessionID = g_ClientID;
	g_SessionMap.insert(make_pair(clientSock, newSession));

	//ĳ���Ͱ�ü �Ҵ�, 16������
	stCHARACTER* newCharacter = new stCHARACTER;
	newCharacter->pSession = newSession;
	newCharacter->dwSessionID = g_ClientID;
	newCharacter->byDirection = dfACTION_MOVE_LL;
	newCharacter->byMoveDirection = dfACTION_MOVE_LL;
	newCharacter->chHP = 100;
	newCharacter->shX = rand() % 6400;
	newCharacter->shY = rand() % 6400;
	GetSector(newCharacter->shX, newCharacter->shY, &newCharacter->CurSector);
	GetSector(newCharacter->shX, newCharacter->shY, &newCharacter->OldSector);
	newCharacter->dwAction = dfACTION_STAND;
	g_CharacterMap.insert(make_pair(g_ClientID, newCharacter));

	//���ε��� Ŭ���̾�Ʈ�� �´� ���Ϳ� �ִ� �۾�
	g_Sector[newCharacter->CurSector.iY][newCharacter->CurSector.iX].push_back(newCharacter);

	CSerializationBuffer pPacket;
	mpChreateMyCharacter(&pPacket, newCharacter->dwSessionID, newCharacter->byDirection, newCharacter->shX, newCharacter->shY, newCharacter->chHP);
	SendPacketUnicast(newSession, &pPacket);

	//���������� ���� ������ �������ִ� �����鿡�� �Ѹ��� �κ�
	mpCreateOtherCharacter(&pPacket, newCharacter->dwSessionID, newCharacter->byDirection, newCharacter->shX, newCharacter->shY, newCharacter->chHP);
	SendPacketAround(newSession, &pPacket);

	//���������� ������ ������ �����ڸ� �Ѹ��� �κ�
	stSECTOR_AROUND sectorAround;
	GetSectorAround(newCharacter->CurSector.iX, newCharacter->CurSector.iY, &sectorAround);

	for (int i = 0; i < sectorAround.iCount; i++)
	{
		for (auto j = g_Sector[sectorAround.Around[i].iY][sectorAround.Around[i].iX].begin(); j != g_Sector[sectorAround.Around[i].iY][sectorAround.Around[i].iX].end(); ++j)
		{
			if (*j == newCharacter)
				continue;

			mpCreateOtherCharacter(&pPacket, (*j)->dwSessionID, (*j)->byDirection, (*j)->shX, (*j)->shY, (*j)->chHP);
			SendPacketUnicast(newSession, &pPacket);
		}
	}

	return true;
}

int CompleteRecvPacket(stSESSION* pSession)
{
	stPACKET_HEADER stPacketHeader;
	int iRecvQsize;

	iRecvQsize = pSession->RecvQ.GetUseSize();

	//���� ������ �˻��ؾ��Ѵ�. �׷��� ��Ŷ��� ũ�� �̻��� ��쿡��!
	if (sizeof(stPACKET_HEADER) > iRecvQsize)
		return 1;

	pSession->RecvQ.Peek((char*)&stPacketHeader, sizeof(stPACKET_HEADER));

	if (stPacketHeader.byCode != dfPACKET_CODE)
		return -1;

	if (stPacketHeader.bySize + sizeof(stPacketHeader) > iRecvQsize)
		return 1;

	//������� ������ ����� �����Ͱ� ���� ������..
	pSession->RecvQ.MoveFront(sizeof(stPacketHeader));

	CSerializationBuffer packet;

	if (!pSession->RecvQ.Dequeue(packet.GetBufferPtr(), stPacketHeader.bySize))
		return -1;

	packet.MoveWritePos(stPacketHeader.bySize);

	//��Ŷ�� �������� �з�
	if (!PacketProc(pSession, stPacketHeader.byType, &packet))
		return -1;

	return 0;
}

bool PacketProc(stSESSION* pSession, BYTE byPacketType, CSerializationBuffer* pPacket)
{
	switch (byPacketType)
	{
	case dfPACKET_CS_MOVE_START:
		netPacketProcMoveStart(pSession, pPacket);
		break;
	case dfPACKET_CS_MOVE_STOP:
		netPacketProcMoveStop(pSession, pPacket);
		break;
	case dfPACKET_CS_ATTACK1:
		netPacketProcAttack1(pSession, pPacket);
		break;
	case dfPACKET_CS_ATTACK2:
		netPacketProcAttack2(pSession, pPacket);
		break;
	case dfPACKET_CS_ATTACK3:
		netPacketProcAttack3(pSession, pPacket);
		break;
	case dfPACKET_CS_ECHO:
		netPacketProcEcho(pSession, pPacket);
		break;
	default:
		g_DeleteSet.insert(pSession);
		return false;
	}
	return true;
}

bool netPacketProcMoveStart(stSESSION* pSession, CSerializationBuffer* pPacket)
{
	BYTE byDirection;
	short shX, shY;

	*pPacket >> byDirection;
	*pPacket >> shX;
	*pPacket >> shY;

	//_LOG(dfLOG_LEVEL_DEBUG, L"# MOVESTART # SessionID : %d / Direction : %d / X : %d / Y : %d",
	//	pSession->dwSessionID, byDirection, shX, shY);

	//printf("# MOVESTART # SessionID : %d / Direction : %d / X : %d / Y : %d\n",
		//pSession->dwSessionID, byDirection, shX, shY);

	stCHARACTER* pCharacter = FindCharacter(pSession->dwSessionID);

	if (pCharacter == NULL)
	{
		_LOG(dfLOG_LEVEL_ERROR, L"# MOVESTART > SessionID:%d Character Not Found!",
			pSession->dwSessionID);
		return false;
	}

	//������ ��ġ�� ���� ��Ŷ�� ��ġ���� �ʹ� ū ���̰� ���ٸ� ��ũ ��Ŷ�� ������ ��ǥ ����.

	if (abs(pCharacter->shX - shX) > dfERROR_RANGE || abs(pCharacter->shY - shY) > dfERROR_RANGE)
	{
		// �̰�
		wprintf(L"[ ERROR_LOG ] SYNC >> Session ID: %d MoveDirection: %d / ServerX:%d / ServerY:%d / ClientX: %d / ClientY: %d \n", pSession->dwSessionID, pCharacter->byMoveDirection, pCharacter->shX, pCharacter->shY, shX, shY);
		mpSync(pPacket, pCharacter->dwSessionID, pCharacter->shX, pCharacter->shY);
		SendPacketAround(pCharacter->pSession, pPacket, true);

		shX = pCharacter->shX;
		shY = pCharacter->shY;
	}

	pCharacter->dwAction = byDirection;

	//�ܼ� ����ǥ�ÿ� byDirection�� �� 8����� MoveDirection�� ����.
	pCharacter->byMoveDirection = byDirection;

	switch (byDirection)
	{
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
		pCharacter->byDirection = dfPACKET_MOVE_DIR_RR;
		break;
	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LD:
	case dfPACKET_MOVE_DIR_LU:
		pCharacter->byDirection = dfPACKET_MOVE_DIR_LL;
		break;
	}
	pCharacter->shX = shX;
	pCharacter->shY = shY;

	//������ �ϸ鼭 ��ǥ�� �ణ ������ ��� ���͸� ������Ʈ��
	if (SectorUpdateCharacter(pCharacter))
	{
		//���Ͱ� ����� ��� Ŭ�󿡰� ���� ������ ������.
		CharacterSectorUpdatePacket(pCharacter);
	}

	mpMoveStart(pPacket, pSession->dwSessionID, pCharacter->byMoveDirection, pCharacter->shX, pCharacter->shY);

	//���� �������� ����ڿ��� ��� ��Ŷ�� �Ѹ���.
	SendPacketAround(pSession, pPacket);

	return true;
}

bool netPacketProcMoveStop(stSESSION* pSession, CSerializationBuffer* pPacket)
{
	//printf("movestop\n");
	BYTE byDirection;
	short shX, shY;

	*pPacket >> byDirection;
	*pPacket >> shX;
	*pPacket >> shY;

	//printf("move stop %d %d\n", shX, shY);

	stCHARACTER* pCharacter = FindCharacter(pSession->dwSessionID);

	if (pCharacter == NULL)
	{
		printf("netPacketProcMoveStop error\n");
		return false;
	}

	pCharacter->dwAction = dfACTION_STAND;
	//pCharacter->byMoveDirection = byDirection;

	if (abs(pCharacter->shX - shX) > dfERROR_RANGE || abs(pCharacter->shY - shY) > dfERROR_RANGE)
	{
		//printf("�����ũ!!\n");
		mpSync(pPacket, pCharacter->dwSessionID, pCharacter->shX, pCharacter->shY);
		SendPacketAround(pCharacter->pSession, pPacket, true);

		shX = pCharacter->shX;
		shY = pCharacter->shY;
	}

	switch (byDirection)
	{
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
		pCharacter->byDirection = dfPACKET_MOVE_DIR_RR;
		break;
	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LD:
	case dfPACKET_MOVE_DIR_LU:
		pCharacter->byDirection = dfPACKET_MOVE_DIR_LL;
		break;
	default:
		DisconnectSession(pSession->Socket);
		break;
	}

	mpMoveStop(pPacket, pSession->dwSessionID, pCharacter->byDirection, shX, shY);

	SendPacketAround(pSession, pPacket);

	return true;
}

bool netPacketProcAttack1(stSESSION* pSession, CSerializationBuffer* pPacket)
{
	printf("����\n");
	BYTE byDirection;
	short X, Y;

	*pPacket >> byDirection;
	*pPacket >> X;
	*pPacket >> Y;

	stCHARACTER* pCharacter = FindCharacter(pSession->dwSessionID);

	if (pCharacter == NULL)
	{
		return false;
	}

	pCharacter->dwAction = dfACTION_ATTACK1;

	mpAttack1(pPacket, pSession->dwSessionID, pCharacter->byDirection, X, Y);

	SendPacketAround(pSession, pPacket);

	return true;
}

bool netPacketProcAttack2(stSESSION* pSession, CSerializationBuffer* pPacket)
{
	BYTE byDirection;
	short X, Y;

	*pPacket >> byDirection;
	*pPacket >> X;
	*pPacket >> Y;

	stCHARACTER* pCharacter = FindCharacter(pSession->dwSessionID);

	if (pCharacter == NULL)
	{
		return false;
	}

	pCharacter->dwAction = dfACTION_ATTACK1;

	mpAttack2(pPacket, pSession->dwSessionID, pCharacter->byDirection, X, Y);

	SendPacketAround(pSession, pPacket);

	return true;
}

bool netPacketProcAttack3(stSESSION* pSession, CSerializationBuffer* pPacket)
{
	BYTE byDirection;
	short X, Y;

	*pPacket >> byDirection;
	*pPacket >> X;
	*pPacket >> Y;

	stCHARACTER* pCharacter = FindCharacter(pSession->dwSessionID);

	if (pCharacter == NULL)
	{
		return false;
	}

	pCharacter->dwAction = dfACTION_ATTACK1;

	mpAttack3(pPacket, pSession->dwSessionID, pCharacter->byDirection, X, Y);

	SendPacketAround(pSession, pPacket);

	return true;
}

bool netPacketProcEcho(stSESSION* pSession, CSerializationBuffer* pPacket)
{
	stPACKET_HEADER header;

	header.byCode = dfPACKET_CODE;
	header.bySize = 4;
	header.byType = dfPACKET_SC_ECHO;

	DWORD time;

	*pPacket >> time;

	pPacket->Clear();
	pPacket->PutData((char*)&header, sizeof(header));
	*pPacket << time;

	SendPacketUnicast(pSession, pPacket);
	return true;
}

//�� �Լ� �ȿ��� ���ͱ��� ���� ����մϴ�.
void SendPacketAround(stSESSION* pSession, CSerializationBuffer* pPacket, bool sendToMe)
{
	stCHARACTER* charPtr = g_CharacterMap.find(pSession->dwSessionID)->second;
	stSECTOR_AROUND sectorAround;

	//�� �Լ��� ����ϸ� pSession�� Ŭ���̾�Ʈ �ֺ��� ���͸� ���ϰԵ˴ϴ�.
	//�ֺ��� ���Ͱ� ��� �ִ���, ������ ��ġ�� �����
	GetSectorAround(charPtr->CurSector.iX, charPtr->CurSector.iY, &sectorAround);

	for (int i = 0; i < sectorAround.iCount; i++)
		SendPacketOneSector(sectorAround.Around[i].iX, sectorAround.Around[i].iY, pPacket, pSession);
}

void SendPacketOneSector(int iSectorX, int iSectorY, CSerializationBuffer* pPacket, stSESSION* pExceptSession)
{
	for (auto i = g_Sector[iSectorY][iSectorX].begin(); i != g_Sector[iSectorY][iSectorX].end(); ++i)
	{
		if (pExceptSession == (*i)->pSession)
			continue;

		SendPacketUnicast((*i)->pSession, pPacket);
	}
}

void SendPacketUnicast(stSESSION* pSession, CSerializationBuffer* pPacket)
{
	pSession->SendQ.Enqueue(pPacket->GetBufferPtr(), pPacket->GetUseSize());
}

void SendPacketBroadcast(stSESSION* pSession, CSerializationBuffer* pPacket)
{
	//pSession�� ���ӻ��� ����
	for (auto i = g_CharacterMap.begin(); i != g_CharacterMap.end(); ++i)
		(*i).second->pSession->SendQ.Enqueue((char*)pPacket, pPacket->GetUseSize());
}

void mpChreateMyCharacter(CSerializationBuffer* pPacket, DWORD id, BYTE direction, short X, short Y, char hp)
{
	stPACKET_HEADER stPacketHeader;
	stPacketHeader.byCode = dfPACKET_CODE;
	stPacketHeader.bySize = 10;
	stPacketHeader.byType = dfPACKET_SC_CREATE_MY_CHARACTER;

	pPacket->Clear();
	pPacket->PutData((char*)&stPacketHeader, sizeof(stPacketHeader));
	*pPacket << id;
	*pPacket << direction;
	*pPacket << X;
	*pPacket << Y;
	*pPacket << hp;
}

void mpCreateOtherCharacter(CSerializationBuffer* pPacket, DWORD id, BYTE direction, short X, short Y, char hp)
{
	stPACKET_HEADER stPacketHeader;
	stPacketHeader.byCode = dfPACKET_CODE;
	stPacketHeader.bySize = 10;
	stPacketHeader.byType = dfPACKET_SC_CREATE_OTHER_CHARACTER;

	pPacket->Clear();
	pPacket->PutData((char*)&stPacketHeader, sizeof(stPacketHeader));
	*pPacket << id;
	*pPacket << direction;
	*pPacket << X;
	*pPacket << Y;
	*pPacket << hp;
}

void mpSync(CSerializationBuffer* pPacket, DWORD id, short X, short Y)
{
	//printf("MP SYNC !!!! ~~~ \n");

	stPACKET_HEADER stPacketHeader;
	stPacketHeader.byCode = dfPACKET_CODE;
	stPacketHeader.bySize = 8;
	stPacketHeader.byType = dfPACKET_SC_SYNC;

	pPacket->Clear();
	pPacket->PutData((char*)&stPacketHeader, sizeof(stPacketHeader));
	*pPacket << id;
	*pPacket << X;
	*pPacket << Y;
}

// �̰� ����  �� �ν���d
void mpMoveStart(CSerializationBuffer* pPacket, DWORD id, BYTE byDir, short X, short Y)
{
	stPACKET_HEADER stPacketHeader;
	stPacketHeader.byCode = dfPACKET_CODE;
	stPacketHeader.bySize = 9;
	stPacketHeader.byType = dfPACKET_SC_MOVE_START;

	pPacket->Clear();
	pPacket->PutData((char*)&stPacketHeader, sizeof(stPacketHeader));
	*pPacket << id;
	*pPacket << byDir;
	*pPacket << X;
	*pPacket << Y;
}

void mpMoveStop(CSerializationBuffer* pPacket, DWORD id, BYTE byDir, short X, short Y)
{
	stPACKET_HEADER stPacketHeader;
	stPacketHeader.byCode = dfPACKET_CODE;
	stPacketHeader.bySize = 9;
	stPacketHeader.byType = dfPACKET_SC_MOVE_STOP;

	pPacket->Clear();
	pPacket->PutData((char*)&stPacketHeader, sizeof(stPacketHeader));
	*pPacket << id;
	*pPacket << byDir;
	*pPacket << X;
	*pPacket << Y;
}

void mpAttack1(CSerializationBuffer* pPacket, DWORD id, BYTE byDir, short X, short Y)
{
	stPACKET_HEADER stPacketHeader;
	stPacketHeader.byCode = dfPACKET_CODE;
	stPacketHeader.bySize = 9;
	stPacketHeader.byType = dfPACKET_SC_ATTACK1;

	pPacket->Clear();
	pPacket->PutData((char*)&stPacketHeader, sizeof(stPacketHeader));
	*pPacket << id;
	*pPacket << byDir;
	*pPacket << X;
	*pPacket << Y;
}

void mpAttack2(CSerializationBuffer* pPacket, DWORD id, BYTE byDir, short X, short Y)
{
	stPACKET_HEADER stPacketHeader;
	stPacketHeader.byCode = dfPACKET_CODE;
	stPacketHeader.bySize = 9;
	stPacketHeader.byType = dfPACKET_SC_ATTACK2;

	pPacket->Clear();
	pPacket->PutData((char*)&stPacketHeader, sizeof(stPacketHeader));
	*pPacket << id;
	*pPacket << byDir;
	*pPacket << X;
	*pPacket << Y;
}

void mpAttack3(CSerializationBuffer* pPacket, DWORD id, BYTE byDir, short X, short Y)
{
	stPACKET_HEADER stPacketHeader;
	stPacketHeader.byCode = dfPACKET_CODE;
	stPacketHeader.bySize = 9;
	stPacketHeader.byType = dfPACKET_SC_ATTACK3;

	pPacket->Clear();
	pPacket->PutData((char*)&stPacketHeader, sizeof(stPacketHeader));
	*pPacket << id;
	*pPacket << byDir;
	*pPacket << X;
	*pPacket << Y;
}

void mpDeleteCharacter(CSerializationBuffer* pPacket, DWORD id)
{
	stPACKET_HEADER stPacketHeader;
	stPacketHeader.byCode = dfPACKET_CODE;
	stPacketHeader.bySize = 4;
	stPacketHeader.byType = dfPACKET_SC_DELETE_CHARACTER;

	pPacket->Clear();
	pPacket->PutData((char*)&stPacketHeader, sizeof(stPacketHeader));
	*pPacket << id;
}

void mpDamage(CSerializationBuffer* pPacket, DWORD attackID, DWORD damageID, char damageHP)
{
	stPACKET_HEADER stPacketHeader;
	stPacketHeader.byCode = dfPACKET_CODE;
	stPacketHeader.bySize = 9;
	stPacketHeader.byType = dfPACKET_SC_DAMAGE;

	pPacket->Clear();
	pPacket->PutData((char*)&stPacketHeader, sizeof(stPacketHeader));
	*pPacket << attackID;
	*pPacket << damageID;
	*pPacket << damageHP;
}

//Ư�� ���� ��ǥ ���� �ֺ� ����� ���� ���,, �� �Լ��� GetUpdateSectorAround���� ���δ�.
void GetSectorAround(int iSectorX, int iSectorY, stSECTOR_AROUND* pSectorAround)
{
	//�̰� ���ִ� ������ ���� ���� �ִ� ���Ϳ��� ��������� �� ó�� �����ϴ� ������ ��ǥ�� ��� ����.
	iSectorX--;
	iSectorY--;

	pSectorAround->iCount = 0;

	for (int iCntY = 0; iCntY < 3; iCntY++)
	{
		//�� ���͸����� ������ 256
		//�� X,Y�� ���Ͱ����� 25��, �����ڸ��� ������� ���� ����� ���͵��� ���ϱ�����.
		if (iSectorY + iCntY < 0 || iSectorY + iCntY >= dfSECTOR_MAX_Y)
			continue;

		for (int iCntX = 0; iCntX < 3; iCntX++)
		{
			if (iSectorX + iCntX < 0 || iSectorX + iCntX >= dfSECTOR_MAX_X)
				continue;

			pSectorAround->Around[pSectorAround->iCount].iX = iSectorX + iCntX;
			pSectorAround->Around[pSectorAround->iCount].iY = iSectorY + iCntY;
			//iCount�� �ڽ��� ����ǿ� �ִ� ������ ����
			++pSectorAround->iCount;
		}
	}
}

//���Ϳ��� ���͸� �̵��Ͽ��� �� ���� ����ǿ��� ���� ����, ���� �߰��� ������ ���� ���ϴ� �Լ�
//�� �Լ��� �ܵ����δ� ������ �ʰ� CharacterSectorUpdatePacket�ȿ��� ȣ���Ѵ�
void GetUpdateSectorAround(stCHARACTER* pCharacter, stSECTOR_AROUND* pRemoveSector, stSECTOR_AROUND* pAddSector)
{
	int iCntOld, iCntCur;
	bool bFind;
	stSECTOR_AROUND oldSectorAround, curSectorAround;

	oldSectorAround.iCount = 0;
	curSectorAround.iCount = 0;

	pRemoveSector->iCount = 0;
	pAddSector->iCount = 0;

	GetSectorAround(pCharacter->OldSector.iX, pCharacter->OldSector.iY, &oldSectorAround);
	GetSectorAround(pCharacter->CurSector.iX, pCharacter->CurSector.iY, &curSectorAround);

	//���� ���� ���� ��, �ű� ���Ϳ��� ���� ������ ã�Ƽ� RemoveSector�� ����
	//���� ���Ϳ����� ����Ǽ��͵��� ������ŭ �ݺ���
	for (iCntOld = 0; iCntOld < oldSectorAround.iCount; iCntOld++)
	{
		bFind = false;
		//���� �ű� ���Ϳ����� ����Ǽ����� ������ŭ �ݺ���
		for (iCntCur = 0; iCntCur < curSectorAround.iCount; iCntCur++)
		{
			if (oldSectorAround.Around[iCntOld].iX == curSectorAround.Around[iCntCur].iX &&
				oldSectorAround.Around[iCntOld].iY == curSectorAround.Around[iCntCur].iY)
			{
				bFind = true;
				break;
			}
		}
		//bFind�� false��� �űԼ��Ϳ��� ���� ������
		if (bFind == false)
		{
			//�Ϸ� ������ ���� �������� ���͸� �ǹ��մϴ�.
			pRemoveSector->Around[pRemoveSector->iCount] = oldSectorAround.Around[iCntOld];
			//�������� ���Ͱ��� + 1 ������Ű��
			pRemoveSector->iCount++;
		}
	}

	//���ʹ� �ݴ�� �������Ϳ��� �������� �űԼ��Ϳ��� �߰��� ���͸� ��󳻴��۾�
	for (iCntCur = 0; iCntCur < curSectorAround.iCount; iCntCur++)
	{
		bFind = false;
		for (iCntOld = 0; iCntOld < oldSectorAround.iCount; iCntOld++)
		{
			if (oldSectorAround.Around[iCntOld].iX == curSectorAround.Around[iCntCur].iX &&
				oldSectorAround.Around[iCntOld].iY == curSectorAround.Around[iCntCur].iY)
			{
				bFind = true;
				break;
			}
		}
		if (bFind == false)
		{
			//�űԼ��Ϳ� ���� �߰��Ǵ� ���ʹ� �Ϸ� �־��ش�.
			pAddSector->Around[pAddSector->iCount] = curSectorAround.Around[iCntCur];
			pAddSector->iCount++;
		}
	}
}

//ĳ������ ��ǥ�� ����� ��� �� �Լ��� ȣ���Ͽ� ���͸� �������� ������ �Ǵ��Ѵ�.
//���� ���Ͱ� �̵��� �־��ٸ� true�� �����Ѵ�. ���͵� �����ؾߵ�.
bool SectorUpdateCharacter(stCHARACTER* pCharacter)
{
	stSECTOR_POS newSectionPos;

	//��� �̷��� lnlineó���ؾߵǴµ�..
	// �̺κ��� �ٸ��ŵ�� ?
	// �� �Լ��� ĳ������ ���� ��ǥ��, ��� ���Ϳ� ���c�Ǿ��ִ��� �˶��
	// �� �¾ƿ� ���� �ֱ��ѵ� ���� ���� ���ϴ� ũ�Ⱑ 50 * 50 �� ������ ũ��ϱ� �ϴ� �ٲ㺼���� �Ȱ��� ?
	// �������̶� �� 25 25 �� �߰ŵ��? 2525? ������ ����?
	// �ϴ� ���Լ��� �̵� �غ���
	GetSector(pCharacter->shX, pCharacter->shY, &newSectionPos);


	// ���� ���Ϳ� ���� ����� ���Ͱ� �ٸ��ٸ� ��������
	if (pCharacter->CurSector.iX != newSectionPos.iX || pCharacter->CurSector.iY != newSectionPos.iY)
	{
		// ���� ���͸� oldSector�� ������
		pCharacter->OldSector.iX = pCharacter->CurSector.iX;
		pCharacter->OldSector.iY = pCharacter->CurSector.iY;

		//���⼭ CurSector�� �ٲ��ְ�����
		pCharacter->CurSector.iX = newSectionPos.iX;
		pCharacter->CurSector.iY = newSectionPos.iY;

		g_Sector[pCharacter->OldSector.iY][pCharacter->OldSector.iX].remove(pCharacter);
		g_Sector[pCharacter->CurSector.iY][pCharacter->CurSector.iX].push_back(pCharacter);
		return true;
	}
	return false;
}

//�� �Լ��� ���Ϳ� ��ȭ�� ������ ��쿡�� ����ȴ�.
bool CharacterSectorUpdatePacket(stCHARACTER* pCharacter)
{
	//1.���� ���Ϳ��� ������ �κп� ���� ĳ���� ���� �޽���
	//2.�̵��ϴ� ĳ���Ϳ��� ���� ���Ϳ��� ���ܵ� ������ ĳ���͵� ���� ��Ű�� �޽���
	//3.���� �߰��� ���Ϳ� ���� ĳ���� ���� �޽��� & �̵� �޽���
	//4.�̵��ϴ� ĳ���Ϳ��� ���� �����ϴ� ������ ĳ���͵� ���� �޽���
	stSECTOR_AROUND RemoveSector, AddSector;
	stCHARACTER* pExistCharacter;

	list<stCHARACTER*>* pSectorList;
	list<stCHARACTER*>::iterator listItor;

	CSerializationBuffer packet1;
	CSerializationBuffer packet2;

	int iCnt;

	//�� �Լ��� �����ϸ�, RemoveSector�� AddSector�� ��������, �߰������� ������ ä�����Եȴ�.
	GetUpdateSectorAround(pCharacter, &RemoveSector, &AddSector);

	//RemoveSector�� ĳ���� ���� ��Ŷ �����
	mpDeleteCharacter(&packet1, pCharacter->dwSessionID);

	// ���� ���°Ű���?
	// ���� ���� 2���� �߾��
	// ��ø��� �ѹ� �ڼ��� ���Կ�
	// �ٵ� �ϳ��� �ص� ������������
	//RemoveSector�� ĳ���� ���� ��Ŷ ������
	// ���� ���� ���ڸ�ŭ �ݺ�
	// ����ѵ���?? �غκ�
	for (iCnt = 0; iCnt < RemoveSector.iCount; iCnt++)
	{
		//�ش� ������ �ε����� ��ȸ�ϸ鼭 �� ���ͳ��� �ִ� �÷��̾�鿡�� ���� ĳ���� ������Ű�� �޽��� ����
		SendPacketOneSector(RemoveSector.Around[iCnt].iX, RemoveSector.Around[iCnt].iY, &packet1, NULL);

		pSectorList = &g_Sector[RemoveSector.Around[iCnt].iY][RemoveSector.Around[iCnt].iX];

		for (auto listItor = pSectorList->begin(); listItor != pSectorList->end(); ++listItor)
		{
			if (*listItor == pCharacter)
				continue;

			mpDeleteCharacter(&packet1, (*listItor)->dwSessionID);
			//�� �ڽ����� ����
			SendPacketUnicast(pCharacter->pSession, &packet1);
		}
	}

	// �� ���ģ �κ��� ����־�� ?
	// �̰� ������? �̰Ŵ� �׳� ���������� �ݺ��� �ϳ��� �ذᰡ���Ѱ͵��̿�������, �����ִ°͵� �� �ݺ��� �ȿ��ְ� �ּ��Ѱ�
	//�ٸ� ���ο� ���ͷ� �� ĳ���Ϳ��� ������ ���� ���Ϳ� �����ߴ� ĳ���͵� �����ϴ� ��Ŷ ������
	//for (iCnt = 0; iCnt < RemoveSector.iCount; iCnt++)
	//{
	//	pSectorList = &g_Sector[RemoveSector.Around[iCnt].iY][RemoveSector.Around[iCnt].iX];

	//	for (auto listItor = pSectorList->begin(); listItor != pSectorList->end(); ++listItor)
	//	{
	//		if (*listItor == pCharacter)
	//			continue;

	//		mpDeleteCharacter(&packet1, (*listItor)->dwSessionID);
	//		//�� �ڽ����� ����
	//		SendPacketUnicast(pCharacter->pSession, &packet1);
	//	}
	//}

	mpCreateOtherCharacter(&packet1, pCharacter->dwSessionID, pCharacter->byDirection, pCharacter->shX, pCharacter->shY, pCharacter->chHP);
	mpMoveStart(&packet2, pCharacter->dwSessionID, pCharacter->byMoveDirection, pCharacter->shX, pCharacter->shY);
	// ���⼭ �ñ��Ѱ�, �ʵ� mpMoveStart�� ���� ����?
	//AddSector�� ��ȸ�ϸ鼭 �� �ڽ��� ������ �޽����� ����
	// �׳� ���� ������ 
	for (iCnt = 0; iCnt < AddSector.iCount; iCnt++)
	{
		SendPacketOneSector(AddSector.Around[iCnt].iX, AddSector.Around[iCnt].iY, &packet1, pCharacter->pSession);
		SendPacketOneSector(AddSector.Around[iCnt].iX, AddSector.Around[iCnt].iY, &packet2, pCharacter->pSession);

		pSectorList = &g_Sector[AddSector.Around[iCnt].iY][AddSector.Around[iCnt].iX];

		//�̰� ���� �ش缽�Ϳ� ���ԵǾ��մ� �����ڵ� ���Ƽ� �ǵ���ִ°���, �ڽ��� �ƴϰ� Ÿ���� ���
		// ���� �ٸ� ĳ���� ���� �κ����� ? ������ �� �ڽ��� �ٸ� ĳ���� �����ϴ°�
		for (auto listItor = pSectorList->begin(); listItor != pSectorList->end(); ++listItor)
		{
			if (*listItor != pCharacter)
			{
				mpCreateOtherCharacter(&packet1, (*listItor)->dwSessionID, (*listItor)->byDirection,
					(*listItor)->shX, (*listItor)->shY, (*listItor)->chHP);

				SendPacketUnicast(pCharacter->pSession, &packet1);
				//�� AddSector�� ĳ���Ͱ� �Ȱ� �־��ٸ� �̵� ��Ŷ�� ���� ����
				// �ƹ� �̻����� ����������ä�����;�������ä����������� �����Ϸ���;����;�̳���;�Ӥ�����
				// ���� �� �������� �ٲ�߰ڴµ��� ����
				// �� moveStart�ҋ� action�� �� ���� ��������?
				// �� ������ �����ϳ�... 
				switch ((*listItor)->dwAction)
				{
					//printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>  %d\n", (*listItor)->dwAction);
				case dfACTION_MOVE_LL:
				case dfACTION_MOVE_LU:
				case dfACTION_MOVE_UU:
				case dfACTION_MOVE_RU:
				case dfACTION_MOVE_RR:
				case dfACTION_MOVE_RD:
				case dfACTION_MOVE_DD:
				case dfACTION_MOVE_LD:
					mpMoveStart(&packet1, (*listItor)->dwSessionID, (*listItor)->byMoveDirection,
						(*listItor)->shX, (*listItor)->shY);
					SendPacketUnicast(pCharacter->pSession, &packet1);
					break;
				}
			}
		}
	}
	//for (iCnt = 0; iCnt < AddSector.iCount; iCnt++)
	//{
	//	pSectorList = &g_Sector[AddSector.Around[iCnt].iY][AddSector.Around[iCnt].iX];

	//	for (auto listItor = pSectorList->begin(); listItor != pSectorList->end(); ++listItor)
	//	{
	//		if (*listItor != pCharacter)
	//		{
	//			mpCreateOtherCharacter(&packet1, (*listItor)->dwSessionID, (*listItor)->byDirection,
	//				(*listItor)->shX, (*listItor)->shY, (*listItor)->chHP);

	//			SendPacketUnicast(pCharacter->pSession, &packet1);
	//			//�� AddSector�� ĳ���Ͱ� �Ȱ� �־��ٸ� �̵� ��Ŷ�� ���� ����
	//			switch ((*listItor)->dwAction)
	//			{
	//			//printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>  %d\n", (*listItor)->dwAction);
	//			case dfACTION_MOVE_LL:
	//			case dfACTION_MOVE_LU:
	//			case dfACTION_MOVE_UU:
	//			case dfACTION_MOVE_RU:
	//			case dfACTION_MOVE_RR:
	//			case dfACTION_MOVE_RD:
	//			case dfACTION_MOVE_DD:
	//			case dfACTION_MOVE_LD:
	//				mpMoveStart(&packet1, (*listItor)->dwSessionID, (*listItor)->byMoveDirection,
	//					(*listItor)->shX, (*listItor)->shY);
	//				SendPacketUnicast(pCharacter->pSession, &packet1);
	//				break;
	//			}
	//		}
	//	}
	//}

	return true;
}

bool CharacterMoveCheck(short X, short Y)
{
	if (X >= dfRANGE_MOVE_LEFT && X < dfRANGE_MOVE_RIGHT
		&& Y >= dfRANGE_MOVE_TOP && Y < dfRANGE_MOVE_BOTTOM)
		return true;
	else
		return false;
}

void GetSector(short X, short Y, stSECTOR_POS* sectorPtr)
{
	sectorPtr->iX = X / dfSECTER_SIZE;
	sectorPtr->iY = Y / dfSECTER_SIZE;
}

void SectorAddCharacter(stCHARACTER* pCharacter)
{
	stSECTOR_POS sectorPos;
	GetSector(pCharacter->shX, pCharacter->shY, &sectorPos);
	g_Sector[sectorPos.iY][sectorPos.iX].push_back(pCharacter);
}

void SectorRemoveCharacter(stCHARACTER* pCharacter)
{
	stSECTOR_POS sectorPos;
	GetSector(pCharacter->shX, pCharacter->shY, &sectorPos);
	g_Sector[sectorPos.iY][sectorPos.iX].remove(pCharacter);
}
