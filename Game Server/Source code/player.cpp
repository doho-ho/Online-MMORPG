#include "stdafx.h"

player::player()
{
	// member variables initialization.
	accountNo = 0;
	timeTick = 0;

	authFlag = false;
	authFlag = false;
	loadDataFlag = false;

}

player::~player()
{

}




void player::onAuth_clientJoin(void)
{
	authFlag = false;
	authTOgame = false;
}

void player::onAuth_clientLeave(void)
{
	authFlag = false;
	return;
}

void player::onAuth_Packet(Sbuf *_buf)
{
	WORD type;
	*_buf >> type;
	switch (type)
	{
	case game_loginUser_req:
		proc_loginReq(_buf);
		break;
	default:
		disconnect();
		break;
	}
	return;
}

void player::onGame_clientJoin(void)
{
	frameTime = GetTickCount64();
	proc_createCharacter();
}

void player::onGame_clientLeave(void)
{
	map->removeTile(this);

	if (sec->removeSector(this))
		sec->proc_userSectorUpdate(this);

	deathFlag = false;
	currentSector.xPos = previousSector.xPos = currentSector.yPos = previousSector.yPos = -1;
}

void player::onGame_Packet(Sbuf *_buf)
{
	WORD type;
	*_buf >> type;
	switch (type)
	{
	case game_move_req:
		proc_move(_buf);
		break;
	case game_stop_req:
		proc_stop(_buf);
		break;
	case game_attack_req:
		proc_attack(_buf);
		break;
	case game_hit_req:
		proc_hit(_buf);
		break;
	default:
		disconnect(); 
		break;
	}
	return;
}

void player::onGame_Release(void)
{
	if(loadDataFlag)
		server->proc_reqSaveData(this);
	server->proc_reqLogout(this);
	accountNo = -1;
	loadDataFlag = false;
	return;
}


void player::proc_loginReq(Sbuf *_buf)
{
	// unsigned __int64 sessionKey;
	// int		version;			// 클라이언트 버전

	unsigned __int64 sessionKey;
	int Version;
	*_buf >> sessionKey;
	*_buf >> Version;

	char Result;
	loginInfo* info = server->loginClient->AUTH_getSessionkey(sessionKey);
	if (info == NULL)
		Result = login_Result::Failed;
	else
	{
		Result = login_Result::Success;
		authFlag = true;
		accountNo = info->acNo;
		strcpy_s(characterObjectID, 25, info->oid);
		delete info;
	}
		
	Sbuf * buf = packet_loginRes(Result);
	if (Result == login_Result::Success)
		sendPacket(buf);
	else
		sendPacket(buf, true);
	buf->Free();
}

void player::proc_createCharacter(void)
{
	int xTile, yTile;
	map->transPosToTile(xPos, zPos, &xTile, &yTile);
	
	map->setTile(this, xTile, yTile);
	sec->addSector(this);
	sec->proc_userSectorUpdate(this);

 	Sbuf *buf = packet_createCharacter(game_newChar_res, this);
	sendPacket(buf);
	buf->Free();


}

void player:: proc_move(Sbuf *_buf)
{
	// unsigned __int64 Index;
	// float	xDir;
	// float	zDir;
	// float	xForward;
	// float	zForward;
	// float	xPos;
	// float	zPos;
	
	unsigned __int64 Index;
	float _xDir, _zDir, _xForward, _zForward, _xPos, _zPos;
	*_buf >> Index;
	*_buf >> _xDir;
	*_buf >> _zDir;
	*_buf >> _xForward;
	*_buf >> _zForward;
	*_buf >> _xPos;
	*_buf >> _zPos;

	if (clientID != Index)
		return;
	
	if (abs(xPos - _xPos) >= dfPOSITON_ERROR_RANGE_X || abs(zPos - _zPos) >= dfPOSITON_ERROR_RANGE_Y)
		proc_sync();
	else
	{
		xPos = _xPos;
		zPos = _zPos;
	}
	xDir = _xDir;
	zDir = _zDir;
	xForward = _xForward;
	zForward = _zForward;

	nowStatus = playerStatus::MOVE;

//	printf("[Index : %lld] proc_Move xDir : %f / zDir : %f\n", Index, xDir, zDir);
	Sbuf *buf = NULL;
	buf = packet_moveRes(clientID, xForward, zForward, xDir, zDir);
	sec->sendSectorAround(currentSector.xPos, currentSector.yPos, buf);
	buf->Free();
}

