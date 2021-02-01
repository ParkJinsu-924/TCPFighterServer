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

	//삭제해야될 세션들을 전부 Disconnect해준다.
	for (auto i = g_DeleteSet.begin(); i != g_DeleteSet.end(); ++i)
	{
		DisconnectSession((*i)->Socket);
	}

	g_DeleteSet.clear();

	stSESSION* pSession;

	SOCKET UserTable_SOCKET[FD_SETSIZE] = { INVALID_SOCKET };
	int iSocketCount = 0;

	//--------------------------------------------------
	//FD_SET은 FD_SETSIZE 만큼씩만 소켓 검사가 가능하다
	//그러므로 그 개수만큼 넣어서 사용함
	//--------------------------------------------------

	FD_SET ReadSet;
	FD_SET WriteSet;

	FD_ZERO(&ReadSet);
	FD_ZERO(&WriteSet);

	FD_SET(g_ListenSocket, &ReadSet);
	UserTable_SOCKET[iSocketCount] = g_ListenSocket;

	iSocketCount++;

	//리슨소켓 및 접속중인 모든 클라이언트에 대해 SOCKET을 체크한다.

	for (auto SessionIter = g_SessionMap.begin(); SessionIter != g_SessionMap.end();)
	{
		pSession = SessionIter->second;

		UserTable_SOCKET[iSocketCount] = pSession->Socket;

		FD_SET(pSession->Socket, &ReadSet);
		if (pSession->SendQ.GetUseSize() > 0)
			FD_SET(pSession->Socket, &WriteSet);

		iSocketCount++;

		//미리 해보림, 영향을 안받게합니다.
		++SessionIter;

		//select최대치에 도달, 만들어진 테이블 정보로 select호출 후 정리
		if (FD_SETSIZE <= iSocketCount)
		{
			netSelectSocket(UserTable_SOCKET, &ReadSet, &WriteSet);

			FD_ZERO(&ReadSet);
			FD_ZERO(&WriteSet);

			memset(UserTable_SOCKET, INVALID_SOCKET, sizeof(SOCKET) * FD_SETSIZE);

			//단일 스레드방식이기 때문에 서버의 로직이 부하가 걸리는 경우 접속자 처리가 엄청 느려진다.
			//그런 이유로 64개씩 select처리를 할 때 매번 accept처리를 해주도록 한다.

			FD_SET(g_ListenSocket, &ReadSet);
			UserTable_SOCKET[0] = g_ListenSocket;

			iSocketCount = 1;
		}
	}

	//1. 64개에 도달하지 않았을 경우 : 리슨소켓 + 일반소켓 검사하기 위함
	//2. 64개에 도달했을 경우 : 리슨소켓만 검사하기 위함
	if (iSocketCount > 0)
		netSelectSocket(UserTable_SOCKET, &ReadSet, &WriteSet);

	//g_netIOProcessTime += timeGetTime() - startTime;
}

//소켓들의 select를 호출합니다.
void netSelectSocket(SOCKET* pTableSocket, FD_SET* pReadSet, FD_SET* pWriteSet)
{
	timeval time;
	//만약 에러가 난 클라이언트가 존재할수도있기때문에 이 bool변수가 필요함
	//한 소켓이 WriteSet과 ReadSet이 동시에 해야할경우 필요
	bool bProcFlag;

	//반응한 소켓의 갯수
	int iResult;

	time.tv_sec = 0;
	time.tv_usec = 0;

	//64개 이상의 소캣에선 본 함수를 여러 번 호출하면서 소켓을 확인 해야 하므로
	//timeout 은 0으로 해야한다. 그렇지 않으면 뒤쪽 소켓들 검사 타이밍이 점점 늦어진다.

	iResult = select(NULL, pReadSet, pWriteSet, NULL, &time);

	if (iResult > 0)
	{
		for (int iCnt = 0; iCnt < FD_SETSIZE; iCnt++)
		{
			bProcFlag = true;
			if (pTableSocket[iCnt] == INVALID_SOCKET)
				break;

			//Write체크
			if (FD_ISSET(pTableSocket[iCnt], pWriteSet))
			{
				--iResult;
				bProcFlag = netProcSend(pTableSocket[iCnt]);
			}
			//Read체크
			if (FD_ISSET(pTableSocket[iCnt], pReadSet))
			{
				--iResult;
				//ProcSend부분에서 에러(접속끊김...등)상황으로
				//이미 해당 클라이언트가 접속종료를 한 경우가 있기때문에
				//bProcFlag 로 확인 후 Recv진행

				if (bProcFlag)
				{
					//Listen소켓은 별도 처리
					if (pTableSocket[iCnt] == g_ListenSocket)
						netProcAccept();
					else if (pTableSocket[iCnt] != g_ListenSocket)
						netProcRecv(pTableSocket[iCnt]);
				}
			}
		}
	}
	else if (iResult == SOCKET_ERROR)
		//Error(L"select() error"); <- 이거 만들어야함
		wprintf(L"select() error\n");
}

