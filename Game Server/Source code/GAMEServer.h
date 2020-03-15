#pragma once

#define healTickTime	1000
#define healPerTime	100

class GAMEServer : public MMOServer
{
private:
	bool		printFlag;
	bool		DBFlag;
	int			Version;

	unsigned int maxUser;

	const char *configData;
	const char *mapConfig;
	int	mapFileSize;

	bool loginServerConnectedFlag;
		
protected:
	LONG DBFrame;
	LONG pDBFrame;

public:
	LOGINClient *loginClient;
	player *playerArray;

	Map *map;
	Sector *sec;
	
	lockFreeQueue<AUTH_GameData_res*> *authDBQ;
	lockFreeQueue<DBInfo*> *DBQ;
	memoryPool<DBInfo> *DBPool;

private:
	void loadConfig(const char *_configData);
	char* loadMap(const char *_fileName);

	void gameServermonitoringDataUpdate();

	player* getUser(unsigned __int64 _Index);

	void proc_reqUserData(void);
	void proc_resUserData(void);

	void proc_move(player *_user, unsigned __int64 _timeVal);
	void proc_perHeal(player *_user);

public:
	GAMEServer(const char *_configData);
	~GAMEServer();

	int getVersion();
	void proc_loginServerConnected(bool _val);
	void proc_reqSaveData(player *_user);
	void proc_reqLogout(player *_user);
	bool proc_attackVerification(player *_attacker, unsigned __int64 _targetIndex, int _damage, int *_currentHP);

	static unsigned __stdcall printThread(LPVOID _data);
	static unsigned __stdcall DBThread(LPVOID _data);
		
	// 가상함수
	virtual void onAuth_Update(void);
	virtual void onGame_Update(void);

};