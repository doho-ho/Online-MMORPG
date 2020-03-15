#include "stdafx.h"

LoginServer::LoginServer(const char* _config)
{
	printf("Starting Login Server...\n\n");

	loadConfig(_config);

	InitializeSRWLock(&userLock);

	printFlag = true;
	
	printf("Intialization complete\n");

	DB = new loginDB(_config, this, true, true);

	userPool = new memoryPool<user>;
	loadPool = new memoryPool<charArr>;
	lanServer = new loginLan(_config, this); 

	loginWait = 0;
	loginSuccess = 0;

	HANDLE hThread;
	hThread = (HANDLE)_beginthreadex(NULL, 0, printThread, (LPVOID)this, 0, 0);
	CloseHandle(hThread);

	printf("Begin thread complete\n");

	setEncodeKey(Code, Key1, Key2);
	Start(ip,port, threadCount, nagleOpt, maxClient);

	printf("Start!\n\n");
}

LoginServer::~LoginServer()
{
	std::list<user*>::iterator iter = userList.begin();
	std::list<user*>::iterator endIter = userList.end();

	for (iter; iter != endIter;iter++)
	{
		userPool->Free((*iter));
		iter = userList.erase(iter);
	}

	printFlag = false;

	delete userPool;
	delete DB;
	delete loadPool;
}

void LoginServer::loadConfig(const char* _config)
{
	// config 파일 읽어와서 멤버 변수에 값 저장.
	rapidjson::Document Doc;
	Doc.Parse(_config);

	// IP Adress, port, maxCount, threadCount
	// DB 정보
	// ini 파일 생성
	rapidjson::Value &sys = Doc["NET"];
	port = sys["SERVER_PORT"].GetUint();
	nagleOpt = sys["NAGLE"].GetBool();
	threadCount = sys["WORKER_THREAD"].GetUint();
	maxClient = sys["MAX_USER"].GetUint();

	rapidjson::Value &code = sys["BUF_KEY"];
	assert(arry.IsArry());

	Code = (char)code[0].GetInt();
	Key1 = (char)code[1].GetInt();
	Key2 = (char)code[2].GetInt();

	printf("Login server config load complete\n");
}

void LoginServer::terminateServer(void)
{
	Stop();
	delete this;
}

unsigned _stdcall LoginServer::printThread(LPVOID _data)
{
	LoginServer *sv = (LoginServer*)_data;
	loginLan *lsv = sv->lanServer;

	SYSTEMTIME stNowTime;
	GetLocalTime(&stNowTime);
	char startTime[100];

	sprintf_s(startTime, 100,"Start Time : %d.%d.%d %02d:%02d:%02d", stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay,
		stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond);

	// 출력 변수
	int loginWait;
	int loginSuccess;

	while (sv->printFlag)
	{
		sv->renewVariable(&loginWait, &loginSuccess);

		printf("=================================================================\n");
		printf("\t\t\t\t*** LoginServer ***\n\n");
		printf("%s\n", startTime);
		printf("[Login] ========================================================\n");
		printf("   playerPool Alloc : [ %04d ]  /  Used : [ %d ] \n", sv->userPool->getAllocCount(), sv->userPool->getUsedCount());
		printf("\n");
		printf("[IOCP] ==========================================================\n");
		printf("   accept Total : [ %d ]  / \n", sv->getAcceptTotal());
		printf("   accept TPS   : [ %d ] \n", sv->getAcceptTPS());
		printf("   Recv TPS     : [ %d ]  /  Send TPS : [ %d ] \n\n", sv->getRecvTPS(), sv->getSendTPS());
		printf("=================================================================\n");
		printf("[LAN] ==========================================================\n");
		printf("   accept Total : [ %d ]  / \n", lsv->getAcceptTotal());
		printf("   accept TPS   : [ %d ] \n", lsv->getAcceptTPS());
		printf("   Recv TPS     : [ %d ]  /  Send TPS : [ %d ] \n\n", lsv->getRecvTPS(), lsv->getSendTPS());
		printf("=================================================================\n");
		Sleep(990);
	}

	printf("Print thread terminated\n");
	return 0;
}

