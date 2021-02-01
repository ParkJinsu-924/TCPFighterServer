#include "Include.h"
#include "netFunction.h"

#define TARGET_FRAME 25
#define TARGET_MS	40

//extern DWORD g_netIOProcessCallCount;
//extern DWORD g_netIOProcessTime;

int g_ClientID;
bool g_bShutdown = false;
SOCKET g_ListenSocket;
map<SOCKET, stSESSION*> g_SessionMap;
map<DWORD, stCHARACTER*> g_CharacterMap;
list<stCHARACTER*> g_Sector[dfSECTOR_MAX_Y][dfSECTOR_MAX_X];//각 섹터마다 그 섹터의 접속자들의 목록을 가지고있음
set<stSESSION*> g_DeleteSet;
int g_iLogLevel;
WCHAR g_szLogBuff[1024];

void ServerControl();

void Update();

void NetStartUp();

bool UpdateSkip();

int main()
{
	timeBeginPeriod(1); //타이머 해상도 높이기

	//LoadData();			//설정 및 게임데이터, DB, 데이터 로딩
	NetStartUp();		//네트워크 초기화, 리슨소켓 생성 및 listen

	//만약 중단플래그가 true 라면 중단
	while (!g_bShutdown)//서버의 메인 루프, 전역의 g_Shutdown값에 의해 종료결정
	{
		netIOProcess();//네트워크 송수신 처리

		//업데이트는 게임의 로직(월드, 이벤트, 몬스터, 캐릭터 ... 등등)
		//실제 게임에서 돌아가야 하는 로직을 처리한다.

		Update();		//게임 로직 업데이트

		ServerControl();//키보드 입력을 통해서 서버를 제어할 경우 사용
		//Monitor();		//모니터링 정보를 표시, 저장, 전송하는 경우 사용
	}

	//서버의 종료 대기

	//서버는 함부로 종료하면 안된다.
	//DB에저장할데이터나기타마무리해야할일들이끝났는지확인한뒤에꺼주어야한다.

	//netCleanUp();
	timeEndPeriod(1);
	return 0;
}

void ServerControl()
{
	//키보드 컨트롤 잠금, 풀림 변수
	static bool bControlMode = false;

	//---------------------------------
	//L : LOCK | U : UNLOCK | Q : SERVER END
	//---------------------------------

	if (_kbhit())//0이 아닌값이 리턴된다면 버퍼에 대기중인것이 있다는 뜻
	{
		WCHAR ControlKey = _getwch();

		//키보드 제어 허용
		if (L'u' == ControlKey || L'U' == ControlKey)
		{
			bControlMode = true;

			wprintf(L"Control Mode : Press Q - Quit\n");
			wprintf(L"Control Mode : Press L - Key Lock\n");
		}

		//키보드 제어 잠금
		if ((L'l' == ControlKey || L'L' == ControlKey) && bControlMode == true)
		{
			wprintf(L"Control Lock.. Press U - Control Unlock\n");
			bControlMode = false;
		}

		//키보드 제어 풀림 상태에서 특정 기능
		if ((L'q' == ControlKey || L'Q' == ControlKey) && bControlMode)
		{
			g_bShutdown = true;
		}

		//기타 컨트롤 기능 추가...
	}
}

void NetStartUp()
{
	WSADATA wsa;

	WSAStartup(MAKEWORD(2, 2), &wsa);

	g_ListenSocket = socket(AF_INET, SOCK_STREAM, NULL);

	SOCKADDR_IN listenSockAddr;
	listenSockAddr.sin_family = AF_INET;
	listenSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	listenSockAddr.sin_port = htons(dfTCP_PORT);

	bind(g_ListenSocket, (SOCKADDR*)&listenSockAddr, sizeof(listenSockAddr));

	listen(g_ListenSocket, SOMAXCONN);

	u_long on = 1;

	ioctlsocket(g_ListenSocket, FIONBIO, &on);
}

