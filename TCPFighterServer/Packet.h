#pragma once
#ifndef __PACKET_DEFINE__
#define __PACKET_DEFINE__

#define dfTCP_PORT	20000
#define dfPACKET_CODE 0x89
//-----------------------------------------------------------------
// ȭ�� �̵� ����.
//-----------------------------------------------------------------
#define dfRANGE_MOVE_TOP 0
#define dfRANGE_MOVE_LEFT 0
#define dfRANGE_MOVE_RIGHT 6400
#define dfRANGE_MOVE_BOTTOM 6400

//---------------------------------------------------------------
// ���ݹ���.
//---------------------------------------------------------------
#define dfATTACK1_RANGE_X 80
#define dfATTACK2_RANGE_X 90
#define dfATTACK3_RANGE_X 100
#define dfATTACK1_RANGE_Y 10
#define dfATTACK2_RANGE_Y 10
#define dfATTACK3_RANGE_Y 20


//---------------------------------------------------------------
// ���� ������.
//---------------------------------------------------------------
#define dfATTACK1_DAMAGE 1
#define dfATTACK2_DAMAGE 2
#define dfATTACK3_DAMAGE 3

//---------------------------------------------------------------
// ��Ÿ ����
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
// ĳ���� �̵� �ӵ�   // 25fps ���� �̵��ӵ�
//-----------------------------------------------------------------
#define dfSPEED_PLAYER_X 6
#define dfSPEED_PLAYER_Y 4

//-----------------------------------------------------------------
// ���� �߽� ũ��
//-----------------------------------------------------------------
#define dfSECTOR_MAX_X 50
#define dfSECTOR_MAX_Y 50 

#define dfSECTER_SIZE (dfRANGE_MOVE_RIGHT / dfSECTOR_MAX_X)

//-----------------------------------------------------------------
// �̵� ����üũ ����
//-----------------------------------------------------------------
#define dfERROR_RANGE 50

#pragma pack(1)

struct stPACKET_HEADER
{
	unsigned char byCode;       //������
	unsigned char bySize;		//������
	unsigned char byType;		//Ÿ��
};
//---------------------------------------------------------------
// ��Ŷ���.
//
//---------------------------------------------------------------
/*
	BYTE	byCode;			// ��Ŷ�ڵ� 0x89 ����.
	BYTE	bySize;			// ��Ŷ ������.
	BYTE	byType;			// ��ŶŸ��.
*/