void netProcRecv(SOCKET sock)
{
	stSESSION* pSession;
	int iBuffSize;
	int enqueueSize;
	int iResult;

	//소켓을 통해 세션을 찾는다.
	pSession = FindSession(sock);

	//만약 찾고자하는 세션이 없다면
	if (pSession == NULL)
		return;

	//마지막으로 받은 메시지 타임
	//pSession->dwLastRecvTime = timeGetTime();

	if (pSession->RecvQ.GetFreeSize() == 0)
	{
		DisconnectSession(sock);
		return;
	}

	iBuffSize = pSession->RecvQ.GetDirectEnqueueSize();

	//받기 작업
	iResult = recv(sock, pSession->RecvQ.GetRearBufferPtr(), iBuffSize, NULL);

	//정상종료 || 에러일 경우 클라끊기
	if (iResult == SOCKET_ERROR || iResult == 0)
	{
		g_DeleteSet.insert(pSession);
		return;
	}

	//만약 받을 데이터가 있다면
	if (iResult > 0)
	{
		pSession->RecvQ.MoveRear(iResult);

		while (1)
		{
			iResult = CompleteRecvPacket(pSession);

			if (iResult == 1)//더이상 처리할 패킷이 없음
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

	if (directDequeueSize != sessionPtr->SendQ.GetUseSize())//send두번해야됨
	{
		// 이거 하나
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
	else//send 한 번 만 함
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

	//새션객체 할당
	stSESSION* newSession = new stSESSION;
	g_ClientID++;
	newSession->Socket = clientSock;
	newSession->dwSessionID = g_ClientID;
	g_SessionMap.insert(make_pair(clientSock, newSession));

	//캐릭터객체 할당, 16페이지
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

	//새로들어온 클라이언트를 맞는 섹터에 넣는 작업
	g_Sector[newCharacter->CurSector.iY][newCharacter->CurSector.iX].push_back(newCharacter);

	CSerializationBuffer pPacket;
	mpChreateMyCharacter(&pPacket, newCharacter->dwSessionID, newCharacter->byDirection, newCharacter->shX, newCharacter->shY, newCharacter->chHP);
	SendPacketUnicast(newSession, &pPacket);

	//새로접속한 나를 기존에 접속해있던 유저들에게 뿌리는 부분
	mpCreateOtherCharacter(&pPacket, newCharacter->dwSessionID, newCharacter->byDirection, newCharacter->shX, newCharacter->shY, newCharacter->chHP);
	SendPacketAround(newSession, &pPacket);

	//새로접속한 나에게 기존에 접속자를 뿌리는 부분
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

	//받은 내용을 검사해야한다. 그런데 패킷헤더 크기 이상인 경우에만!
	if (sizeof(stPACKET_HEADER) > iRecvQsize)
		return 1;

	pSession->RecvQ.Peek((char*)&stPacketHeader, sizeof(stPACKET_HEADER));

	if (stPacketHeader.byCode != dfPACKET_CODE)
		return -1;

	if (stPacketHeader.bySize + sizeof(stPacketHeader) > iRecvQsize)
		return 1;

	//여기까지 왔으면 헤더와 데이터가 전부 존재함..
	pSession->RecvQ.MoveFront(sizeof(stPacketHeader));

	CSerializationBuffer packet;

	if (!pSession->RecvQ.Dequeue(packet.GetBufferPtr(), stPacketHeader.bySize))
		return -1;

	packet.MoveWritePos(stPacketHeader.bySize);

	//패킷을 종류별로 분류
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

	//서버의 위치와 받은 패킷의 위치값이 너무 큰 차이가 난다면 싱크 패킷을 보내어 좌표 보정.

	if (abs(pCharacter->shX - shX) > dfERROR_RANGE || abs(pCharacter->shY - shY) > dfERROR_RANGE)
	{
		// 이거
		wprintf(L"[ ERROR_LOG ] SYNC >> Session ID: %d MoveDirection: %d / ServerX:%d / ServerY:%d / ClientX: %d / ClientY: %d \n", pSession->dwSessionID, pCharacter->byMoveDirection, pCharacter->shX, pCharacter->shY, shX, shY);
		mpSync(pPacket, pCharacter->dwSessionID, pCharacter->shX, pCharacter->shY);
		SendPacketAround(pCharacter->pSession, pPacket, true);

		shX = pCharacter->shX;
		shY = pCharacter->shY;
	}

	pCharacter->dwAction = byDirection;

	//단순 방향표시용 byDirection과 시 8방향용 MoveDirection이 있음.
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

	//정지를 하면서 좌표가 약간 조절된 경우 섹터를 업데이트함
	if (SectorUpdateCharacter(pCharacter))
	{
		//섹터가 변경된 경우 클라에게 관련 정보를 보낸다.
		CharacterSectorUpdatePacket(pCharacter);
	}

	mpMoveStart(pPacket, pSession->dwSessionID, pCharacter->byMoveDirection, pCharacter->shX, pCharacter->shY);

	//현재 접속중인 사용자에게 모든 패킷을 뿌린다.
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
		//printf("멈춤씽크!!\n");
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
	printf("동작\n");
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

//이 함수 안에서 섹터까지 전부 계산합니다.
void SendPacketAround(stSESSION* pSession, CSerializationBuffer* pPacket, bool sendToMe)
{
	stCHARACTER* charPtr = g_CharacterMap.find(pSession->dwSessionID)->second;
	stSECTOR_AROUND sectorAround;

	//이 함수를 통과하면 pSession의 클라이언트 주변의 섹터를 구하게됩니다.
	//주변에 섹터가 몇개가 있는지, 각각의 위치가 어딘지
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
	//pSession의 쓰임새가 없다
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

// 이거 말고  프 로시저d
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

//특정 섹터 좌표 기준 주변 영향권 섹터 얻기,, 이 함수는 GetUpdateSectorAround에서 쓰인다.
void GetSectorAround(int iSectorX, int iSectorY, stSECTOR_AROUND* pSectorAround)
{
	//이걸 해주는 이유는 내가 현재 있는 섹터에서 좌측상단의 맨 처음 시작하는 섹터의 좌표를 얻기 위함.
	iSectorX--;
	iSectorY--;

	pSectorAround->iCount = 0;

	for (int iCntY = 0; iCntY < 3; iCntY++)
	{
		//각 섹터마다의 지름은 256
		//각 X,Y의 섹터갯수는 25개, 가장자리에 있을경우 맵을 벗어나는 섹터들은 안하기위함.
		if (iSectorY + iCntY < 0 || iSectorY + iCntY >= dfSECTOR_MAX_Y)
			continue;

		for (int iCntX = 0; iCntX < 3; iCntX++)
		{
			if (iSectorX + iCntX < 0 || iSectorX + iCntX >= dfSECTOR_MAX_X)
				continue;

			pSectorAround->Around[pSectorAround->iCount].iX = iSectorX + iCntX;
			pSectorAround->Around[pSectorAround->iCount].iY = iSectorY + iCntY;
			//iCount는 자신의 영향권에 있는 섹터의 갯수
			++pSectorAround->iCount;
		}
	}
}

//섹터에서 섹터를 이동하였을 때 섹터 영향권에서 빠진 섹터, 새로 추가된 섹터의 정보 구하는 함수
//이 함수는 단독으로는 쓰이지 않고 CharacterSectorUpdatePacket안에서 호출한다
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

	//이전 섹터 정보 중, 신규 섹터에는 없는 정보를 찾아서 RemoveSector에 넣음
	//이전 섹터에서의 영향권섹터들의 갯수만큼 반복문
	for (iCntOld = 0; iCntOld < oldSectorAround.iCount; iCntOld++)
	{
		bFind = false;
		//현재 옮긴 섹터에서의 영향권섹터의 갯수만큼 반복문
		for (iCntCur = 0; iCntCur < curSectorAround.iCount; iCntCur++)
		{
			if (oldSectorAround.Around[iCntOld].iX == curSectorAround.Around[iCntCur].iX &&
				oldSectorAround.Around[iCntOld].iY == curSectorAround.Around[iCntCur].iY)
			{
				bFind = true;
				break;
			}
		}
		//bFind가 false라면 신규섹터에는 없는 섹터임
		if (bFind == false)
		{
			//일로 들어오는 것은 지워지는 섹터를 의미합니다.
			pRemoveSector->Around[pRemoveSector->iCount] = oldSectorAround.Around[iCntOld];
			//지워지는 섹터갯수 + 1 증가시키기
			pRemoveSector->iCount++;
		}
	}

	//위와는 반대로 이전섹터에는 없었지만 신규섹터에서 추가된 섹터를 골라내는작업
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
			//신규섹터에 새로 추가되는 섹터는 일로 넣어준다.
			pAddSector->Around[pAddSector->iCount] = curSectorAround.Around[iCntCur];
			pAddSector->iCount++;
		}
	}
}

//캐릭터의 좌표가 변경된 경우 이 함수를 호출하여 섹터를 변경할지 말지를 판단한다.
//만약 섹터간 이동이 있었다면 true를 리턴한다. 섹터도 변경해야됨.
bool SectorUpdateCharacter(stCHARACTER* pCharacter)
{
	stSECTOR_POS newSectionPos;

	//사실 이런걸 lnline처리해야되는데..
	// 이부분이 다르거든요 ?
	// 이 함수는 캐릭터의 현재 좌표로, 어느 섹터에 포홤되어있는지 알라고
	// 네 맞아여 저도 있긴한데 섹터 영역 구하는 크기가 50 * 50 아 어차피 크기니깡 일단 바꿔볼래요 똑같이 ?
	// 찬훈형이랑 저 25 25 로 했거든요? 2525? 단위가 뭐임?
	// 일단 이함수로 이동 해보죵
	GetSector(pCharacter->shX, pCharacter->shY, &newSectionPos);


	// 기존 섹터와 새로 계산한 섹터가 다르다면 로직실행
	if (pCharacter->CurSector.iX != newSectionPos.iX || pCharacter->CurSector.iY != newSectionPos.iY)
	{
		// 현재 섹터를 oldSector로 조지고
		pCharacter->OldSector.iX = pCharacter->CurSector.iX;
		pCharacter->OldSector.iY = pCharacter->CurSector.iY;

		//여기서 CurSector을 바꿔주고있음
		pCharacter->CurSector.iX = newSectionPos.iX;
		pCharacter->CurSector.iY = newSectionPos.iY;

		g_Sector[pCharacter->OldSector.iY][pCharacter->OldSector.iX].remove(pCharacter);
		g_Sector[pCharacter->CurSector.iY][pCharacter->CurSector.iX].push_back(pCharacter);
		return true;
	}
	return false;
}

//이 함수는 섹터에 변화가 생겼을 경우에만 실행된다.
bool CharacterSectorUpdatePacket(stCHARACTER* pCharacter)
{
	//1.이전 섹터에서 없어진 부분에 대해 캐릭터 삭제 메시지
	//2.이동하는 캐릭터에게 이전 섹터에서 제외된 섹터의 캐릭터들 삭제 시키는 메시지
	//3.새로 추가된 섹터에 대해 캐릭터 생성 메시지 & 이동 메시지
	//4.이동하는 캐릭터에게 새로 진입하는 섹터의 캐릭터들 생성 메시지
	stSECTOR_AROUND RemoveSector, AddSector;
	stCHARACTER* pExistCharacter;

	list<stCHARACTER*>* pSectorList;
	list<stCHARACTER*>::iterator listItor;

	CSerializationBuffer packet1;
	CSerializationBuffer packet2;

	int iCnt;

	//이 함수를 실행하면, RemoveSector과 AddSector에 삭제섹터, 추가섹터의 정보가 채워지게된다.
	GetUpdateSectorAround(pCharacter, &RemoveSector, &AddSector);

	//RemoveSector에 캐릭터 삭제 패킷 만들기
	mpDeleteCharacter(&packet1, pCharacter->dwSessionID);

	// 문제 없는거같지?
	// 아항 저는 2개로 했어요
	// 잠시만여 한번 자세히 볼게영
	// 근데 하나로 해도 도리꺼같은데
	//RemoveSector에 캐릭터 삭제 패킷 보내기
	// 삭제 섹터 숫자만큼 반복
	// 깔꼼한데영?? 밑부분
	for (iCnt = 0; iCnt < RemoveSector.iCount; iCnt++)
	{
		//해당 섹터의 인덱스를 순회하면서 그 섹터내에 있는 플레이어들에게 빠진 캐릭터 삭제시키는 메시지 전송
		SendPacketOneSector(RemoveSector.Around[iCnt].iX, RemoveSector.Around[iCnt].iY, &packet1, NULL);

		pSectorList = &g_Sector[RemoveSector.Around[iCnt].iY][RemoveSector.Around[iCnt].iX];

		for (auto listItor = pSectorList->begin(); listItor != pSectorList->end(); ++listItor)
		{
			if (*listItor == pCharacter)
				continue;

			mpDeleteCharacter(&packet1, (*listItor)->dwSessionID);
			//나 자신한테 전송
			SendPacketUnicast(pCharacter->pSession, &packet1);
		}
	}

	// 형 블록친 부분이 어디있어요 ?
	// 이건 뭐에요? 이거는 그냥 로직적으로 반복문 하나에 해결가능한것들이여가지고, 여기있는것들 위 반복문 안에넣고 주석한거
	//다른 새로운 섹터로 간 캐릭터에게 이전에 삭제 섹터에 존재했던 캐릭터들 삭제하는 패킷 보내기
	//for (iCnt = 0; iCnt < RemoveSector.iCount; iCnt++)
	//{
	//	pSectorList = &g_Sector[RemoveSector.Around[iCnt].iY][RemoveSector.Around[iCnt].iX];

	//	for (auto listItor = pSectorList->begin(); listItor != pSectorList->end(); ++listItor)
	//	{
	//		if (*listItor == pCharacter)
	//			continue;

	//		mpDeleteCharacter(&packet1, (*listItor)->dwSessionID);
	//		//나 자신한테 전송
	//		SendPacketUnicast(pCharacter->pSession, &packet1);
	//	}
	//}

	mpCreateOtherCharacter(&packet1, pCharacter->dwSessionID, pCharacter->byDirection, pCharacter->shX, pCharacter->shY, pCharacter->chHP);
	mpMoveStart(&packet2, pCharacter->dwSessionID, pCharacter->byMoveDirection, pCharacter->shX, pCharacter->shY);
	// 여기서 궁금한게, 너도 mpMoveStart을 같이 보내?
	//AddSector를 순회하면서 나 자신을 만들라는 메시지를 보냄
	// 네네 같이 보내여 
	for (iCnt = 0; iCnt < AddSector.iCount; iCnt++)
	{
		SendPacketOneSector(AddSector.Around[iCnt].iX, AddSector.Around[iCnt].iY, &packet1, pCharacter->pSession);
		SendPacketOneSector(AddSector.Around[iCnt].iX, AddSector.Around[iCnt].iY, &packet2, pCharacter->pSession);

		pSectorList = &g_Sector[AddSector.Around[iCnt].iY][AddSector.Around[iCnt].iX];

		//이게 이제 해당섹터에 포함되어잇는 접속자들 돌아서 맨들어주는거임, 자신은 아니고 타인일 경우
		// ㅇㅎ 다른 캐릭터 생성 부분이죠 ? ㅇㅇㅇ 나 자신이 다른 캐릭터 생성하는거
		for (auto listItor = pSectorList->begin(); listItor != pSectorList->end(); ++listItor)
		{
			if (*listItor != pCharacter)
			{
				mpCreateOtherCharacter(&packet1, (*listItor)->dwSessionID, (*listItor)->byDirection,
					(*listItor)->shX, (*listItor)->shY, (*listItor)->chHP);

				SendPacketUnicast(pCharacter->pSession, &packet1);
				//새 AddSector의 캐릭터가 걷고 있었다면 이동 패킷을 만들어서 보냄
				// 아무 이상없어요 형ㅁㅇㄴ라ㅓㅐㅑㅗ;ㄻㅇ나ㅓㅇㄴㅁ리ㅏㅗ ㅁㅇ니럼닝;ㅏ럼;이나러;ㅣㅁㅇ나
				// 제가 이 로직으로 바꿔야겠는데여 ㅋㅋ
				// 그 moveStart할떄 action에 잘 적용 시켜주죠?
				// 이 다음걸 봐야하나... 
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
	//			//새 AddSector의 캐릭터가 걷고 있었다면 이동 패킷을 만들어서 보냄
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
