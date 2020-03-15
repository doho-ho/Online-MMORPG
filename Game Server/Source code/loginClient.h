#pragma once

struct loginInfo {
	char oid[25];
	unsigned __int64 acNo;
};

class LOGINClient : public LanClient
{
private:
	char IP[16];
	short Port;
	bool nagleOpt;
	int		workerCount;
	int		serverNumber;

	char gameIP[16];
	short	gamePort;

	GAMEServer *server;
	
	bool connectedFlag;

	std::map<unsigned __int64, loginInfo*> userAuthMap;

private:
	void loadConfig(const char *_configData);

	void proc_clientLogin();
	void proc_serverLoginResponse(Sbuf *_buf);
	void proc_userAuth(Sbuf *_buf);

	Sbuf* packet_clientLogin();
	Sbuf* packet_userAuth(unsigned __int64 _Index);
	Sbuf* packet_userLogout(unsigned __int64 _acNo, char *_oid, int _Level, WCHAR *_nickName);

public:
	LOGINClient(const char *_configData, GAMEServer *_server);
	~LOGINClient();

	// 로직함수
	loginInfo* AUTH_getSessionkey(unsigned __int64 _Index);
	void proc_userLogout(unsigned __int64 _acNo, char *_oid, int _Level, WCHAR *_nickName);
	// 가상함수
	virtual void OnClientJoin(void);
	virtual void OnClientLeave(void);
	virtual void OnRecv(Sbuf *_buf);		// 수신 완료 후
	virtual void OnError(int _errorCode, WCHAR *_string);		// 오류메세지 전송
	virtual void OnTPS(void);
};