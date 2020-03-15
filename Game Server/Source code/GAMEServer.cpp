#include "stdafx.h"

GAMEServer::GAMEServer(const char *_configData)
{
	loadConfig(_configData);

	// 초기화
	printFlag = false;
	DBFlag = false;
	Version = 0.1;
	loginClient = new LOGINClient(_configData, this);
	configData = _configData;

	DBFrame = 0;
	loginServerConnectedFlag = false;

	map = new Map(mapConfig,mapFileSize);
	sec = new Sector(mapConfig, mapFileSize);
	
	playerArray = new player[maxUser];

	authDBQ = new lockFreeQueue<AUTH_GameData_res*>;
	DBQ = new lockFreeQueue<DBInfo*>;
	DBPool = new memoryPool<DBInfo>;

	int count = 0;
	for (count; count < maxUser; count++)
	{
		playerArray[count].map = map;
		playerArray[count].sec = sec;
		playerArray[count].server = this;
	}

	setSessionArry(playerArray, maxUser);



	Start(_configData);
	HANDLE hcp;
	hcp = (HANDLE)_beginthreadex(NULL, 0, printThread, (LPVOID)this, 0, 0);
	CloseHandle(hcp);
	hcp = (HANDLE)_beginthreadex(NULL, 0, DBThread, (LPVOID)this, 0, 0);
	CloseHandle(hcp);

	printf("START GAME SERVER\n");
}

GAMEServer::~GAMEServer()
{
	printFlag = true;
	DBFlag = true;
}

void GAMEServer::loadConfig(const char *_configData)
{
	// config 파일 읽어와서 멤버 변수에 값 저장.
	rapidjson::Document Doc;
	Doc.Parse(_configData);

	rapidjson::Value &sys = Doc["NET"];
	maxUser = sys["MAX_USER"].GetUint();
	maxUser = sys["MAX_USER"].GetUint();

	rapidjson::Value &map = Doc["MAP_CONFIG"];
	mapConfig = loadMap(map["FILE_NAME"].GetString());
}

char* GAMEServer::loadMap(const char *_fileName)
{
	FILE *fp;
	printf("Loding [%s]..\t", _fileName);
	fopen_s(&fp, _fileName, "rb");
	if (!fp)
	{
		printf("fail\n");
		printf("파일 이름 오류 [%s]\n", _fileName);
		fclose(fp);
		return NULL;
	}

	fseek(fp, 0, SEEK_END);
	mapFileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char *buffer = new char[(mapFileSize + 1)];
	size_t test = fread(buffer, mapFileSize, 1, fp);
	if (test == 0)
	{
		printf("fail\n");
		printf("파일 읽기 오류 [%s]\n", _fileName);
		delete[] buffer;
		fclose(fp);
		return NULL;
	}
	printf("%s read complete!\n",_fileName);
	buffer[mapFileSize] = '\0';
	fclose(fp);

	mapFileSize += 1;
	return buffer;

}

void GAMEServer::gameServermonitoringDataUpdate()
{
	pDBFrame = DBFrame;
	DBFrame = 0;
}

int GAMEServer::getVersion()
{
	return Version;
}

void GAMEServer::proc_loginServerConnected(bool _val)
{
	loginServerConnectedFlag = _val;
}