void Update()
{
	if (!UpdateSkip())
		return;

	//static int oldTime = 0;
	//int curTime = timeGetTime();
	//if (curTime - oldTime < 40)//25프레임
	//	return;
	//oldTime = curTime;
	// 이ㄱ거 그냥 안햇거든 아 그거말고 그 데드레커닝할때 클라 입력시간없으면 
	// 아그리고 데드레커닝? 그거 시간 안ㅅ느다고했지?
	// 어떤 시간이용?
	stCHARACTER* pCharacter = nullptr;

	//캐릭터 정보를 찾아서 처리한다.
	for (auto itor = g_CharacterMap.begin(); itor != g_CharacterMap.end();)
	{
		pCharacter = itor->second;
		itor++;
		//현재 동작에 따른 처리
		switch (pCharacter->dwAction)
		{
		case dfACTION_MOVE_LL:
			if (CharacterMoveCheck(pCharacter->shX - dfSPEED_PLAYER_X, pCharacter->shY))
				pCharacter->shX -= dfSPEED_PLAYER_X;
			break;
		case dfACTION_MOVE_LU:
			if (CharacterMoveCheck(pCharacter->shX - dfSPEED_PLAYER_X, pCharacter->shY - dfSPEED_PLAYER_Y))
			{
				pCharacter->shX -= dfSPEED_PLAYER_X;
				pCharacter->shY -= dfSPEED_PLAYER_Y;
			}
			break;
		case dfACTION_MOVE_UU:
			if (CharacterMoveCheck(pCharacter->shX, pCharacter->shY - dfSPEED_PLAYER_Y))
				pCharacter->shY -= dfSPEED_PLAYER_Y;
			break;
		case dfACTION_MOVE_RU:
			if (CharacterMoveCheck(pCharacter->shX + dfSPEED_PLAYER_X, pCharacter->shY - dfSPEED_PLAYER_Y))
			{
				pCharacter->shX += dfSPEED_PLAYER_X;
				pCharacter->shY -= dfSPEED_PLAYER_Y;
			}
			break;
		case dfACTION_MOVE_RR:
			if (CharacterMoveCheck(pCharacter->shX + dfSPEED_PLAYER_X, pCharacter->shY))
				pCharacter->shX += dfSPEED_PLAYER_X;
			break;
		case dfACTION_MOVE_RD:
			if (CharacterMoveCheck(pCharacter->shX + dfSPEED_PLAYER_X, pCharacter->shY + dfSPEED_PLAYER_Y))
			{
				pCharacter->shX += dfSPEED_PLAYER_X;
				pCharacter->shY += dfSPEED_PLAYER_Y;
			}
			break;
		case dfACTION_MOVE_DD:
			if (CharacterMoveCheck(pCharacter->shX, pCharacter->shY + dfSPEED_PLAYER_Y))
				pCharacter->shY += dfSPEED_PLAYER_Y;
			break;
		case dfACTION_MOVE_LD:
			if (CharacterMoveCheck(pCharacter->shX - dfSPEED_PLAYER_X, pCharacter->shY + dfSPEED_PLAYER_Y))
			{
				pCharacter->shX -= dfSPEED_PLAYER_X;
				pCharacter->shY += dfSPEED_PLAYER_Y;
			}
			break;
		}

		//조건문안에 있는 범위가 이동관련 define집합을 의미함, 즉 이 조건문을 탄다면 이동하는 경우를 의미함
		if (pCharacter->dwAction >= dfACTION_MOVE_LL && pCharacter->dwAction <= dfACTION_MOVE_LD)
		{
			//이동인 경우 섹터 업데이트 함
			//섹터간 이동이 있었을 경우 true 없을경우 false
			if (SectorUpdateCharacter(pCharacter))
			{
				//섹터가 변경된 경우에는 클라에게 관련 정보를 보낸다.
				CharacterSectorUpdatePacket(pCharacter);
			}
		}
	}
}

bool UpdateSkip()
{
	// Update() 호출 타이밍 확인용
	static DWORD oldTime = timeGetTime();
	DWORD       curTime = timeGetTime();

	// 복구 해야할 프레임
	static DWORD recoveryFrame = 0;

	// FPS 출력 용도
	static DWORD frameCheckOldTime = timeGetTime();

	// 기존 프레임
	static DWORD updateFPS = 0;
	// 복구한 프레임
	static DWORD recoveryUpdateFPS = 0;

	// 1초마다 FPS 출력
	if (curTime - frameCheckOldTime >= 1000)
	//완존
	{
		// 프레임은 기존 프레임 + 복구한 프레임이다.
		frameCheckOldTime = curTime;

		// updateFPS가 목표 Frame보다 낮다면 복구 해야할 프레임이 있다는 의미
		if (updateFPS < TARGET_FRAME)
		{
			// 복구 해야할 프레임에 추가
			recoveryFrame += (TARGET_FRAME - updateFPS);
		}


		// 1초 지났으므로 FPS 초기화
		updateFPS = 0;
		recoveryUpdateFPS = 0;
	}

	// TARGET_MS 넘을때 마다 한번씩 Update 호출 
	if (curTime - oldTime >= TARGET_MS)
	{
		oldTime = curTime;
		updateFPS++;
		return true;
	}

	// 복구 해야할 프레임이 있다면 바로 Update 호출
	if (recoveryFrame > 0)
	{
		// 복구해야할 프레임 1 감소
		recoveryFrame--;
		// 복구한 프레임 1 증가
		recoveryUpdateFPS++;
		return true;
	}

	return false;

}