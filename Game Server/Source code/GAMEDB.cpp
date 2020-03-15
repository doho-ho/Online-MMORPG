#include "stdafx.h"

MONGO_GameDB::MONGO_GameDB(const char *_configData)
{
	mongo = new mongoConnector();
	loadConfig(_configData);
}

MONGO_GameDB::~MONGO_GameDB()
{

}

void MONGO_GameDB::loadConfig(const char *_configData)
{
	rapidjson::Document Doc;
	Doc.Parse(_configData);

	rapidjson::Value &mongoDB = Doc["MONGO_DB"];
	mongo->setDB(mongoDB["URI"].GetString(), mongoDB["DOCUMENT"].GetString(), mongoDB["COLLECTION"].GetString(), mongoDB["SERVER_NAME"].GetString());

}

AUTH_GameData_res* MONGO_GameDB::proc_loadData(AUTH_GameData_req *_data)
{
	char oid[25];
	unsigned __int64 Index;
	strcpy_s(oid, 25, _data->oid);
	Index = _data->Index;

	delete _data;
	bson_oid_t ova;
	bson_oid_init_from_string(&ova, oid);

	bson_t *Selector = bson_new();
	BSON_APPEND_OID(Selector, "_id", &ova);

	if (!mongo->findQuery(Selector))
	{
		bson_free(Selector);
		return false;
	}
	bson_free(Selector);

	char *str;
	rapidjson::Document jsonDocument;
	
	AUTH_GameData_res *out = new AUTH_GameData_res;
	const bson_t *Doc = NULL;
	str = mongo->cursorNext(Doc);
	if (!str)
		return false;
	jsonDocument.Parse(str);
	
	char nickName[20];

	out->Index = Index;
	out->xPos = jsonDocument["xPos"].GetDouble();
	out->zPos = jsonDocument["zPos"].GetDouble();
	out->xForward = jsonDocument["xForward"].GetDouble();
	out->zForward = jsonDocument["zForward"].GetDouble();
	out->Level = jsonDocument["LEVEL"].GetInt();
	out->maxHP = jsonDocument["maxHP"].GetInt();
	out->currentHP = jsonDocument["currentHP"].GetInt();
	strcpy_s(nickName, 20, jsonDocument["NICKNAME"].GetString());
	MultiByteToWideChar(CP_UTF8, 0, nickName, 20, out->nickName, 10);

	return out;
}

bool MONGO_GameDB::proc_saveDatA(GAME_SaveData *_data)
{
	bson_oid_t oid;

	bson_t *Selector = bson_new();


	bson_oid_init_from_string(&oid, _data->oid);
	BSON_APPEND_OID(Selector, "_id", &oid);

	char nick[20];
	WideCharToMultiByte(CP_UTF8, 0, _data->nickName, 10, nick, 20, NULL, NULL);
	
	/*
	bson_t *Query = bson_new();
	BSON_APPEND_UTF8(Query, "NICKNAME", nick);
	BSON_APPEND_DOUBLE(Query, "xPos", _data->xPos);
	BSON_APPEND_DOUBLE(Query, "zPos", _data->zPos);
	BSON_APPEND_DOUBLE(Query, "xForward", _data->xForward);
	BSON_APPEND_DOUBLE(Query, "zForward", _data->zForward);
	BSON_APPEND_INT32(Query, "LEVEL", _data->Level);
	BSON_APPEND_INT32(Query, "HP", _data->HP);
	*/
	bson_t *Query = BCON_NEW("$set", "{", "NICKNAME", BCON_UTF8(nick),
		"xPos", BCON_DOUBLE(_data->xPos), "zPos", BCON_DOUBLE(_data->zPos),
		"xForward", BCON_DOUBLE(_data->xForward), "zForward", BCON_DOUBLE(_data->zForward),
		"LEVEL", BCON_INT32(_data->Level), "maxHP", BCON_INT32(_data->maxHP), "currentHP", BCON_INT32(_data->currentHP), "}");
	delete _data;

	if (!mongo->upsertQuery(Selector,Query))
	{
		bson_free(Query);
		return false;
	}
	bson_free(Query);
	return true;
}