#pragma  once

//////////////////////////////////////////////////////////////////////////
//1: ��ͼ���ݹ���
//2: mapobject����
//3����Ϣ�ַ����viewListTTT.h/cpp
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
