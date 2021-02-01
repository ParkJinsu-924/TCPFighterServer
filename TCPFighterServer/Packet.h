#pragma once
#ifndef __PACKET_DEFINE__
#define __PACKET_DEFINE__

#define dfTCP_PORT	20000
#define dfPACKET_CODE 0x89
//-----------------------------------------------------------------
// 화면 이동 범위.
//-----------------------------------------------------------------
#define dfRANGE_MOVE_TOP 0
#define dfRANGE_MOVE_LEFT 0
#define dfRANGE_MOVE_RIGHT 6400
#define dfRANGE_MOVE_BOTTOM 6400

//---------------------------------------------------------------
// 공격범위.
//---------------------------------------------------------------
#define dfATTACK1_RANGE_X 80
#define dfATTACK2_RANGE_X 90
#define dfATTACK3_RANGE_X 100
#define dfATTACK1_RANGE_Y 10
#define dfATTACK2_RANGE_Y 10
#define dfATTACK3_RANGE_Y 20


//---------------------------------------------------------------
// 공격 데미지.
//---------------------------------------------------------------
#define dfATTACK1_DAMAGE 1
#define dfATTACK2_DAMAGE 2
#define dfATTACK3_DAMAGE 3

//---------------------------------------------------------------
// 기타 동작
//---------------------------------------------------------------
#define dfACTION_MOVE_LL		0
#define dfACTION_MOVE_LU		1
#define dfACTION_MOVE_UU		2
#define dfACTION_MOVE_RU		3
#define dfACTION_MOVE_RR		4
#define dfACTION_MOVE_RD		5
#define dfACTION_MOVE_DD		6
#define dfACTION_MOVE_LD		7

#define dfACTION_PUSH			11
#define dfACTION_STAND			12

#define dfACTION_ATTACK1		8
#define dfACTION_ATTACK2		9
#define dfACTION_ATTACK3		10

#define dfSHADOW				100
#define dfGUAGE_HP				500

#define dfDIR_RIGHT				1
#define dfDIR_LEFT				2

#define dfDELAY_STAND			5
#define dfDELAY_MOVE			4
#define dfDELAY_ATTACK1			3
#define dfDELAY_ATTACK2			4
#define dfDELAY_ATTACK3			4
#define dfDELAY_EFFECT			3

//-----------------------------------------------------------------
// 캐릭터 이동 속도   // 25fps 기준 이동속도
//-----------------------------------------------------------------
#define dfSPEED_PLAYER_X 6
#define dfSPEED_PLAYER_Y 4

//-----------------------------------------------------------------
// 섹터 멕스 크기
//-----------------------------------------------------------------
#define dfSECTOR_MAX_X 50
#define dfSECTOR_MAX_Y 50 

#define dfSECTER_SIZE (dfRANGE_MOVE_RIGHT / dfSECTOR_MAX_X)

//-----------------------------------------------------------------
// 이동 오류체크 범위
//-----------------------------------------------------------------
#define dfERROR_RANGE 50

#pragma pack(1)

struct stPACKET_HEADER
{
	unsigned char byCode;       //고정값
	unsigned char bySize;		//사이즈
	unsigned char byType;		//타입
};
//---------------------------------------------------------------
// 패킷헤더.
//
//---------------------------------------------------------------
/*
	BYTE	byCode;			// 패킷코드 0x89 고정.
	BYTE	bySize;			// 패킷 사이즈.
	BYTE	byType;			// 패킷타입.
*/

struct stPACKET_SC_CREATE_MY_CHARACTER
{
	unsigned int id;//4
	unsigned char direction;//1
	unsigned short X;//2
	unsigned short Y;//2
	unsigned char hp;//1
};
//CBaseObject & CPlayerObject의 멤버변수로 존재함
#define	dfPACKET_SC_CREATE_MY_CHARACTER			0
//---------------------------------------------------------------
// 클라이언트 자신의 캐릭터 할당		Server -> Client
//
// 서버에 접속시 최초로 받게되는 패킷으로 자신이 할당받은 ID 와
// 자신의 최초 위치, HP 를 받게 된다. (처음에 한번 받게 됨)
// 
// 이 패킷을 받으면 자신의 ID,X,Y,HP 를 저장하고 캐릭터를 생성시켜야 한다.
//
//	4	-	ID
//	1	-	Direction	(LL / RR)
//	2	-	X
//	2	-	Y
//	1	-	HP
//
//---------------------------------------------------------------