unsigned __stdcall GAMEServer::printThread(LPVOID data)
{
	GAMEServer *server = (GAMEServer*)data;
	LOGINClient *client = server->loginClient;

	memoryPool<Sbuf> *pool = Sbuf::pool;
	time_t timeStamp = 0;
	while (1)
	{
		if (server->printFlag)
			break;
		time(&timeStamp);
		server->monitoring();
		server->gameServermonitoringDataUpdate();
		printf("=================================================================\n");
		printf("\t\t\t*** GameServer ***\n");
		printf("\t\t\t\t     [ Login Server Connected : %s ]\n", server->loginServerConnectedFlag?"ON":"OFF");
		printf("[GAME] ========================================================\n");
		printf("   User Count : [ %03d ] \n", server->sessionCount);
		printf("   AUTH Mode : [ %d ]  /  GAME Mode : [ %d ] \n", server->pAuth, server->pGame);
		printf("   AUTH TO GAME : [ %d ]  /  LOGOUT WAIT : [ %d ] \n\n", server->pAuthToGame, server->pLogout);
		printf("[QUEUE] ========================================================\n");
		printf("   AUTH Q : [ %03d ]  / DB Q : [ %d ] \n", server->AUTHQ.getUsedSize(), server->DBQ->getUsedSize());
		printf("   AUTH DB Q : [ %03d ] \n\n", server->authDBQ->getUsedSize(), server->DBQ->getUsedSize());
		printf("[THREAD] ========================================================\n");
		printf("   AUTH THREAD : [ %d ]  / GAME THREAD : [ %d ]\n", server->pAuthFrame, server->pGameFrame);
		printf("   SEND THREAD : [ %d ]  / DB THREAD : [ %d ]\n\n", server->pSendFrame, server->pDBFrame);
		printf("[POOL] ========================================================\n");
		printf("   Sbuf Alloc : [ %d ]      /  Sbuf Used : [ %d ] \n", pool->getAllocCount(), pool->getUsedCount());
		printf("   DBPool Alloc : [ %d ]  /  DBPool Used : [ %d ] \n\n", server->DBPool->getAllocCount(), server->DBPool->getUsedCount());
		printf("[NETWORK] ==========================================================\n");
		printf("   accept Total : [ %lld ]  /  accept TPS   : [ %d ] \n", server->acceptTotal, server->pAcceptTPS);
		printf("   Recv TPS     : [ %d ]  /  Send TPS : [ %d ] \n\n", server->pRecvTPS, server->pSendTPS);

		Sleep(990);
	}
	printf("printhread closed\n");
	return 0;
}

unsigned __stdcall GAMEServer::DBThread(LPVOID _data)
{
	GAMEServer *server = (GAMEServer*)_data;
	lockFreeQueue<DBInfo*> *DBQ = server->DBQ;
	DBInfo *data = NULL;
	int DBPacketDealy = 0;
	MONGO_GameDB *gameDB = new MONGO_GameDB(server->configData);
	memoryPool<DBInfo> *DBPool = server->DBPool;
	lockFreeQueue<AUTH_GameData_res*> *authDBQ = server->authDBQ;
	AUTH_GameData_res *retval = NULL;
	while (1)
	{
		if (server->DBFlag && DBQ->getUsedSize() == 0)
			break;
		DBPacketDealy = 0;
		while (DBPacketDealy < DB_PACKET_DEALY)
		{
			DBQ->dequeue(&data);
			if (!data)
				break;

			switch (data->DBType)
			{
			case DBpacketType::USER_LOAD_DATA:
				retval = gameDB->proc_loadData((AUTH_GameData_req*)data->DBData);
				if (retval)
					authDBQ->enqueue(retval);
				break;
			case DBpacketType::USER_SAVE_DATA:
				gameDB->proc_saveDatA((GAME_SaveData*)data->DBData);
				break;
			default:
				break;
			}
			DBPool->Free(data);
			data = NULL;
		}
		server->DBFrame++;
		Sleep(20);
	}

	delete authDBQ;
	delete DBQ;
	delete DBPool;
	return 0;
}

void GAMEServer::onAuth_Update(void)
{
	proc_reqUserData();
	proc_resUserData();
}

void GAMEServer::onGame_Update(void)
{
	int count = 0;
	ULONGLONG timeVal;
	ULONGLONG nowTime;
	player *user = NULL;
	for (count; count < maxSession; count++)
	{
		if (playerArray[count].Mode == MODE_GAME)
		{
			user = &playerArray[count];
			nowTime = GetTickCount64();
			timeVal = nowTime - user->frameTime;
			user->timeTick += timeVal;
			// 이동처리
			if (user->nowStatus == MOVE)
				proc_move(user, timeVal);
			if (user->timeTick >= healTickTime)
			{
				printf("[%lld] HEAL PACKET : %lld \n",user->clientID, user->timeTick);
				user->timeTick -= healTickTime;
				proc_perHeal(user);
			}

			user->frameTime = nowTime;
		}
	}
}

player* GAMEServer::getUser(unsigned __int64 _Index)
{
	int Count = 0;
	for (Count; Count < maxSession; Count++)
	{
		if (playerArray[Count].clientID == _Index)
			return &playerArray[Count];
	}
	return NULL;
}

