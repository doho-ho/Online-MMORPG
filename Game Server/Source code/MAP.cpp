#include "stdafx.h"
#include "MAP.h"

Map::Map(const char *_configData,const int _fileSize)
{
	initMap(_configData,_fileSize);
}

Map::~Map()
{
	int Count = 0;
	for (Count; Count < xSize; Count++)
	{
		delete[] field[Count];
		delete[] tile[Count];
	}
	delete[] field;
	delete[] tile;
}


void Map::initMap(const char *_configData,const int _fileSize)
{
	char* buffer = new char[_fileSize];
	memcpy_s(buffer, _fileSize, _configData, _fileSize);

	char seps[] = "\n";
	char seps1[] = ",";
	char* token = NULL;
	char* nextToken = NULL;
	char* result = NULL;

	token = strtok_s(buffer, seps, &nextToken);

	result = strtok_s(NULL, seps1, &token);
	while (result)
	{
		if (!strcmp(result, "tileSize"))
		{
			result = strtok_s(NULL, seps1, &token);
			xSize = atoi(result);
			result = strtok_s(NULL, seps1, &token);
			ySize = atoi(result);
		}
		if (!strcmp(result, "posMax"))
		{
			result = strtok_s(NULL, seps1, &token);
			maxPosXSize = atoi(result);
			result = strtok_s(NULL, seps1, &token);
			maxPosYSize = atoi(result);
		}
		result = strtok_s(NULL, seps1, &token);
	}

	positionPerTileXSize = maxPosXSize / xSize;
	positionPerTileYSize = maxPosYSize / ySize;

	token = strtok_s(NULL, seps, &nextToken);

	int Count = 0;
	field = new bool*[ySize];
	tile = new tileNode*[ySize];
	for (Count; Count < xSize; Count++)
	{
		field[Count] = new bool[xSize];
		tile[Count] = new tileNode[xSize];
	}

	int xPos = 0;
	int yPos = 0;

	while (token != NULL)
	{
		while (1)
		{
			result = strtok_s(NULL, seps1, &token);
			if (xPos == xSize || !result) break;
			if (strcmp(result, "X") == 0 || strcmp(result, "x") == 0)
				field[yPos][xPos] = false;
			else
				field[yPos][xPos] = true;
			xPos++;
		}
		token = strtok_s(NULL, seps, &nextToken);
		yPos++;
		xPos = 0;
	}
	delete buffer;
}

void Map::transPosToTile(float _xPos, float _yPos, int *_xTile, int *_yTile)
{
	if (!_xTile || !_yTile)
		return;

	*_xTile = _xPos / positionPerTileXSize;
	*_yTile = abs(_yPos) / positionPerTileYSize;

	if (*_xTile < 0 || *_xTile >= xSize)
		*_xTile = -1;
	if (*_yTile < 0 || *_yTile >= ySize)
		*_yTile = -1;
}

void Map::setTile(player *_user, int _xTile, int _yTile)
{
	if (_xTile < 0 || _xTile >= xSize
		|| _yTile<0 || _yTile >= ySize)
		return;

	int nXtile = _user->xTile;
	int nYtile = _user->yTile;

	if (nXtile == _xTile && nYtile == _yTile) return;

	if (nXtile != -1 && nYtile != -1)
		tile[nYtile][nXtile].userList.remove(_user);

	
	_user->xTile = _xTile;
	_user->yTile = _yTile;

	tile[_user->yTile][_user->xTile].userList.push_back(_user);
}

void Map::removeTile(player *_user)
{
	int nXtile = _user->xTile;
	int nYtile = _user->yTile;

	if (nXtile != -1 && nYtile != -1)
		tile[nYtile][nXtile].userList.remove(_user);

	_user->xTile = -1;
	_user->yTile = -1;
}

bool Map::checkField(int _xTile, int _yTile)
{
	if (field[_yTile][_xTile])
		return true;
	return false;
}

tileNode* Map::getTile(int _xTile, int _yTile)
{
	return &tile[_yTile][_xTile];
}

// 섹터처리

Sector::Sector(const char *_configData, const int _fileSize)
{
	initSector(_configData, _fileSize);
}

Sector::~Sector()
{

}

void Sector::initSector(const char *_configData, const int _fileSize)
{
	char* buffer = new char[_fileSize];
	memcpy_s(buffer, _fileSize, _configData, _fileSize);

	char seps[] = "\n";
	char seps1[] = ",";
	char* token = NULL;
	char* nextToken = NULL;
	char* result = NULL;

	token = strtok_s(buffer, seps, &nextToken);

	result = strtok_s(NULL, seps1, &token);
	while (result)
	{
		if (!strcmp(result, "sectorSize"))
		{
			result = strtok_s(NULL, seps1, &token);
			xSize = atoi(result);
			result = strtok_s(NULL, seps1, &token);
			ySize = atoi(result);
		}
		if (!strcmp(result, "tileToSector"))
		{
			result = strtok_s(NULL, seps1, &token);
			tileSizePerSectorXSize = atoi(result);
			result = strtok_s(NULL, seps1, &token);
			tileSizePerSectorYSize = atoi(result);
		}
		result = strtok_s(NULL, seps1, &token);
	}

	token = strtok_s(NULL, seps, &nextToken);

	int Count = 0;
	fieldSector = new sector*[ySize];
	for (Count; Count < ySize; Count++)
		fieldSector[Count] = new sector[xSize];

	int xPos = 0;
	int yPos = 0;

	delete buffer;
}

