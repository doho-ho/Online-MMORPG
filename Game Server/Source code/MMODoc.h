#pragma once

#define AUTH_ACCEPT_DELAY  100
#define AUTH_PACKET_DELAY 5000
#define AUTH_DATA_DELAY	100
#define GAME_PACKET_DEALY 5000
#define GAME_RELEASE_DEALY 1000
#define GAME_MODE_DEALY 1000
#define GAME_LOGOUT_DEALY 1000
#define DB_PACKET_DEALY 100

#define dfMOVE_SPEED (float)0.3f
#define dfMOVE_FRAME (float)20.0f

#define dfID_MAX_LEN 10
#define dfNICK_MAX_LEN 10
#define maxWSABUF 200

#define dfMAP_POS_MAX 100

#define dfPOSITON_ERROR_RANGE_X	5.0f
#define dfPOSITON_ERROR_RANGE_Y	5.0f

//---------------------------------------------
// 타일 좌표를 섹터 좌표로 변경
//
//---------------------------------------------
#define TILE_to_SECTOR_X(Tile) (Tile / dfSECTOR_TILE_WIDTH)
#define TILE_to_SECTOR_Y(Tile) (Tile / dfSECTOR_TILE_HEIGHT)

typedef ULONG64 SESSION_ID;

struct sessionLock
{
	LONG flag;
	LONG IOCP_count;
};

struct session
{
	session()
	{
		lock.IOCP_count = 0;
		lock.flag = false;

		id = 0;
		sock = INVALID_SOCKET;
		IOCP_count = &lock.IOCP_count;
		sendCount = 0;
		sendFlag = 0;
		sendPostCount = 0;
		disconnectFlag = false;
	}

	SESSION_ID id;

	SOCKET sock;

	winBuffer recvQ;
	lockFreeQueue<Sbuf*> sendQ;
	OVERLAPPED recvOver, sendOver;
	LONG *IOCP_count;
	LONG sendCount;
	LONG sendPostCount;
	LONG disconnectFlag;		// 보내고 끊기 플래그. true 면 보내고 끊어라.
	volatile LONG sendFlag;
	volatile LONG recvFlag;

	sessionLock lock;
};

#pragma pack(push, 1)
struct header
{
	BYTE code;
	WORD len;
	BYTE randCode;
	BYTE checkSum;
};

#pragma pack(pop)

