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

//socket ���� ���� ã��
stSESSION* FindSession(SOCKET sock);

//id�� ĳ���� ã��
stCHARACTER* FindCharacter(DWORD id);

//���ο� ������ ����, ����Ѵ�.
stSESSION* CreateSession(SOCKET sock);

//�ش� ���� ����ó��
void DisconnectSession(SOCKET sock);

//���� ���� ��Ʈ��ũ ó�� �Լ�
void netIOProcess();

//select��üũ�Լ�
void netSelectSocket(SOCKET* pTableSocket, FD_SET* pReadSet, FD_SET* pWriteSet);

//Recvó��
void netProcRecv(SOCKET sock);

//Sendó��
bool netProcSend(SOCKET sock);

//����� ���� �̺�Ʈ ó��
bool netProcAccept();

//��Ŷ�� �Ϸ�Ǿ����� Ȯ�� �� �Ϸ� �Ǿ��ٸ� ��Ŷ�� ó���Ѵ�.
int CompleteRecvPacket(stSESSION* pSession);

//��Ŷ Ÿ�Կ� ���� ó�� �Լ� ȣ��
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

//================���ʹ��� ��Ŷ�����Լ�
void SendPacketAround(stSESSION* pSession, CSerializationBuffer* pPacket, bool sendToMe = false); //��� ��Ŷ�� ����

void SendPacketOneSector(int iSectorX, int iSectorY, CSerializationBuffer* pPacket, stSESSION* pExceptSession); //

void SendPacketUnicast(stSESSION* pSession, CSerializationBuffer* pPacket);

//================�� ��ü Ŭ���̾�Ʈ ��Ŷ���� [ ���� �� ���� ���� ]
void SendPacketBroadcast(stSESSION* pSession, CSerializationBuffer* pPacket);

//================���Ͱ����Լ�
void GetSectorAround(int iSectorX, int iSectorY, stSECTOR_AROUND* pSectorAround);

void GetUpdateSectorAround(stCHARACTER* pCharacter, stSECTOR_AROUND* pRemoveSector, stSECTOR_AROUND* pAddSector);

bool SectorUpdateCharacter(stCHARACTER* pCharacter);

bool CharacterSectorUpdatePacket(stCHARACTER* pCharacter);

void GetSector(short X, short Y, stSECTOR_POS* sectorPtr);

void SectorAddCharacter(stCHARACTER* pCharacter);

void SectorRemoveCharacter(stCHARACTER* pCharacter);

//================ĳ���� �̵� ���� ���� Ȯ��
bool CharacterMoveCheck(short X, short Y);