void LoginServer::renewVariable(int *_wait, int *_success)
{
	setTPS();
	_wait = &loginWait;
	_success = &loginSuccess;
}

user* LoginServer::getUser(unsigned __int64 _index)
{
	std::list<user*>::iterator iter = userList.begin();
	std::list<user*>::iterator endIter = userList.end();

	for (iter; iter != endIter; iter++)
	{
		if ((*iter)->ssIndex == _index)
			return (*iter);
	}
	return NULL;
}

bool LoginServer::searchOverlapped(__int64 _accountNo)
{
	std::list<user*>::iterator iter = userList.begin();
	std::list<user*>::iterator endIter = userList.end();

	for (iter; iter != endIter; iter++)
	{
		if ((*iter)->accountNo == _accountNo)
			return true;
	}

	return false;
}

void LoginServer::proc_signUp(unsigned __int64 _Index, Sbuf *_buf)
{
	// char			id[10]
	// char			pass[15]
	char Result = 0;

	char userID[10] = { 0 , };
	char userPass[15] = { 0, };

	_buf->pop(userID, 10);
	_buf->pop(userPass, 15);

	// DB Query
	loginDB_Login_In data;
	strcpy_s(data.id, userID);
	strcpy_s(data.pass, userPass);
	if (!DB->mysql_signUp(&data, NULL))
		Result = login_Result::Overalpped;
	else
		Result = login_Result::Success;

	Sbuf *buf = NULL;
	buf = packet_sendResult(login_signUp_res, Result);
	SendPacket(_Index,buf);
	buf->Free();
}

void LoginServer::proc_withDrawal(unsigned __int64 _Index, Sbuf *_buf)
{
	char Result = login_Result::Failed;
	char userID[10] = { 0 , };
	char userPass[15] = { 0, };

	_buf->pop(userID, 10);
	_buf->pop(userPass, 15);

	// DB Query
	loginDB_Login_In data;
	strcpy_s(data.id, userID);
	strcpy_s(data.pass, userPass);
	if (DB->proc_withDrawal(&data, NULL))
		Result = login_Result::Success;

	Sbuf *buf = NULL;
	buf = packet_sendResult(login_withDrawal_res, Result);
	SendPacket(_Index, buf);
	buf->Free();
}

void LoginServer::proc_createChar(unsigned __int64 _Index, Sbuf *_buf)
{
	// short		Type
	// WCHAR	nickName[10]

	user *client = getUser(_Index);
	if (!client)
		clientShutdown(_Index);

	Sbuf *buf = NULL;

	if (client->character->Count >= 3)
		buf = packet_sendResult(login_createChar_res, login_Result::Failed);
	else
	{
		WCHAR nickName[NICKNAME_SIZE];
		short charType;
		*_buf >> charType;
		_buf->pop((char*)&nickName, 2 * NICKNAME_SIZE);

		// DB 캐릭터 생성 쿼리 전송 & 캐릭터 정보 받아오기
		loginDB_Create_In data;
		wcscpy_s(data.nickName, 10, nickName);
		data.accountNo = client->accountNo;
		data.charType = charType;
		if (!DB->mongo_createChar(&data, NULL))
			buf = packet_sendResult(login_createChar_res, login_Result::Failed);
		else
		{
			charArr *returnData = loadPool->Alloc();
			if (!DB->mongo_loadChar(client->accountNo, returnData))
				CCrashDump::Crash();

			if (client->character)
				loadPool->Free(client->character);
			client->character = returnData;
			if (!DB->recent_createChar(getOID(nickName,returnData),nickName))
				CCrashDump::Crash();
			buf = packet_charList(login_createChar_res, login_Result::Success, client->character);
		}
	}
		SendPacket(_Index, buf);
		buf->Free();
}

