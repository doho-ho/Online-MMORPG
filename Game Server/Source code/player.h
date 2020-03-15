#pragma once

class GAMEServer;

class player : public GameSession
{
public:
	GAMEServer *server;

	bool				authFlag;
	bool				loadDataFlag;

	char		charType;
	short		Level;
	int			maxHP;
	int			currentHP;
	bool		deathFlag;

	int xTile, yTile;

	playerStatus nowStatus;

	unsigned __int64 moveTime;
	unsigned __int64 frameTime;
	unsigned __int64 overMoveTime;

	float xPos, zPos, xDir, zDir, xForward, zForward;
	bool transMovingForward;

	sectorInfo currentSector;
	sectorInfo previousSector;

	Map *map;
	Sector *sec;

public:
	INT64 accountNo;
	char	characterObjectID[25];

	WCHAR id[dfID_MAX_LEN];
	WCHAR nickName[dfNICK_MAX_LEN];

	// 기타 필요 컨텐츠들 다음에 추가.
	ULONGLONG timeTick;
public:
	player();
	~player();

	// 로직함수
		// auth
		void proc_loginReq(Sbuf *_buf);

		// game
		void proc_createCharacter(void);
		
		void proc_move(Sbuf *_buf);
		void proc_stop(Sbuf *_buf);
		void proc_sync(void);
		void proc_attack(Sbuf *_buf);
		void proc_hit(Sbuf *_buf);
		int proc_damage(int _damage);
		int proc_Heal(int _recoveryAmount);

	// 패킷 함수
	Sbuf* packet_loginRes(BYTE _status);
	Sbuf* packet_createCharacter(short _packetType, player *_user);
	Sbuf* packet_removeCharacter(unsigned __int64 _Index);
	Sbuf* packet_moveRes(unsigned __int64 _Index, float _xForward, float _zForward, float _xDir, float _zDir);
	Sbuf* packet_stopRes(unsigned __int64 _Index, float _xForward, float _zForward, float _xPos, float _zPos);
	Sbuf* packet_syncRes(unsigned __int64 _Index, float _xForward, float _zForward, float _xPos, float _zPos);
	Sbuf *packet_attack(unsigned __int64 _Index, float _xForward, float _zForward);
	Sbuf *packet_hit(unsigned __int64 _targetIndex, float _xPos, float _zPos, int _damage, int _currentHP);
	Sbuf *packet_Recovery(unsigned __int64 _Index, int _recoveryAmount ,int _currentHP);

	// 가상함수
	void onAuth_clientJoin(void);
	void onAuth_clientLeave(void);
	void onAuth_Packet(Sbuf *_buf);

	void onGame_clientJoin(void);
	void onGame_clientLeave(void);
	void onGame_Packet(Sbuf *_buf);

	void onGame_Release(void);

};