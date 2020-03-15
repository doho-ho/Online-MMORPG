#include "stdafx.h"



loginDB::loginDB(const char *_config, LoginServer *_server ,bool _Mysql, bool _Mongo) 
{
	server = _server;
	mysql = new mysqlConnector();
	mongo = new mongoConnector();
	recentDB = new mongoConnector();

	loadConfig(_config, _Mysql, _Mongo);
}

loginDB::~loginDB()
{

}

void loginDB::loadConfig(const char *_config, bool _Mysql, bool _Mongo)
{
	rapidjson::Document Doc;
	Doc.Parse(_config);

	if (_Mysql)
	{
		rapidjson::Value &mysqlDB = Doc["MYSQL_DB"];
		mysql->setDB(mysqlDB["HOST"].GetString(), mysqlDB["USER"].GetString(), mysqlDB["PASS"].GetString(), mysqlDB["DB_NAME"].GetString(), mysqlDB["PORT"].GetUint());
	}

	if (_Mongo)
	{
		rapidjson::Value &mongoDB = Doc["MONGO_DB"];
		mongo->setDB(mongoDB["URI"].GetString(), mongoDB["DOCUMENT"].GetString(), mongoDB["COLLECTION"].GetString(),mongoDB["SERVER_NAME"].GetString());

		rapidjson::Value &recent = Doc["RECENT_DB"];
		recentDB->setDB(recent["URI"].GetString(), recent["DOCUMENT"].GetString(), recent["COLLECTION"].GetString(), recent["SERVER_NAME"].GetString());
	}
}

bool loginDB::proc_withDrawal(loginDB_Login_In *_in, LPVOID *_out)
{
	int accountNo;
	if (!mysql_login(_in, &accountNo))
		return false;
	
	if (!mysql_withDrawal(accountNo))
		return false;

	charArr *returnData = server->loadPool->Alloc();

	if (!mongo_loadChar(accountNo, returnData))
		return false;

	int Count = 0;
	for (Count; Count < returnData->Count; Count++)
	{
		if (!proc_deleteChar(returnData->Result[Count].charNo, NULL)) {
			server->loadPool->Free(returnData);
			return false;
		}
	}

	server->loadPool->Free(returnData);
	return true;
}

bool loginDB::proc_deleteChar(char *_in, LPVOID *_out)
{
	if (!mongo_deleteChar(_in, _out))
		return false;
	if (!recent_deleteChar(_in))
		return false;

	return true;
}

bool loginDB::mysql_login(loginDB_Login_In *_in, int *_out)
{
	std::string Query = "SELECT `accountno`,`loginState` FROM `mmorpg`.`account` WHERE `id`=\"";
	Query += _in->id;
	Query += "\" AND `pass`=\"";
	Query += _in->pass;
	Query += "\"";
	
	if (!mysql->selectQuery(Query))
		return false;

	int loginState;
	MYSQL_ROW row = mysql->fetchRow();
	if (row)
	{
		*_out = atoi(row[0]);
		loginState = atoi(row[1]);
		mysql->freeResult();
		if (loginState == true)
			return false;
		return true;
	}

	return false;
}

bool loginDB::mysql_withDrawal(int _in)
{
	std::string Query = "DELETE FROM `mmorpg`.`account` WHERE `accountNo`='";
	Query += std::to_string(_in);
	Query += "'";

	return mysql->Query(Query);
}

bool loginDB::mysql_updateLoginstatus(unsigned __int64 _acNo, bool _val)
{
	std::string Query = "UPDATE `mmorpg`.`account` SET `loginState`='";
	Query += std::to_string(_val);
	Query += "' WHERE `accountNo` = '";
	Query += std::to_string(_acNo);
	Query += "'";
	return mysql->Query(Query);
}

bool loginDB::mysql_signUp(loginDB_Login_In *_in, LPVOID *_out)
{
	std::string Query = "INSERT INTO `mmorpg`.`account` (`id`, `pass`) VALUES('";
	Query += _in->id;
	Query += "','";
	Query += _in->pass;
	Query += "')";
	return mysql->Query(Query);
}

bool loginDB::mongo_createChar(loginDB_Create_In *_in, LPVOID *_out)
{
	loginDB_Create_In *data = (loginDB_Create_In*)_in;

	bson_oid_t oid;
	//bson_t *Selector = bson_new();
	bson_t *Query = bson_new();

	//bson_oid_init(&oid, NULL);
	//BSON_APPEND_OID(Selector, "_id", &oid);

	char nick[20];

	WideCharToMultiByte(CP_UTF8, 0, data->nickName, 10, nick, 20,NULL,NULL);

	
	BSON_APPEND_UTF8(Query, "NICKNAME", nick);
	BSON_APPEND_INT32(Query, "ACCOUNTNO", data->accountNo);
	BSON_APPEND_INT32(Query, "CHARTYPE", data->charType);
	BSON_APPEND_INT32(Query, "LEVEL",1);

	if (!mongo->insertQuery(Query))
	{
		bson_free(Query);
		return false;
	}
	bson_free(Query);
	return true;
}