void LoginServer::proc_deleteChar(unsigned __int64 _Index, Sbuf *_buf)
{
	// char  charIndex
	char charIndex = 0;

	*_buf >> charIndex;

	user* client = NULL;
	client = getUser(_Index);
	if (!client)
		clientShutdown(_Index);
	
	Sbuf *buf = NULL;

	if (client->character->Count == 0 || client->character->Result[charIndex].level == 0)
		buf = packet_sendResult(login_deleteChar_res, login_Result::Failed);
	else
	{
		WCHAR nickName[NICKNAME_SIZE];
		_buf->pop((char*)&nickName, sizeof(char) * NICKNAME_SIZE);

		// DB 캐릭터 삭제 쿼리 전송 & 캐릭터 정보 받아오기
		if (!DB->proc_deleteChar(client->character->Result[charIndex].charNo, NULL))
			buf = packet_sendResult(login_deleteChar_res, login_Result::Failed);
		else
		{
			charArr *returnData = loadPool->Alloc();
			if (!DB->mongo_loadChar(client->accountNo, returnData))
				CCrashDump::Crash();

			if (client->character)
				loadPool->Free(client->character);
			client->character = returnData;

			buf = packet_charList(login_deleteChar_res, login_Result::Success, client->character);
		}
	}
	SendPacket(_Index, buf);
	buf->Free();
}

void LoginServer::proc_selectChar(unsigned __int64 _index, Sbuf *_buf)
{
	// char		charIndex

	char	charIndex;
	*_buf >> charIndex;

	user* client = NULL;
	client = getUser(_index);
	if (!client)
		clientShutdown(_index);
	
	Sbuf *buf = NULL;
	if (client->character->Result[charIndex].level != 0)
		lanServer->proc_userAuthToServer(client->ssIndex, client->character->Result[charIndex].charNo, client->accountNo);
	else
	{
		buf = packet_sendResult(login_userSelect_res, login_Result::Failed);
		SendPacket(_index, buf);
		buf->Free();
	}
}

void LoginServer::proc_charList(unsigned __int64 _index, Sbuf *_buf)
{
	user* client = NULL;
	client = getUser(_index);
	if (!client)
		clientShutdown(_index);


	Sbuf *buf = NULL;
	buf = packet_charList(login_charList_res, login_Result::Success, client->character);
	SendPacket(_index, buf);
	buf->Free();
}

char* LoginServer::getOID(WCHAR *_name, charArr *_data)
{
	int Count = 0;
	for (Count; Count < _data->Count; Count++)
	{
		if (0 == wcscmp(_data->Result[Count].nickName, _name))
			return _data->Result[Count].charNo;
	}
	return NULL;
}

void LoginServer::proc_userLogin(unsigned __int64 _index, Sbuf *_buf)
{
	// char		id[10]
	// char		pass[15]

	char userID[10] = { 0 ,};
	char userPass[15] = { 0, };
	
	_buf->pop(userID, 10);
	_buf->pop(userPass, 15);

	char Result = login_Result::Failed;
	int accountNo = -1;

	// 1. ID/PASS 조회 및 ACNO 받아와서 일치여부 확인.
	loginDB_Login_In data;
	strcpy_s(data.id, userID);
	strcpy_s(data.pass, userPass);
	if (DB->mysql_login(&data, &accountNo))
	{
		Result = login_Result::Success;
		if (!DB->mysql_updateLoginstatus(accountNo, true))
			CCrashDump::Crash();
	}
	else
		Result = login_Result::Overalpped;
	// 2. 일치한다면 USER DB에 접근해서 ACNO의 캐릭터 및 서버정보 받아온다.

	if (Result != login_Result::Overalpped)
	{
		charArr *returnData = loadPool->Alloc();
		if (!DB->mongo_loadChar(accountNo, returnData))
			CCrashDump::Crash();

		user *client = getUser(_index);
		if (!client)
			clientShutdown(_index);
		client->character = returnData;
		client->accountNo = accountNo;
	}
	Sbuf *buf = NULL;
	buf = packet_sendResult(login_Login_res, Result);

	SendPacket(_index, buf);
	buf->Free();
}

Sbuf* LoginServer::packet_charList(short _Type, unsigned char _Result,  charArr *_data)
{
	// short		Type
	// charArr	Data

	Sbuf *buf = Sbuf::Alloc();

	*buf << _Type;
	*buf << _Result;

	if (_Result != login_Result::Success)
		return buf;

	*buf << _data->Count;
	int count = 0;
	for (count; count < _data->Count; count++)
	{
		buf->push(_data->Result[count].charNo, sizeof(char) * 25);
		buf->push((char*)&_data->Result[count].nickName, sizeof(WCHAR) * 10);
		*buf << _data->Result[count].level;
		*buf << _data->Result[count].charType;
	}

	return buf;
}