void Sector::transTileToSector(int _xTile, int _yTile, int *_xSector, int *_ySector)
{
	if (!_xSector || !_ySector) return;

	*_xSector = _xTile / tileSizePerSectorXSize;
	*_ySector = _yTile / tileSizePerSectorYSize;
}

bool Sector::addSector(player *_user)
{
	sectorInfo *cur = &_user->currentSector;
	sectorInfo *old = &_user->previousSector;

	if (cur->xPos != -1 || cur->yPos != -1)
		return false;

	int nXpos = -1;
	int nYpos = -1;

	transTileToSector(_user->xTile, _user->yTile, &nXpos, &nYpos);

	if (nXpos < 0 || nXpos >= xSize || nYpos<0 || nYpos >= ySize)
		return false;

	fieldSector[nYpos][nXpos].userList.push_back(_user);

	old->xPos = cur->xPos;
	old->yPos = cur->yPos;
	cur->xPos = nXpos;
	cur->yPos= nYpos;

	return true;
}

bool Sector::removeSector(player *_user)
{
	sectorInfo *cur = &_user->currentSector;
	sectorInfo *old = &_user->previousSector;

	if (cur->xPos == -1 || cur->yPos == -1)
		return false;

	std::list<player*>*userList = &fieldSector[cur->yPos][cur->xPos].userList;
	userList->remove(_user);

	old->xPos = cur->xPos;
	old->yPos = cur->yPos;
	cur->xPos = -1;
	cur->yPos = -1;
	return true;
}

bool Sector::updateSector(player *_user)
{
	sectorInfo *cur = &_user->currentSector;
	sectorInfo *old = &_user->previousSector;

	int xSector = -1;
	int ySector = -1;

	transTileToSector(_user->xTile, _user->yTile, &xSector, &ySector);
	if (cur->xPos == xSector && cur->yPos == ySector)
		return false;

	int oldX = cur->xPos;
	int oldY = cur->yPos;
	removeSector(_user);
	addSector(_user);

	old->xPos = oldX;
	old->yPos = oldY;
	return true;
}

void Sector::getSectorAround(int _xSector, int _ySector, sectorAround *_around)
{
	if (_xSector == -1 || _ySector == -1) return;
	int xCount = 0, yCount = 0;

	_xSector--;
	_ySector--;

	_around->count = 0;

	for (yCount; yCount < 3; yCount++)
	{
		if (_ySector + yCount < 0 || _ySector + yCount >= xSize)
			continue;
		xCount = 0;
		for (xCount; xCount < 3; xCount++)
		{
			if (_xSector + xCount < 0 || _xSector + xCount >= ySize)
				continue;

			_around->data[_around->count].xTile = _xSector + xCount;
			_around->data[_around->count].yTile = _ySector + yCount;
			_around->count++;
		}
	}
	return;
}

void Sector::getUpdateSectorAround(player *_user, sectorAround *_remove, sectorAround *_add)
{
	int oldCount = 0, curCount = 0;
	bool findFlag;
	sectorAround old, cur;
	old.count = 0;
	cur.count = 0;

	_remove->count = 0;
	_add->count = 0;

	getSectorAround(_user->previousSector.xPos, _user->previousSector.yPos, &old);
	getSectorAround(_user->currentSector.xPos, _user->currentSector.yPos, &cur);

	// 이전섹터 대비 신규섹터에 없는 것들.
	for (oldCount; oldCount < old.count; oldCount++)
	{
		findFlag = false;

		curCount = 0;
		for (curCount; curCount < cur.count; curCount++)
		{
			if (old.data[oldCount].xTile == cur.data[curCount].xTile &&
				old.data[oldCount].yTile == cur.data[curCount].yTile)
			{
				findFlag = true;
				break;
			}
		}
		if (!findFlag)
		{
			_remove->data[_remove->count] = old.data[oldCount];
			_remove->count++;
		}
	}

	curCount = 0;
	for (curCount; curCount < cur.count; curCount++)
	{
		findFlag = false;
		oldCount = 0;
		for (oldCount; oldCount < old.count; oldCount++)
		{
			if (old.data[oldCount].xTile == cur.data[curCount].xTile &&
				old.data[oldCount].yTile == cur.data[curCount].yTile)
			{
				findFlag = true;
				break;
			}
		}
		if (!findFlag)
		{
			_add->data[_add->count] = cur.data[curCount];
			_add->count++;
		}
	}

	return;
}

