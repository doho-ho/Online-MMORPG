#pragma once

class GAMEServer;
class player;

struct tileNode
{
	std::list<player*> userList;
};

struct sector
{
	std::list<player*>userList;
};

struct tileInfo
{
	int xTile;
	int yTile;
};

struct sectorInfo
{
	int xPos = -1;
	int yPos = -1;
};

struct sectorAround
{
	int count;
	tileInfo data[9];
};

class Map
{
private:
	bool			**field;
	tileNode		**tile;

	int xSize, ySize;
	int maxPosXSize, maxPosYSize;
	int positionPerTileXSize, positionPerTileYSize;

private:
	void initMap(const char *_configData,const  int _fileSize);

public:
	Map(const char *_configData, const int _fileSize);
	~Map();
	
	void transPosToTile(float _xPos, float _yPos, int *_xTile, int *_yTile);

	void setTile(player *_user, int _xTile, int _yTile);
	void removeTile(player *_user);
	bool checkField(int _xTile, int _yTile);
	tileNode* getTile(int _xTile, int _yTile);

	friend GAMEServer;
};

class Sector
{
private:
	sector		**fieldSector;

	int xSize, ySize;
	int tileSizePerSectorXSize, tileSizePerSectorYSize;

private:
	void initSector(const char *_configData, const int _fileSize);

public:
	Sector(const char *_configData, const int _fileSize);
	~Sector();

	void transTileToSector(int _xTile, int _yTile, int *_xSector, int *_ySector);
	bool addSector(player *_user);
	bool removeSector(player *_user);
	bool updateSector(player *_user);

	void getSectorAround(int _xSector, int _ySector, sectorAround *_around);
	void getUpdateSectorAround(player *_user, sectorAround *_remove, sectorAround *_add);

	void proc_userSectorUpdate(player *_user);

	void sendSectorAround(int _xSector, int _ySector, Sbuf *_buf, player *_user = NULL);
	void sendSectorOne(int _xSector, int _ySector, Sbuf *_buf, player *_user = NULL);
	void sendBroadcast(Sbuf *_buf);

};