Sbuf* LoginServer::packet_sendResult(short _Type, char _result)
{
	// short		Type
	// char		result	

	Sbuf *buf = Sbuf::Alloc();

	*buf << _Type;
	*buf << _result;

	return buf;
}

Sbuf* LoginServer::packet_serverAuth(int _acNo, unsigned __int64 _sessionKey, char* _charNo)
{
	// unsigned short Type
	// unsigned unsigned int				accountNo
	// int		characterNo
	// unsigned __int64		sessionKey

	Sbuf *buf = Sbuf::Alloc();

	*buf << (unsigned short)login_userAuth_req;
	*buf <<  _acNo;
	*buf << _sessionKey;
	buf->push(_charNo,25);

	return buf;
}

void LoginServer::proc_goodbyeUser(unsigned __int64 _index)
{
	Sbuf* buf = Sbuf::Alloc();
	buf = lanServer->packet_userGameConnect(_index);
	user *client = getUser(_index);
	if(client)
		client->loginFlag = true;
	SendPacket(_index, buf);
	buf->Free();
}

void LoginServer::OnClinetJoin(unsigned __int64 _index)
{
	user *client = userPool->Alloc();
	client->ssIndex = _index;
	client->accountNo = -1;
	AcquireSRWLockExclusive(&userLock);
	userList.push_back(client);
	ReleaseSRWLockExclusive(&userLock);
}

void LoginServer::OnClientLeave(unsigned __int64 _index)
{
	user *client = NULL;
	AcquireSRWLockExclusive(&userLock);
	std::list<user*>::iterator iter = userList.begin();
	std::list<user*>::iterator endIter = userList.end();
	for (iter; iter != endIter; iter++)
	{
		if ((*iter)->ssIndex == _index)
		{
			client = (*iter);

			if (client->loginFlag == true)
				DB->mysql_updateLoginstatus(client->accountNo, false);

			client->ssIndex = 0;
			client->accountNo = 0;
			client->chatConnect = false;
			client->gameConnect = false;
			client->loginFlag = false;

			if (client->character)
				loadPool->Free(client->character);

			client->character = NULL;
			userPool->Free(client);
			userList.erase(iter);
			break;
		}
	}
	ReleaseSRWLockExclusive(&userLock);
}

bool LoginServer::OnConnectionRequest(char *_ip, unsigned int _port)
{
	if (whiteFlag == true)
	{
		// 서버 점검 중일 때 테스트를 위한 ip만 접속 허용기능
	}
	return true;
}

void LoginServer::OnRecv(unsigned __int64 _index, Sbuf *_buf)
{
	short Type;
	*_buf >> Type;

	switch (Type)
	{
	case login_signUp_req:
		proc_signUp(_index, _buf);
		break;
	case login_withDrawal_req:
		proc_withDrawal(_index, _buf);
		break;
	case login_Login_req:
		proc_userLogin(_index, _buf);
		break;
	case login_createChar_req:
		proc_createChar(_index, _buf);
		break;
	case login_deleteChar_req:
		proc_deleteChar(_index, _buf);
		break;
	case login_userSelect_req:
		proc_selectChar(_index, _buf);
		break;
	case login_charList_req:
		proc_charList(_index, _buf);
		break;
	default:
		CCrashDump::Crash();
		break;
	}
}

void LoginServer::OnError(int _errorCode, WCHAR *_string)
{
	// 별도의 오류 매크로 사용

}

void LoginServer::proc_userAuthFromServer(unsigned __int64 _Index, int _serverType)
{
	user *client = getUser(_Index);
	if (!client)
		clientShutdown(_Index);

	if (_serverType == EDHServerType::gameServer)
		client->gameConnect = true;
	else if (_serverType == EDHServerType::chatServer)
		client->chatConnect = true;
	else
		CCrashDump::Crash();

	if (client->gameConnect == true)
		proc_goodbyeUser(_Index);
}