void Sector::proc_userSectorUpdate(player *_user)
{
	if (!_user) return;

	sectorAround add, remove;
	
	std::list<player*> *sectorList;
	std::list<player*>::iterator iter;
	std::list<player*>::iterator endIter;
	player *user = NULL;
	Sbuf *buf = NULL;
	getUpdateSectorAround(_user, &remove, &add);

	int xPos, yPos;
	int count = 0;

	if (remove.count != 0)
	{
		// 삭제된 구역에 플레이어가 삭제되었음을 알림.
		buf = _user->packet_removeCharacter(_user->clientID);
		for (count; count < remove.count; count++)
		{
			sendSectorOne(remove.data[count].xTile, remove.data[count].yTile, buf);
		}
		buf->Free();
		// 삭제된 구역들의 플레이어들을 삭제함을 알림
		count = 0;
		for (count; count < remove.count; count++)
		{
			xPos = remove.data[count].xTile;
			yPos = remove.data[count].yTile;
			sectorList = &fieldSector[yPos][xPos].userList;
			iter = sectorList->begin();
			endIter = sectorList->end();
			for (iter; iter != endIter; iter++)
			{
				buf = _user->packet_removeCharacter((*iter)->clientID);
				_user->sendPacket(buf);
				buf->Free();
			}
		}
	}


	if (add.count != 0)
	{
		// 추가된 구역에 플레이어가 생성됨을 알림
		buf = _user->packet_createCharacter(game_newUser_res,_user);
		count = 0;
		for (count; count < add.count; count++)
			sendSectorOne(add.data[count].xTile, add.data[count].yTile, buf, _user);
		buf->Free();


		// 플레이어 액션 상태에 따른 액션값 전송 (이동인 경우)
		if (_user->moveTime != 0  && !_user->deathFlag)
		{
			buf = _user->packet_moveRes(_user->clientID, _user->xForward, _user->zForward,_user->xDir,_user->zDir);
			count = 0;
			for (count; count < add.count; count++)
				sendSectorOne(add.data[count].xTile, add.data[count].yTile, buf, _user);
			buf->Free();
		}

		// 추가된 구역의 값을 플레이어에게 알림

		count = 0;
		for (count; count < add.count; count++)
		{
			xPos = add.data[count].xTile;
			yPos = add.data[count].yTile;
			sectorList = &fieldSector[yPos][xPos].userList;
			iter = sectorList->begin();
			endIter = sectorList->end();
			for (iter; iter != endIter; iter++)
			{
				user = (*iter);

				if (user != _user)	//  본인이 아닌경우
				{
					buf = user->packet_createCharacter(game_newUser_res,user);
					_user->sendPacket(buf);
					buf->Free();

					if(user->nowStatus == MOVE)
					{
						buf = _user->packet_moveRes(_user->clientID, _user->xForward, _user->zForward, _user->xDir, _user->zDir);
						_user->sendPacket(buf);
						buf->Free();
					}
				}
			}
		}
	}

	return;
}

void Sector::sendSectorAround(int _xSector, int _ySector, Sbuf *_buf, player *_user)
{
	if (_xSector == -1 || _ySector == -1) return;
	sectorAround around;
	getSectorAround(_xSector, _ySector, &around);
	
	int count = 0;
	for (count; count < around.count; count++)
		sendSectorOne(around.data[count].xTile, around.data[count].yTile, _buf, _user);
	
	return;
}

void Sector::sendSectorOne(int _xSector, int _ySector, Sbuf *_buf, player *_user)
{
	std::list<player*>::iterator iter = fieldSector[_ySector][_xSector].userList.begin();
	std::list<player*>::iterator endIter = fieldSector[_ySector][_xSector].userList.end();

	if (_user)
	{
		if (_user->currentSector.xPos == _xSector && _user->currentSector.yPos == _ySector)
		{
			for (iter; iter != endIter; iter++)
			{
				if ((*iter) != _user)
					(*iter)->sendPacket(_buf);
			}
			return;
		}
	}

	for (iter; iter != endIter; iter++)
		(*iter)->sendPacket(_buf);	
	return;
}

void Sector::sendBroadcast(Sbuf *_buf)
{
	int xCount = 0, yCount = 0;
	std::list<player*>::iterator iter;
	std::list<player*>::iterator endIter;
	for (yCount; yCount < ySize; yCount++)
	{
		xCount = 0;
		for (xCount; xCount < xSize; xCount++)
		{
			iter = fieldSector[yCount][xCount].userList.begin();
			endIter = fieldSector[yCount][xCount].userList.end();

			for (iter; iter != endIter; iter++)
				(*iter)->sendPacket(_buf);
			
		}
	}

	return;
}