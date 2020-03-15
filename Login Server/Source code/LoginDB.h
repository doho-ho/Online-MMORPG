#pragma once

struct loginDB_Login_In {
	char id[10];
	char pass[15];
};

struct loginDB_Create_In {
	WCHAR nickName[10];
	unsigned __int64 accountNo;
	int charType;
};

struct loginDB_Update_In {
	char oid[25];
	int	Level;
	char nickName[20];
};

struct charSelect
{
	char	charNo[25];		// 캐릭터 번호
	WCHAR	nickName[10];		// 캐릭터 이름
	int	level;		// 캐릭터 레벨
	int charType;
};


struct charArr {
	char Count;
	charSelect Result[3];
};

class LoginServer;

class loginDB
{
private:
	LoginServer *server;
	
	mysqlConnector *mysql;
	mongoConnector *mongo;
	mongoConnector *recentDB;

private:
	void loadConfig(const char *_config, bool _Mysql, bool _Mongo);
	bool recent_createChar(loginDB_Create_In *_in, LPVOID *_out);

public:
	loginDB(const char *_Config, LoginServer *_server ,bool _Mysql = false, bool _Mongo = false);
	~loginDB();

	bool proc_withDrawal(loginDB_Login_In *_in, LPVOID *_out);
	bool proc_deleteChar(char *_in, LPVOID *_out);

	bool mysql_login(loginDB_Login_In *_in, int *_out);
	bool mysql_withDrawal(int _in);
	bool mysql_updateLoginstatus(unsigned __int64 _acNo, bool _val);
	bool mysql_signUp(loginDB_Login_In *_in, LPVOID *_out);

	bool mongo_createChar(loginDB_Create_In *_in, LPVOID *_out);
	bool mongo_deleteChar(char *_in, LPVOID *_out);
	bool mongo_loadChar(const __int64 _in, charArr *_out);
	bool mongo_updateChar(loginDB_Update_In *_in, LPVOID *_out);

	bool recent_createChar(char *_in, WCHAR *_nickName);
	bool recent_deleteChar(char *_in);
};