void player::proc_stop(Sbuf *_buf)
{
	// unsigned __int64 Index;
	// float					xForward;
	// float					zForward;
	// float					xPos;
	// float					zPos

	unsigned __int64 Index;
	float  _xForward, _zForward, _xPos, _zPos;
	*_buf >> Index;
	*_buf >> _xForward;
	*_buf >> _zForward;
	*_buf >> _xPos;
	*_buf >> _zPos;

	if (clientID != Index)
		return;
	nowStatus = playerStatus::NONE;
	if (abs(xPos - _xPos) >= dfPOSITON_ERROR_RANGE_X || abs(zPos - _zPos) >= dfPOSITON_ERROR_RANGE_Y)
		proc_sync();
	else
	{
		xPos = _xPos;
		zPos = _zPos;
	}
	xForward = _xForward;
	zForward = _zForward;

	Sbuf *buf = NULL;
	buf = packet_stopRes(clientID, xForward, zForward,xPos,zPos);
	sec->sendSectorAround(currentSector.xPos, currentSector.yPos, buf);
	buf->Free();
}

void player::proc_sync(void)
{
	Sbuf *buf = NULL;
	buf = packet_syncRes(clientID, xForward, zForward, xPos, zPos);
	sec->sendSectorAround(currentSector.xPos, currentSector.yPos, buf);
	buf->Free();
}

void player::proc_attack(Sbuf *_buf)
{
	// unsigned __int64	Index
	// float					xRotation
	// float					zRotation

	unsigned __int64 Index;
	float xRotation, zRotation;
	*_buf >> Index;
	*_buf >> xRotation;
	*_buf >> zRotation;

	if (Index != clientID)
		return;

	Sbuf *buf = packet_attack(clientID, xForward, zForward);
	sec->sendSectorAround(currentSector.xPos, currentSector.yPos, buf,this);
	buf->Free();
}

void player::proc_hit(Sbuf *_buf)
{
	// unsigned __int64 Index
	// unsigned __int64 targetIndex
	// float                    xPosition
	// float                    zPosition
	// int                      damage

	unsigned __int64 Index, targetIndex;
	float xPos, zPos;
	int Damage;
	*_buf >> Index;
	*_buf >> targetIndex;
	*_buf >> xPos;
	*_buf >> zPos;
	*_buf >> Damage;

	if (Index != clientID)
		return;

	int targetHP;
	if (!server->proc_attackVerification(this, targetIndex,Damage,&targetHP))
		return;

	Sbuf *buf = packet_hit(targetIndex, xPos, zPos, Damage, targetHP);
	sec->sendSectorAround(currentSector.xPos, currentSector.yPos, buf);
	buf->Free();
}

int player::proc_damage(int _damage)
{
	if (_damage >= currentHP) {
		currentHP = 0;
		deathFlag = true;
	}
	else
		currentHP -= _damage;

	return currentHP;
}

int player::proc_Heal(int _recoveryAmount)
{
	int HP = currentHP + _recoveryAmount;
	if (HP >= maxHP)
		HP = maxHP;
	currentHP = HP;
	return HP;
}

Sbuf* player::packet_loginRes(BYTE _status)
{
	//		WORD	Type
	//		BYTE	Status (0: 실패 / 1: 성공 / 2: 신규캐릭터 선택 모드 / 3:버전 다름.)
	//		INT64	AccountNo
	Sbuf *buf = Sbuf::Alloc();
	*buf << (WORD)game_loginUser_res;
	*buf << _status;
	*buf << clientID;

	return buf;
}

