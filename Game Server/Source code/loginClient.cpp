#include "stdafx.h"

LOGINClient::LOGINClient(const char *_configData, GAMEServer *_server)
{
	loadConfig(_configData);
	server = _server;

	Start(IP, Port, workerCount, nagleOpt);
}

LOGINClient::~LOGINClient()
{
	Stop();
}

void LOGINClient::loadConfig(const char *_configData)
{
	// config 파일 읽어와서 멤버 변수에 값 저장.
	rapidjson::Document Doc;
	Doc.Parse(_configData);

	// IP Adress, port, maxCount, threadCount
	// DB 정보
	rapidjson::Value &sev = Doc["GAME_CLIENT"];
	strcpy_s(IP,16, sev["SERVER_IP"].GetString());
	Port = sev["SERVER_PORT"].GetUint();
	nagleOpt = sev["NAGLE"].GetBool();
	workerCount = sev["WORKER_THREAD"].GetUint();
	serverNumber = sev["SERVER_NUMBER"].GetUint();

	rapidjson::Value &gameSev = Doc["NET"];
	strcpy_s(gameIP, 16, gameSev["SERVER_IP"].GetString());
	gamePort = gameSev["SERVER_PORT"].GetInt();
}

void LOGINClient::proc_clientLogin()
{
	Sbuf *buf = NULL;
	buf = packet_clientLogin();
	SendPacket(buf);
	buf->Free();
}

Sbuf* LOGINClient::packet_clientLogin()
{
	// short		Type
	// char		serverNumber
	// char		serverType
	// char		ip[16]
	// short		port
	Sbuf *buf = Sbuf::lanAlloc();
	*buf << (short)login_serverLogin_req;
	*buf << (char)serverNumber;
	*buf << (char)EDHServerType::gameServer;
	buf->push(gameIP, 16);
	*buf << gamePort;

	return buf;
}

void LOGINClient::proc_serverLoginResponse(Sbuf *_buf)
{
	// char		Result
	char Result;
	*_buf >> Result;
	if (Result == login_Result::Success)
		connectedFlag = true;
	else
		CCrashDump::Crash();
}

void LOGINClient::proc_userAuth(Sbuf *_buf)
{
	// unsigned __int64		userIndex
	// char						charOid[25]
	// unsigned __int64		accountNo;

	unsigned __int64 userIndex;
	
	loginInfo *info = new loginInfo;

	*_buf >> userIndex;
	_buf->pop(info->oid, 25);
	*_buf >> info->acNo;

	userAuthMap.insert(std::pair<unsigned __int64, loginInfo*>(userIndex, info));

	Sbuf *buf = NULL;
	buf = packet_userAuth(userIndex);
	SendPacket(buf);
	buf->Free();
}

Sbuf* LOGINClient::packet_userAuth(unsigned __int64 _Index)
{
	// short		Type
	// char		serverType			: 게임서버, 채팅서버 구분 타입
	// unsigned __int64
	Sbuf *buf = Sbuf::lanAlloc();
	*buf << (short)login_userAuth_res;
	*buf << (char)EDHServerType::gameServer;
	*buf << _Index;
	return buf;
}

Sbuf* LOGINClient::packet_userLogout(unsigned __int64 _acNo, char *_oid, int _Level, WCHAR *_nickName)
{
	Sbuf *buf = Sbuf::lanAlloc();
	*buf << (short)game_userLogout_req;
	*buf << _acNo;
	buf->push(_oid, 25);
	*buf << _Level;

	char nickName[20];
	WideCharToMultiByte(CP_UTF8, 0, _nickName, 10, nickName, 20, 0, 0);
	buf->push(nickName, 20);
	return buf;
}

loginInfo* LOGINClient::AUTH_getSessionkey(unsigned __int64 _Index)
{
	std::map<unsigned __int64, loginInfo*>::iterator iter;
	loginInfo *data = NULL;
	char *oid = NULL;
	iter = userAuthMap.find(_Index);
	if (iter == userAuthMap.end())
		return NULL;
	else
	{
		data = iter->second;
		userAuthMap.erase(iter);
		return data;
	}
}

void LOGINClient::proc_userLogout(unsigned __int64 _acNo, char *_oid, int _Level, WCHAR *_nickName)
{
	Sbuf *buf = packet_userLogout(_acNo,_oid,_Level,_nickName);
	SendPacket(buf);
	buf->Free();
}

void LOGINClient::OnClientJoin()
{
	proc_clientLogin();
	server->proc_loginServerConnected(true);
}

void LOGINClient::OnClientLeave()
{
	server->proc_loginServerConnected(false);
}

void LOGINClient::OnRecv(Sbuf *_buf)
{
	short Type;
	*_buf >> Type;
	switch (Type)
	{
	case login_serverLogin_res:
		proc_serverLoginResponse(_buf);
		break;
	case login_userAuth_req:
		proc_userAuth(_buf);
		break;
	default:
		CCrashDump::Crash();
		break;
	}
}

void LOGINClient::OnError(int _errorCode, WCHAR *_string)
{

}

void LOGINClient::OnTPS()
{

}