struct stPACKET_SC_CREATE_MY_CHARACTER
{
	unsigned int id;//4
	unsigned char direction;//1
	unsigned short X;//2
	unsigned short Y;//2
	unsigned char hp;//1
};
//CBaseObject & CPlayerObject�� ��������� ������
#define	dfPACKET_SC_CREATE_MY_CHARACTER			0
//---------------------------------------------------------------
// Ŭ���̾�Ʈ �ڽ��� ĳ���� �Ҵ�		Server -> Client
//
// ������ ���ӽ� ���ʷ� �ްԵǴ� ��Ŷ���� �ڽ��� �Ҵ���� ID ��
// �ڽ��� ���� ��ġ, HP �� �ް� �ȴ�. (ó���� �ѹ� �ް� ��)
// 
// �� ��Ŷ�� ������ �ڽ��� ID,X,Y,HP �� �����ϰ� ĳ���͸� �������Ѿ� �Ѵ�.
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
//�� ��Ŷ�� ������ ��ü�� ���� ����Ʈ�� �߰����ְ� ������ ���ش�.
#define	dfPACKET_SC_CREATE_OTHER_CHARACTER		1
//---------------------------------------------------------------
// �ٸ� Ŭ���̾�Ʈ�� ĳ���� ���� ��Ŷ		Server -> Client
//
// ó�� ������ ���ӽ� �̹� ���ӵǾ� �ִ� ĳ���͵��� ����
// �Ǵ� �����߿� ���ӵ� Ŭ���̾�Ʈ���� ������ ����.
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
// ĳ���� ���� ��Ŷ						Server -> Client
//
// ĳ������ �������� �Ǵ� ĳ���Ͱ� �׾����� ���۵�.
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
// ĳ���� �̵����� ��Ŷ						Client -> Server
//
// �ڽ��� ĳ���� �̵����۽� �� ��Ŷ�� ������.
// �̵� �߿��� �� ��Ŷ�� ������ ������, Ű �Է��� ����Ǿ��� ��쿡��
// ������� �Ѵ�.
//
// (���� �̵��� ���� �̵� / ���� �̵��� ���� ���� �̵�... ���)
//
//	1	-	Direction	( ���� ������ �� 8���� ��� )
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
// ĳ���� �̵����� ��Ŷ						Server -> Client
//
// �ٸ� ������ ĳ���� �̵��� �� ��Ŷ�� �޴´�.
// ��Ŷ ���Ž� �ش� ĳ���͸� ã�� �̵�ó���� ���ֵ��� �Ѵ�.
// 
// ��Ŷ ���� �� �ش� Ű�� ����ؼ� ���������� �����ϰ�
// �ش� �������� ��� �̵��� �ϰ� �־�߸� �Ѵ�.
//
//	4	-	ID
//	1	-	Direction	( ���� ������ �� 8���� )
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
// ĳ���� �̵����� ��Ŷ						Client -> Server
//
// �̵��� Ű���� �Է��� ��� �����Ǿ��� ��, �� ��Ŷ�� ������ �����ش�.
// �̵��� ���� ��ȯ�ÿ��� ��ž�� ������ �ʴ´�.
//
//	1	-	Direction	( ���� ������ �� ��/�츸 ��� )
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
// ĳ���� �̵����� ��Ŷ						Server -> Client
//
// ID �� �ش��ϴ� ĳ���Ͱ� �̵��� ������̹Ƿ� 
// ĳ���͸� ã�Ƽ� �����, ��ǥ�� �Է����ְ� ���ߵ��� ó���Ѵ�.
//
//	4	-	ID
//	1	-	Direction	( ���� ������ ��. ��/�츸 ��� )
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
// ĳ���� ���� ��Ŷ							Client -> Server
//
// ���� Ű �Է½� �� ��Ŷ�� �������� ������.
// �浹 �� �������� ���� ����� �������� �˷� �� ���̴�.
//
// ���� ���� ���۽� �ѹ��� �������� ������� �Ѵ�.
//
//	1	-	Direction	( ���� ������ ��. ��/�츸 ��� )
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
// ĳ���� ���� ��Ŷ							Server -> Client
//
// ��Ŷ ���Ž� �ش� ĳ���͸� ã�Ƽ� ����1�� �������� �׼��� �����ش�.
// ������ �ٸ� ��쿡�� �ش� �������� �ٲ� �� ���ش�.
//
//	4	-	ID
//	1	-	Direction	( ���� ������ ��. ��/�츸 ��� )
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
// ĳ���� ���� ��Ŷ							Client -> Server
//
// ���� Ű �Է½� �� ��Ŷ�� �������� ������.
// �浹 �� �������� ���� ����� �������� �˷� �� ���̴�.
//
// ���� ���� ���۽� �ѹ��� �������� ������� �Ѵ�.
//
//	1	-	Direction	( ���� ������ ��. ��/�츸 ��� )
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
// ĳ���� ���� ��Ŷ							Server -> Client
//
// ��Ŷ ���Ž� �ش� ĳ���͸� ã�Ƽ� ����2�� �������� �׼��� �����ش�.
// ������ �ٸ� ��쿡�� �ش� �������� �ٲ� �� ���ش�.
//
//	4	-	ID
//	1	-	Direction	( ���� ������ ��. ��/�츸 ��� )
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
// ĳ���� ���� ��Ŷ							Client -> Server
//
// ���� Ű �Է½� �� ��Ŷ�� �������� ������.
// �浹 �� �������� ���� ����� �������� �˷� �� ���̴�.
//
// ���� ���� ���۽� �ѹ��� �������� ������� �Ѵ�.
//
//	1	-	Direction	( ���� ������ ��. ��/�츸 ��� )
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
// ĳ���� ���� ��Ŷ							Server -> Client
//
// ��Ŷ ���Ž� �ش� ĳ���͸� ã�Ƽ� ����3�� �������� �׼��� �����ش�.
// ������ �ٸ� ��쿡�� �ش� �������� �ٲ� �� ���ش�.
//
//	4	-	ID
//	1	-	Direction	( ���� ������ ��. ��/�츸 ��� )
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
// ĳ���� ������ ��Ŷ							Server -> Client
//
// ���ݿ� ���� ĳ������ ������ ����.
//
//	4	-	AttackID	( ������ ID )
//	4	-	DamageID	( ������ ID )
//	1	-	DamageHP	( ������ HP )
//
//---------------------------------------------------------------




struct stPACKET_CS_SYNC
{
	unsigned short X;
	unsigned short Y;
};
// ������...
#define	dfPACKET_CS_SYNC						250
//---------------------------------------------------------------
// ����ȭ�� ���� ��Ŷ					Client -> Server
//
//
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------

//=============================================================================================================Ÿ��MMO�� ���� ��Ŷ
struct stPACKET_SC_SYNC
{
	unsigned int id;
	unsigned short X;
	unsigned short Y;
};
#define	dfPACKET_SC_SYNC						251
//---------------------------------------------------------------
// ����ȭ�� ���� ��Ŷ					Server -> Client
//
// �����κ��� ����ȭ ��Ŷ�� ������ �ش� ĳ���͸� ã�Ƽ�
// ĳ���� ��ǥ�� �������ش�.
//
//	4	-	ID
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------



#define	dfPACKET_CS_ECHO						252
//---------------------------------------------------------------
// Echo �� ��Ŷ					Client -> Server
//
//	4	-	Time
//
//---------------------------------------------------------------

#define	dfPACKET_SC_ECHO						253
//---------------------------------------------------------------
// Echo ���� ��Ŷ				Server -> Client
//
//	4	-	Time
//
//---------------------------------------------------------------



#endif

#pragma pack()