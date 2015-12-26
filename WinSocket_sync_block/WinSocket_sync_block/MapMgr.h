#pragma  once

//////////////////////////////////////////////////////////////////////////
//1: 地图数据管理
//2: mapobject管理
//3：消息分发替代viewListTTT.h/cpp
//////////////////////////////////////////////////////////////////////////
enum MapObjEnum
{
	ENUM_MAPOBJ_PLAYER = 0,
	ENUM_MAPOBJ_MASTER  = 1,
	ENUM_MAPOBJ_DYNCMAPOBJ = 2,
};
class MapMgr
{
public:
	static int generateID(int type);
	static bool isPlayer(int id);
	static bool isMaster(int id);
	static bool isDyncMapObj(int id);
};


class Map
{

};
