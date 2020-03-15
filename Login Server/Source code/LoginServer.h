#pragma once


struct user {
	unsigned __int64 ssIndex;		// index ID
	__int64 accountNo;	// DB의 accountNo

	bool gameConnect = false;
	bool chatConnect = false;

	charArr *character = NULL;
	bool loginFlag = false;
};

class LoginServer : protected IOCPServer
{
private:

	bool printFlag;			// 출력스레드
	bool whiteFlag;		// 화이트 아이피 옵션
	bool nagleOpt;			// NAGLE OPTION

	char				ip[16];// ip주소
	unsigned short port;	// 포트

	unsigned int maxClient;
	unsigned int threadCount;

	std::list <user*> userList;	// 유저 관리 리스트
	
	memoryPool<user> *userPool;
	memoryPool<charArr> *loadPool;

	SRWLOCK userLock;

	int loginWait;
	int loginSuccess;

	// Encryption Code Key
	BYTE Code, Key1, Key2;

public:
	loginDB* DB;
	loginLan *lanServer;

private:
	// config
	void loadConfig(const char* _config);

	// thread
	static unsigned __stdcall printThread(LPVOID _data);
	
	// 가상함수
	virtual void OnClinetJoin(unsigned __int64 _index);	// accept -> 접속처리 완료 후 호출
	virtual void OnClientLeave(unsigned __int64 _index);		// disconnect 후 호출
	virtual bool OnConnectionRequest(char *_ip, unsigned int _port); // accept 후 [false : 클라이언트 거부 / true : 접속 허용]
	virtual void OnRecv(unsigned __int64 _index, Sbuf *_buf);		// 수신 완료 후
	virtual void OnError(int _errorCode, WCHAR *_string);		// 오류메세지 전송

	// 로직함수
	user* getUser(unsigned __int64 _index);

	bool searchOverlapped(__int64 _accountNo);

	void proc_signUp(unsigned __int64 _index, Sbuf *_buf);			// 클라이언트 회원가입 
	void proc_withDrawal(unsigned __int64 _Index, Sbuf *_buf);		// 클라이언트 회원 탈퇴

	void proc_createChar(unsigned __int64 _Index, Sbuf *_buf);		// 캐릭터 생성
	void proc_deleteChar(unsigned __int64 _Index, Sbuf *_buf);		// 캐릭터 삭제
	void proc_selectChar(unsigned __int64 _Index, Sbuf *_buf);		// 캐릭터 선택
	void proc_charList(unsigned __int64 _Index, Sbuf *_buf);			// 캐릭터 정보 요청
	char* getOID(WCHAR *_name, charArr *_data);

	void proc_userLogin(unsigned __int64 _index, Sbuf* _buf);		// 클라이언트 로그인 처리
	void proc_goodbyeUser(unsigned __int64 _index);

	Sbuf* packet_sendResult(short _Type, char _Result);	// 결과만 보낼 때
	Sbuf* packet_charList(short _Type, unsigned char _Result, charArr *_data);	// 캐릭터 정보 전송 할  때
	Sbuf* packet_serverAuth(int _acNo, unsigned __int64 _sessionKey, char* _charNo);

public:
	LoginServer(const char *_config);
	~LoginServer();

	void terminateServer(void);

	void renewVariable(int *_wait, int *_success);

	void proc_userAuthFromServer(unsigned __int64 _Index, int _serverType);

	friend class loginDB;
};