struct stPACKET_SC_CREATE_OTHER_CHARACTER
{
	unsigned int id;
	unsigned char direction;
	unsigned short X;
	unsigned short Y;
	unsigned char hp;
};
//이 패킷이 들어오면 객체를 새로 리스트에 추가해주고 세팅을 해준다.
#define	dfPACKET_SC_CREATE_OTHER_CHARACTER		1
//---------------------------------------------------------------
// 다른 클라이언트의 캐릭터 생성 패킷		Server -> Client
//
// 처음 서버에 접속시 이미 접속되어 있던 캐릭터들의 정보
// 또는 게임중에 접속된 클라이언트들의 생성용 정보.
//
//
//	4	-	ID
//	1	-	Direction	(LL / RR)
//	2	-	X
//	2	-	Y
//	1	-	HP
//
//---------------------------------------------------------------

struct stPACKET_SC_DELETE_CHARACTER
{
	unsigned int id;
};
#define	dfPACKET_SC_DELETE_CHARACTER			2
//---------------------------------------------------------------
// 캐릭터 삭제 패킷						Server -> Client
//
// 캐릭터의 접속해제 또는 캐릭터가 죽었을때 전송됨.
//
//	4	-	ID
//
//---------------------------------------------------------------


struct stPACKET_CS_MOVE_START
{
	unsigned char direction;
	unsigned short X;
	unsigned short Y;
};
#define	dfPACKET_CS_MOVE_START					10
//---------------------------------------------------------------
// 캐릭터 이동시작 패킷						Client -> Server
//
// 자신의 캐릭터 이동시작시 이 패킷을 보낸다.
// 이동 중에는 본 패킷을 보내지 않으며, 키 입력이 변경되었을 경우에만
// 보내줘야 한다.
//
// (왼쪽 이동중 위로 이동 / 왼쪽 이동중 왼쪽 위로 이동... 등등)
//
//	1	-	Direction	( 방향 디파인 값 8방향 사용 )
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------
#define dfPACKET_MOVE_DIR_LL					0
#define dfPACKET_MOVE_DIR_LU					1
#define dfPACKET_MOVE_DIR_UU					2
#define dfPACKET_MOVE_DIR_RU					3
#define dfPACKET_MOVE_DIR_RR					4
#define dfPACKET_MOVE_DIR_RD					5
#define dfPACKET_MOVE_DIR_DD					6
#define dfPACKET_MOVE_DIR_LD					7



struct stPACKET_SC_MOVE_START
{
	unsigned int id;
	unsigned char direction;
	unsigned short X;
	unsigned short Y;
};
#define	dfPACKET_SC_MOVE_START					11
//---------------------------------------------------------------
// 캐릭터 이동시작 패킷						Server -> Client
//
// 다른 유저의 캐릭터 이동시 본 패킷을 받는다.
// 패킷 수신시 해당 캐릭터를 찾아 이동처리를 해주도록 한다.
// 
// 패킷 수신 시 해당 키가 계속해서 눌린것으로 생각하고
// 해당 방향으로 계속 이동을 하고 있어야만 한다.
//
//	4	-	ID
//	1	-	Direction	( 방향 디파인 값 8방향 )
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------



struct stPACKET_CS_MOVE_STOP
{
	unsigned char direction;
	unsigned short X;
	unsigned short Y;
};
#define	dfPACKET_CS_MOVE_STOP					12
//---------------------------------------------------------------
// 캐릭터 이동중지 패킷						Client -> Server
//
// 이동중 키보드 입력이 없어서 정지되었을 때, 이 패킷을 서버에 보내준다.
// 이동중 방향 전환시에는 스탑을 보내지 않는다.
//
//	1	-	Direction	( 방향 디파인 값 좌/우만 사용 )
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------

struct stPACKET_SC_MOVE_STOP
{
	unsigned int id;
	unsigned char direction;
	unsigned short X;
	unsigned short Y;
};
#define	dfPACKET_SC_MOVE_STOP					13
//---------------------------------------------------------------
// 캐릭터 이동중지 패킷						Server -> Client
//
// ID 에 해당하는 캐릭터가 이동을 멈춘것이므로 
// 캐릭터를 찾아서 방향과, 좌표를 입력해주고 멈추도록 처리한다.
//
//	4	-	ID
//	1	-	Direction	( 방향 디파인 값. 좌/우만 사용 )
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------


struct stPACKET_CS_ATTACK1
{
	unsigned char direction;
	unsigned short X;
	unsigned short Y;
};
#define	dfPACKET_CS_ATTACK1						20
//---------------------------------------------------------------
// 캐릭터 공격 패킷							Client -> Server
//
// 공격 키 입력시 본 패킷을 서버에게 보낸다.
// 충돌 및 데미지에 대한 결과는 서버에서 알려 줄 것이다.
//
// 공격 동작 시작시 한번만 서버에게 보내줘야 한다.
//
//	1	-	Direction	( 방향 디파인 값. 좌/우만 사용 )
//	2	-	X
//	2	-	Y	
//
//---------------------------------------------------------------

