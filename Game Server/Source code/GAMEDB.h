#pragma once

enum DBpacketType{
	USER_LOAD_DATA, USER_SAVE_DATA
};

struct DBInfo {
	DBpacketType DBType;
	LPVOID			*DBData;
};

struct AUTH_GameData_req {
	unsigned __int64 Index;
	char oid[25];
};

struct GAME_SaveData {
	char oid[25];
	float xPos, zPos, xForward, zForward;
	int Level, maxHP, currentHP;
	WCHAR nickName[10];
};

struct AUTH_GameData_res {
	unsigned __int64 Index;
	float xPos, zPos, xForward, zForward;
	int Level, maxHP, currentHP;
	WCHAR nickName[10];
};

class MONGO_GameDB : public mongoConnector
{
private:
	mongoConnector *mongo;

private:
	void loadConfig(const char *_configData);
public:
	MONGO_GameDB(const char *_configData);
	~MONGO_GameDB();


	AUTH_GameData_res* proc_loadData(AUTH_GameData_req *_data);
	bool proc_saveDatA(GAME_SaveData *_data);
};