Sbuf* player::packet_createCharacter(short _packetType, player *_user)
{
	// unsigned short			Type;
	// unsigned __int64			Index;
	// unsigned char			charType;
	// WCHAR						nickName[NICKNAME_SIZE]
	// float							xPos
	// float							zPos
	// float							xForward
	// float							zForward
	//	int								Level
	// int								HP

	Sbuf *buf = Sbuf::Alloc();
	*buf << _packetType;
	*buf << clientID;
	*buf << charType;
	buf->push((char*)&nickName, sizeof(WCHAR)*dfNICK_MAX_LEN);
	*buf << xPos;
	*buf << zPos;
	*buf << xForward;
	*buf << zForward;
	*buf << Level;
	*buf << maxHP;
	*buf << currentHP;

	return buf;
}

Sbuf* player::packet_removeCharacter(unsigned __int64 _Index)
{
	// unsigned short			Type;
	// unsigned __int64		Index;
	Sbuf *buf = Sbuf::Alloc();
	*buf << (short)game_deleteUser_res;
	*buf << _Index;
	return buf;
}

Sbuf* player::packet_moveRes(unsigned __int64 _Index, float _xForward, float _zForward, float _xDir, float _zDir)
{
	// unsigned short	Type;
	// unsigned __int64	Index;
	// float					xDir;
	// float					zDir
	// float					xForward;
	// float					zForward;

	Sbuf *buf = Sbuf::Alloc();
	*buf << (short)game_move_res;
	*buf << clientID;
	*buf << xDir;
	*buf << zDir;
	*buf << xForward;
	*buf << zForward;
	return buf;
}

Sbuf* player::packet_stopRes(unsigned __int64 _Index, float _xForward, float _zForward, float _xPos, float _zPos)
{
	// unsigned short	Type;
	// unsigned __int64 Index;
	// float					xForward;
	// float					zForward;
	// float					xPos;
	// float					zPos

	Sbuf *buf = Sbuf::Alloc();
	*buf << (short)game_stop_res;
	*buf << clientID;
	*buf << xForward;
	*buf << zForward;
	*buf << xPos;
	*buf << zPos;

	return buf;
}

Sbuf* player::packet_syncRes(unsigned __int64 _Index, float _xForward, float _zForward, float _xPos, float _zPos)
{
	// unsigned short	Type;
	// unsigned __int64	Index;
	// float					xPos;
	// float					yPos;
	// float					xForward;
	// float					zForward;

	Sbuf *buf = Sbuf::Alloc();
	*buf << (short)game_sync_res;
	*buf << clientID;
	*buf << xPos;
	*buf << zPos;
	*buf << xForward;
	*buf << zForward;
	
	return buf;
}

Sbuf* player::packet_attack(unsigned __int64 _Index, float _xForward, float _zForward)
{
	// unsigned __int64 Index
	// float					xRotation
	// float                  zRotation

	Sbuf *buf = Sbuf::Alloc();
	*buf << (short)game_attack_res;
	*buf << _Index;
	*buf << _xForward;
	*buf << _zForward;
	return buf;
}

Sbuf* player::packet_hit(unsigned __int64 _targetIndex, float _xPos, float _zPos, int _damage, int _currentHP)
{
	// unsigned short			Type;
	// unsigned __int64		Index;
	Sbuf *buf = Sbuf::Alloc();
	*buf << (short)game_hit_res;
	*buf << _targetIndex;
	*buf << _xPos;
	*buf << _zPos;
	*buf << _damage;
	*buf << _currentHP;
	return buf;
}

Sbuf* player::packet_Recovery(unsigned __int64 _Index, int _recovertAmount, int _currentHP)
{
	//	unsigned short	Type
	// unsigned __int64	Index
	// int						currentHP

	Sbuf *buf = Sbuf::Alloc();
	*buf << (short)game_Recovery_res;
	*buf << _Index;
	*buf << _recovertAmount;
	*buf << _currentHP;
	return buf;
}