bool loginDB::recent_createChar(loginDB_Create_In *_in, LPVOID *_out)
{
	loginDB_Create_In *data = (loginDB_Create_In*)_in;

	bson_oid_t oid;
	//bson_t *Selector = bson_new();
	bson_t *Query = bson_new();

	//bson_oid_init(&oid, NULL);
	//BSON_APPEND_OID(Selector, "_id", &oid);

	char nick[20];

	WideCharToMultiByte(CP_UTF8, 0, data->nickName, 10, nick, 20, NULL, NULL);


	BSON_APPEND_UTF8(Query, "NICKNAME", nick);
	BSON_APPEND_INT32(Query, "ACCOUNTNO", data->accountNo);
	BSON_APPEND_INT32(Query, "CHARTYPE", data->charType);
	BSON_APPEND_INT32(Query, "LEVEL", 1);

	if (!mongo->insertQuery(Query))
	{
		bson_free(Query);
		return false;
	}
	bson_free(Query);
	return true;
}

bool loginDB::mongo_deleteChar(char *_in, LPVOID *_out)
{
	bson_t *Selector = bson_new();
	bson_oid_t oid;
	bson_oid_init_from_string(&oid, _in);
	BSON_APPEND_OID(Selector, "_id", &oid);

	if (!mongo->deleteQuery(Selector))
	{
		bson_free(Selector);
		return false;
	}
	bson_free(Selector);
	return true;
}

bool loginDB::mongo_loadChar(const __int64 _in, charArr *_out)
{
	rapidjson::Document Doc;
	bson_t *Selector = bson_new();
	BSON_APPEND_INT64(Selector, "ACCOUNTNO", _in);
	if (!mongo->findQuery(Selector))
	{
		bson_free(Selector);
		return false;
	}
	bson_free(Selector);
	char* str;
	int arrIndex = 0;
	charSelect *arr = NULL;
	_out->Count = 0;
	char dummyNick[20];
	const bson_t *doc = nullptr;
	while (str = mongo->cursorNext(doc))
	{
		if (!str)
			break;
		arr = &_out->Result[arrIndex];
		Doc.Parse(str);

		rapidjson::Value &sys = Doc["_id"];
		strcpy_s(arr->charNo,25, sys["$oid"].GetString());

		strcpy_s(dummyNick, 20, Doc["NICKNAME"].GetString());
		MultiByteToWideChar(CP_UTF8, 0, dummyNick, 20, arr->nickName, 32);
		arr->level = Doc["LEVEL"].GetUint();
		arr->charType = Doc["CHARTYPE"].GetUint();

		arrIndex++;
		_out->Count++;
		mongo->strFree();
	}
	for (arrIndex; arrIndex < 3; arrIndex++)
		_out->Result[arrIndex].level = 0;

	return true;
}

bool loginDB::mongo_updateChar(loginDB_Update_In *_in, LPVOID *_out)
{
	bson_oid_t ova;
	bson_oid_init_from_string(&ova, _in->oid);

	bson_t *Selector = bson_new();
	BSON_APPEND_OID(Selector, "_id", &ova);
	bson_t *Query = bson_new();
	BSON_APPEND_UTF8(Query, "NICKNAME",_in->nickName);
	BSON_APPEND_INT32(Query, "LEVEL", _in->Level);

	if (!mongo->upsertQuery(Selector, Query))
	{
		bson_free(Selector);
		bson_free(Query);
		return false;
	}
	bson_free(Selector);
	bson_free(Query);
	return true;
}

bool loginDB::recent_createChar(char *_in, WCHAR *_nickName)
{
	if (!_in) return false;

	bson_oid_t ova;
	bson_oid_init_from_string(&ova, _in);

	bson_t *Query = bson_new();
	BSON_APPEND_OID(Query, "_id", &ova);
	
	char nick[20];
	WideCharToMultiByte(CP_UTF8, 0, _nickName, 10, nick, 20, NULL, NULL);

	BSON_APPEND_UTF8(Query, "NICKNAME", nick);
	BSON_APPEND_DOUBLE(Query, "xPos",50);
	BSON_APPEND_DOUBLE(Query, "zPos", -50);
	BSON_APPEND_DOUBLE(Query, "xForward", 0);
	BSON_APPEND_DOUBLE(Query, "zForward", 1);
	BSON_APPEND_INT32(Query, "LEVEL", 1);
	BSON_APPEND_INT32(Query, "maxHP", 100);
	BSON_APPEND_INT32(Query, "currentHP", 100);

	if (!recentDB->insertQuery(Query))
	{
		bson_free(Query);
		return false;
	}
	bson_free(Query);
	return true;
}

bool loginDB::recent_deleteChar(char *_in)
{
	bson_t *Selector = bson_new();
	bson_oid_t oid;
	bson_oid_init_from_string(&oid, _in);
	BSON_APPEND_OID(Selector, "_id", &oid);

	if (!recentDB->deleteQuery(Selector))
	{
		bson_free(Selector);
		return false;
	}
	bson_free(Selector);
	return true;
}