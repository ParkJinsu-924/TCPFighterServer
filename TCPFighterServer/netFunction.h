#pragma once
#include <WinSock2.h>
#include "Packet.h"
#define dfLOG_LEVEL_DEBUG	0
#define dfLOG_LEVEL_WARNING 1
#define dfLOG_LEVEL_ERROR	2

void Log(WCHAR* szString, int iLogLevel);
extern int g_ClientID;
extern bool g_bShutdown;
extern int g_iLogLevel;
extern WCHAR g_szLogBuff[1024];
extern SOCKET g_ListenSocket;
extern map<SOCKET, stSESSION*> g_SessionMap;
extern map<DWORD, stCHARACTER*> g_CharacterMap;
extern list<stCHARACTER*> g_Sector[dfSECTOR_MAX_Y][dfSECTOR_MAX_X];
extern set<stSESSION*> g_DeleteSet;

#define _LOG(LogLevel, fmt, ...)\
do{\
	if (g_iLogLevel <= LogLevel)\
	{\
		wsprintf(g_szLogBuff, fmt, ##__VA_ARGS__);\
		Log(g_szLogBuff, g_iLogLevel);\
	}\
}while(0)\

//socket 으로 세션 찾기
stSESSION* FindSession(SOCKET sock);

//id로 캐릭터 찾기
stCHARACTER* FindCharacter(DWORD id);

//새로운 새션을 생성, 등록한다.
stSESSION* CreateSession(SOCKET sock);

//해당 세선 종료처리
void DisconnectSession(SOCKET sock);

//서버 메인 네트워크 처리 함수
void netIOProcess();

//select모델체크함수
void netSelectSocket(SOCKET* pTableSocket, FD_SET* pReadSet, FD_SET* pWriteSet);

//Recv처리
void netProcRecv(SOCKET sock);

//Send처리
bool netProcSend(SOCKET sock);

//사용자 접속 이벤트 처리
bool netProcAccept();

//패킷이 완료되었는지 확인 후 완료 되었다면 패킷을 처리한다.
int CompleteRecvPacket(stSESSION* pSession);

//패킷 타입에 따른 처리 함수 호출
bool PacketProc(stSESSION* pSession, BYTE byPacketType, CSerializationBuffer* pPacket);

bool netPacketProcMoveStart(stSESSION* pSession, CSerializationBuffer* pPacket);

bool netPacketProcMoveStop(stSESSION* pSession, CSerializationBuffer* pPacket);

bool netPacketProcAttack1(stSESSION* pSession, CSerializationBuffer* pPacket);

bool netPacketProcAttack2(stSESSION* pSession, CSerializationBuffer* pPacket);

bool netPacketProcAttack3(stSESSION* pSession, CSerializationBuffer* pPacket);

bool netPacketProcEcho(stSESSION * pSession, CSerializationBuffer * pPacket);
//================MAKE PACKET
void mpChreateMyCharacter(CSerializationBuffer* pPacket, DWORD id, BYTE direction, short X, short Y, char hp);

void mpCreateOtherCharacter(CSerializationBuffer* pPacket, DWORD id, BYTE direction, short X, short Y, char hp);

void mpDeleteCharacter(CSerializationBuffer* pPacket, DWORD id);

void mpDamage(CSerializationBuffer* pPacket, DWORD attackID, DWORD damageID, char damageHP);

void mpMoveStart(CSerializationBuffer* pPacket, DWORD id, BYTE byDir, short X, short Y);

void mpMoveStop(CSerializationBuffer* pPacket, DWORD id, BYTE byDir, short X, short Y);

void mpAttack1(CSerializationBuffer* pPacket, DWORD id, BYTE byDir, short X, short Y);

void mpAttack2(CSerializationBuffer* pPacket, DWORD id, BYTE byDir, short X, short Y);

void mpAttack3(CSerializationBuffer* pPacket, DWORD id, BYTE byDir, short X, short Y);

void mpSync(CSerializationBuffer* pPacket, DWORD id, short X, short Y);

//================섹터단위 패킷전송함수
void SendPacketAround(stSESSION* pSession, CSerializationBuffer* pPacket, bool sendToMe = false); //대상 패킷에 전송

void SendPacketOneSector(int iSectorX, int iSectorY, CSerializationBuffer* pPacket, stSESSION* pExceptSession); //

void SendPacketUnicast(stSESSION* pSession, CSerializationBuffer* pPacket);

//================걍 전체 클라이언트 패킷전송 [ 많이 쓸 일은 없음 ]
void SendPacketBroadcast(stSESSION* pSession, CSerializationBuffer* pPacket);

//================섹터관련함수
void GetSectorAround(int iSectorX, int iSectorY, stSECTOR_AROUND* pSectorAround);

void GetUpdateSectorAround(stCHARACTER* pCharacter, stSECTOR_AROUND* pRemoveSector, stSECTOR_AROUND* pAddSector);

bool SectorUpdateCharacter(stCHARACTER* pCharacter);

bool CharacterSectorUpdatePacket(stCHARACTER* pCharacter);

void GetSector(short X, short Y, stSECTOR_POS* sectorPtr);

void SectorAddCharacter(stCHARACTER* pCharacter);

void SectorRemoveCharacter(stCHARACTER* pCharacter);

//================캐릭터 이동 가능 범위 확인
bool CharacterMoveCheck(short X, short Y);