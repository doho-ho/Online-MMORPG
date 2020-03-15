#include "stdafx.h"

loginLan::loginLan(const char*_configData, LoginServer *_serverPtr)
{
	loadConfig(_configData);

	monitorThreadFlag = true;
	loginServer = _serverPtr;

	Start(IP, Port, workerCount, nagleOpt, maxSessionCount);
}

loginLan::~loginLan()
{
	monitorThreadFlag = false;
	Stop();
}

void loginLan::loadConfig(const char *_configData)
{
	rapidjson::Document Doc;
	Doc.Parse(_configData);

	rapidjson::Value &lanClientDoc = Doc["LAN"];
	strcpy_s(IP, 16, lanClientDoc["SERVER_IP"].GetString());
	Port = lanClientDoc["SERVER_PORT"].GetInt();
	nagleOpt = lanClientDoc["NAGLE_OPT"].GetBool();
	maxSessionCount = lanClientDoc["MAX_USER_COUNT"].GetUint64();
	workerCount = lanClientDoc["WORKER_THREAD_COUNT"].GetInt();
	serverCount = lanClientDoc["SERVER_COUNT"].GetUint();
}

void loginLan::proc_serverLogin(unsigned __int64 _Index, Sbuf *_buf)
{
	// char		serverNumber
	// char		serverType
	// char		ip[16]
	// short		port

	char serverNumber = -1;
	char serverType = -1;
	char ip[16];
	short port = -1;

	*_buf >> serverNumber;
	*_buf >> serverType;
	_buf->pop(ip, 16);
	*_buf >> port;

	if (serverNumber == -1 || serverType == -1 || port == -1)
		CCrashDump::Crash();

	std::vector<connectedServer*>::iterator iter = serverList.begin();
	std::vector<connectedServer*>::iterator endIter = serverList.end();
	for (iter; iter != endIter; iter++)
	{
		if ((*iter)->Index ==_Index)
		{
			(*iter)->serverNumber = serverNumber;
			(*iter)->serverType = serverType;
			(*iter)->serverPort = port;
			strcpy_s((*iter)->serverIp, 16, ip);
			break;
		}
	}

	Sbuf *buf = packet_serverLoginRes();
	SendPacket(_Index, buf);
	buf->Free();

}

void  loginLan::proc_userAuthFromServer(unsigned __int64 _Index, Sbuf *_buf)
{
	// char					serverType		
	// unsigned __int64 sessionKey

	char serverType = -1;
	unsigned __int64 sessionKey = -1;

	*_buf >> serverType;
	*_buf >> sessionKey;
	loginServer->proc_userAuthFromServer(sessionKey, serverType);
}

void loginLan::proc_userAuthToServer(unsigned __int64 _Index, char *_oid, unsigned __int64 _acNo)
{
	Sbuf *buf = packet_userAuthToServer(_Index, _oid, _acNo);
	
	std::vector<connectedServer*>::iterator iter = serverList.begin();
	std::vector<connectedServer*>::iterator endIter = serverList.end();
	for (iter; iter != endIter; iter++)
	{
		if ((*iter)->serverType == EDHServerType::gameServer)
		{
			SendPacket((*iter)->Index, buf);
			buf->Free();
			break;
		}
	}
}

void loginLan::proc_userLogout(unsigned __int64 _Index, Sbuf *_buf)
{
	// unsigned __int64 accountNo;
	// char		oid[25]
	// int			Level
	// char	nickName[20]
	unsigned __int64 accountNo;
	char oid[25];
	int Level;
	char nickName[20];
	*_buf >> accountNo;
	_buf->pop(oid, 25);
	*_buf >> Level;
	_buf->pop(nickName, 20);

	loginDB_Update_In *data = new loginDB_Update_In;
	strcpy_s(data->oid, 25, oid);
	strcpy_s(data->nickName, 20, nickName);
	data->Level = Level;

	loginServer->DB->mysql_updateLoginstatus(accountNo, false);
	loginServer->DB->mongo_updateChar(data,NULL);
	delete data;
}

Sbuf* loginLan::packet_userAuthToServer(unsigned __int64 _Index, char *_oid, unsigned __int64 _acNo)
{
	// short						Type
	// unsigned __int64		userIndex
	// char						charOid[25]
	//	unsigned __int64		accountNo

	Sbuf *buf = Sbuf::lanAlloc();
	*buf << (short)login_userAuth_req;
	*buf << _Index;
	buf->push(_oid, 25);
	*buf << _acNo;
	return buf;
}

Sbuf* loginLan::packet_serverLoginRes()
{
	// short		Type
	// char		Result
	Sbuf *buf = Sbuf::lanAlloc();
	*buf << (short)login_serverLogin_res;
	*buf << (char)login_Result::Success;
	return buf;
}

Sbuf* loginLan::packet_userGameConnect(unsigned __int64 _sessionKey)
{
	// short		                Type
	// unsigned __int64		sessionKey
	// char	                    gameIp[16]
	// short		                gamePort

	Sbuf *buf = Sbuf::Alloc();

	*buf << (unsigned short)login_userGameConnect_res;
	*buf << _sessionKey;

	std::vector<connectedServer*>::iterator iter = serverList.begin();
	std::vector<connectedServer*>::iterator endIter = serverList.end();
	for (iter; iter != endIter; iter++)
	{
		if ((*iter)->serverType == EDHServerType::gameServer)
		{
			buf->push((*iter)->serverIp, 16);
			*buf << (*iter)->serverPort;
			break;
		}
	}

	return buf;
}

void loginLan::OnClientJoin(unsigned __int64 _Index)
{
	connectedServer* server = new connectedServer;
	serverList.push_back(server);
	server->Index = _Index;
}

void loginLan::OnClientLeave(unsigned __int64 _Index)
{
	std::vector<connectedServer*>::iterator iter = serverList.begin();
	std::vector<connectedServer*>::iterator endIter = serverList.end();

	for (iter; iter != endIter; iter++)
	{
		if ((*iter)->Index == _Index)
		{
			connectedServer *data = (*iter);
			serverList.erase(iter);
			delete data;
			break;
		}
	}
}

void loginLan::OnRecv(unsigned __int64 _Index, Sbuf *_buf)
{
	short Type = -1;
	*_buf >> Type;

	switch (Type)
	{
	case login_userAuth_res:
		proc_userAuthFromServer(_Index, _buf);
		break;
	case login_serverLogin_req:
		proc_serverLogin(_Index, _buf);
		break;
	case game_userLogout_req:
		proc_userLogout(_Index, _buf);
		break;
	default:
		CCrashDump::Crash();
		break;
	}
}

void loginLan::OnSend(unsigned __int64 _Index, int _sendSize)
{

}

void loginLan::OnError(int _errorCode, WCHAR *_string)
{

}

unsigned __stdcall loginLan::monitorThread(LPVOID _data)
{
	loginLan *server = (loginLan*)_data;

	while (server->monitorThreadFlag)
	{

	}
	return 0;
}