struct stPACKET_SC_ATTACK1
{
	unsigned int id;
	unsigned char direction;
	unsigned short X;
	unsigned short Y;
};
#define	dfPACKET_SC_ATTACK1						21
//---------------------------------------------------------------
// 캐릭터 공격 패킷							Server -> Client
//
// 패킷 수신시 해당 캐릭터를 찾아서 공격1번 동작으로 액션을 취해준다.
// 방향이 다를 경우에는 해당 방향으로 바꾼 후 해준다.
//
//	4	-	ID
//	1	-	Direction	( 방향 디파인 값. 좌/우만 사용 )
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------


struct stPACKET_CS_ATTACK2
{
	unsigned char direction;
	unsigned short X;
	unsigned short Y;
};
#define	dfPACKET_CS_ATTACK2						22
//---------------------------------------------------------------
// 캐릭터 공격 패킷							Client -> Server
//
// 공격 키 입력시 본 패킷을 서버에게 보낸다.
// 충돌 및 데미지에 대한 결과는 서버에서 알려 줄 것이다.
//
// 공격 동작 시작시 한번만 서버에게 보내줘야 한다.
//
//	1	-	Direction	( 방향 디파인 값. 좌/우만 사용 )
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------

struct stPACKET_SC_ATTACK2
{
	unsigned int id;
	unsigned char direction;
	unsigned short X;
	unsigned short Y;
};
#define	dfPACKET_SC_ATTACK2						23
//---------------------------------------------------------------
// 캐릭터 공격 패킷							Server -> Client
//
// 패킷 수신시 해당 캐릭터를 찾아서 공격2번 동작으로 액션을 취해준다.
// 방향이 다를 경우에는 해당 방향으로 바꾼 후 해준다.
//
//	4	-	ID
//	1	-	Direction	( 방향 디파인 값. 좌/우만 사용 )
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------

struct stPACKET_CS_ATTACK3
{
	unsigned char direction;
	unsigned short X;
	unsigned short Y;
};
#define	dfPACKET_CS_ATTACK3						24
//---------------------------------------------------------------
// 캐릭터 공격 패킷							Client -> Server
//
// 공격 키 입력시 본 패킷을 서버에게 보낸다.
// 충돌 및 데미지에 대한 결과는 서버에서 알려 줄 것이다.
//
// 공격 동작 시작시 한번만 서버에게 보내줘야 한다.
//
//	1	-	Direction	( 방향 디파인 값. 좌/우만 사용 )
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------

struct stPACKET_SC_ATTACK3
{
	unsigned int id;
	unsigned char direction;
	unsigned short X;
	unsigned short Y;
};
#define	dfPACKET_SC_ATTACK3						25
//---------------------------------------------------------------
// 캐릭터 공격 패킷							Server -> Client
//
// 패킷 수신시 해당 캐릭터를 찾아서 공격3번 동작으로 액션을 취해준다.
// 방향이 다를 경우에는 해당 방향으로 바꾼 후 해준다.
//
//	4	-	ID
//	1	-	Direction	( 방향 디파인 값. 좌/우만 사용 )
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------


struct stPACKET_SC_DAMAGE
{
	unsigned int attackID;
	unsigned int damageID;
	unsigned char damageHP;
};
#define	dfPACKET_SC_DAMAGE						30
//---------------------------------------------------------------
// 캐릭터 데미지 패킷							Server -> Client
//
// 공격에 맞은 캐릭터의 정보를 보냄.
//
//	4	-	AttackID	( 공격자 ID )
//	4	-	DamageID	( 피해자 ID )
//	1	-	DamageHP	( 피해자 HP )
//
//---------------------------------------------------------------




struct stPACKET_CS_SYNC
{
	unsigned short X;
	unsigned short Y;
};
// 사용안함...
#define	dfPACKET_CS_SYNC						250
//---------------------------------------------------------------
// 동기화를 위한 패킷					Client -> Server
//
//
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------

//=============================================================================================================타일MMO를 위한 패킷
struct stPACKET_SC_SYNC
{
	unsigned int id;
	unsigned short X;
	unsigned short Y;
};
#define	dfPACKET_SC_SYNC						251
//---------------------------------------------------------------
// 동기화를 위한 패킷					Server -> Client
//
// 서버로부터 동기화 패킷을 받으면 해당 캐릭터를 찾아서
// 캐릭터 좌표를 보정해준다.
//
//	4	-	ID
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------



#define	dfPACKET_CS_ECHO						252
//---------------------------------------------------------------
// Echo 용 패킷					Client -> Server
//
//	4	-	Time
//
//---------------------------------------------------------------

#define	dfPACKET_SC_ECHO						253
//---------------------------------------------------------------
// Echo 응답 패킷				Server -> Client
//
//	4	-	Time
//
//---------------------------------------------------------------



#endif

#pragma pack()