void GAMEServer::proc_reqUserData(void)
{
	int Count = 0;
	for (Count; Count < maxSession; Count++)
	{
		if (playerArray[Count].Mode == MODE_AUTH && playerArray[Count].authFlag == true)
		{
			if (DBFlag)	// DBFlag가 TRUE면 게임서버 종료된 것.
				return;

			DBInfo *info = DBPool->Alloc();
			info->DBType = DBpacketType::USER_LOAD_DATA;
			AUTH_GameData_req *data = new AUTH_GameData_req;
			data->Index = playerArray[Count].clientID;
			strcpy_s(data->oid, 25, playerArray[Count].characterObjectID);
			info->DBData = (LPVOID*)data;
			DBQ->enqueue(info);
			playerArray[Count].authFlag = false;
		}
	}
}

void GAMEServer::proc_resUserData(void)
{
	int Count = 0;
	AUTH_GameData_res *data = NULL;
	for (Count; Count < AUTH_DATA_DELAY; Count++)
	{
		authDBQ->dequeue(&data);
		if (!data)
			continue;

		player *user = getUser(data->Index);
		if (user)
		{
			user->xPos = data->xPos;
			user->zPos = data->zPos;
			user->xForward = data->xForward;
			user->zForward = data->zForward;
			user->Level = data->Level;
			user->maxHP = data->maxHP;
			user->currentHP = data->currentHP;
			wcscpy_s(user->nickName, 10, data->nickName);
			user->authTOgame = true;
			user->loadDataFlag = true;
		}
		delete data;
	}
}

void GAMEServer::proc_reqSaveData(player *_user)
{
	DBInfo *info = DBPool->Alloc();
	info->DBType = DBpacketType::USER_SAVE_DATA;
	GAME_SaveData *data = new GAME_SaveData;
	strcpy_s(data->oid, 25, _user->characterObjectID);
	data->xPos = _user->xPos;
	data->zPos = _user->zPos;
	data->xForward = _user->xForward;
	data->zForward = _user->zForward;
	data->Level = _user->Level;
	data->maxHP = _user->maxHP;
	data->currentHP = _user->currentHP;
	wcscpy_s(data->nickName, 10, _user->nickName);
	info->DBData = (LPVOID*)data;
	DBQ->enqueue(info);
}

void GAMEServer::proc_reqLogout(player *_user)
{
	loginClient->proc_userLogout(_user->accountNo, _user->characterObjectID, _user->Level, _user->nickName);
}

void GAMEServer::proc_move(player *_user, unsigned __int64 _timeVal)
{
	// 현재 프레임과 이전 프레임의 시간 차를 구합니다.

	if (_user->transMovingForward)
	{
		_user->transMovingForward = false;
		_user->overMoveTime = _user->moveTime;
	}
	_user->moveTime += _timeVal;

	int xTile = _user->xTile, yTile = _user->yTile;
	float xSpeed, ySpeed;


	if (_user->moveTime < dfMOVE_FRAME)
		return;

	_user->overMoveTime = 0;

	int moveTime = _user->moveTime / dfMOVE_FRAME;
	_user->moveTime -= (moveTime * dfMOVE_FRAME);
	float moveSpeed = moveTime * dfMOVE_SPEED;
	float xPos = _user->xPos;
	float zPos = _user->zPos;

	xPos += _user->xDir;
	zPos += _user->zDir;

	if (xPos < 0 || xPos >= map->maxPosXSize || zPos >= 0 || zPos <= (map->maxPosYSize * -1))
		return;

	map->transPosToTile(xPos, zPos, &xTile, &yTile);

	if (!map->checkField(xTile, yTile))
		return;

	_user->xPos = xPos;
	_user->zPos = zPos;

	if (_user->xTile != xTile || _user->yTile != yTile)
		map->setTile(_user, xTile, yTile);

	if (sec->updateSector(_user))
		sec->proc_userSectorUpdate(_user);
}

void GAMEServer::proc_perHeal(player *_user)
{
	int currentHP = _user->proc_Heal(healPerTime);
	Sbuf *buf = _user->packet_Recovery(_user->clientID, healPerTime,currentHP);
	sec->sendSectorAround(_user->currentSector.xPos, _user->currentSector.yPos, buf);
	buf->Free();
}

bool GAMEServer::proc_attackVerification(player *_attacker, unsigned __int64 _targetIndex, int _damage, int *_currentHP)
{
	if (!_currentHP)
		return false;
	player *targeter = getUser(_targetIndex);
	if (!targeter)
		return false;

	if (abs(_attacker->xTile - targeter->xTile) > 2 || abs(_attacker->yTile - targeter->yTile) > 2)
		return false;

	int currentHP = targeter->proc_damage(_damage);
	*_currentHP = currentHP;

	return true;
}

