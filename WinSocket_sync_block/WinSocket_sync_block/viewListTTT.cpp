#include "stdafx.h"
#include "TcpReceiver.h"
#include "viewListTTT.h"
#include "GUGPackage.h"
#include "ioTools.h"
#include "MapMgr.h"
#include "Event.h"
#include<stdio.h>
#include<stdlib.h>
#include <math.h>

#include "AStart.h"
#define randomDW(x) (-rand()%x + (short)(x*0.5))
#define random(x) (rand()%x)
//#define max(a,b) ((a)>(b)?(a):(b))

short bornX = 64;
short bornZ = 38;
short AIUpdateTime = 7;

using namespace GUGGAME;
void act_2_utf8(const char* src, size_t src_len, char* des, size_t des_len);

direction Direction[8] =
{
	{ 0, 1 },
	{ 1, 1 },
	{ 1, 0 },
	{ 1, -1},
	{ 0, -1 },
	{ -1, -1 },
	{ -1, 0 },
	{ -1, 1 },
};
direction DirectionVertical[8] =
{
	{ 1, 0 },
	{ 1, -1 },
	{ 0, -1 },
	{ -1, -1 },
	{ -1, 0 },
	{ -1, 1 },
	{ 0, 1 },
	{ 1, 1 },
};

enum ENUM_CREATURE_ID
{
	CREATURE_HERO=0,
	CREATURE_OTHERPLAYER=1,
	CREATURE_MASTER1=2,
	CREATURE_MASTER2=3,
	CREATURE_MASTERBOSS=4,
	CREATURE_DYNCMAPOBJ = 5,
};

enum BLOCK
{
	BK_0 = 0,
	BK_ISLAND = 1,
	BK_S = 1<<1,
	BK_T = 1<<2,
	BK_C = 1<<3,
	BK_MASTER = 1<<6,
	BK_PLAYER = 1<<7,
};


char* ViewList::mapDataEx = nullptr;
ViewList::clientQueue ViewList::_tcpClientEventQueue;
ViewList::iocpQueue	ViewList::_iocpEventQueue;
HANDLE ViewList::_clientEvent;
playerData ViewList::playerArray[100];
masterData ViewList::masterArray[500];
hatch ViewList::hatchArray[10];
CriticalSection ViewList::m_lock;
float ViewList::fightLen = 1.0f;

short viewLen = 200; // 距离大于viewLen*viewLen的时候就不发送移动包
short mapW = 0;
short mapH = 0;

hatch::hatch()
{
	_MapObjectID = 1;
	_x = 0;
	_z = 0;
	_radius = 10;
	_cnt = 5;
	_spawnCnt = 0;
	_flag = 0;
	_name[0] = 0;
}


bool isOnLine(playerData &player)
{
	return player.onLine == 1;
}

bool needSend(playerData* player)
{
	if (NULL == player) return false;

	if (isOnLine(*player)==false) return false;

	if (player->mapid <= 0) return false;

	return true;
}

inline int getIndex(short x, short z)
{
	return z + x*mapH;
	//return z*mapW + x;
}

void clearMoveFlag(short x, short z)
{
	ViewList::mapDataEx[getIndex(x, z)] &= ~BK_MASTER;
}
void clearDyncBlockFlag(short x, short z)
{
	ViewList::mapDataEx[getIndex(x, z)] &= ~BK_MASTER;
}
bool checkPos(short x, short z)
{
	if (x < 0) return false;
	if (x > mapW) return false;
	if (z < 0) return false;
	if (z > mapH) return false;
	return true;
}

bool canMove(short x, short z)
{
	return false==ViewList::isBlock(x,z);
}


BLOCK getBlock(short x, short z)
{
	if (checkPos(x,z))
	{
		return (BLOCK)ViewList::mapDataEx[getIndex(x, z)];
	}
	return BK_ISLAND;
}


void setBlock(short x, short z, BLOCK bk)
{
	if (checkPos(x, z) && getBlock(x, z) != BK_ISLAND)
	{
		ViewList::mapDataEx[getIndex(x, z)] |= bk;
	}
}

void clearBlock(short x, short z, BLOCK bk)
{
	ViewList::mapDataEx[getIndex(x, z)] &= ~bk;
}


bool Move(short x, short z, short x1, short z1)
{
	if (!checkPos(x,z) && !checkPos(x1,z1))
	{
		printf("move params error \n why...................");
		return false;
	}
	if (x == x1&&z == z1) return true;

	if (canMove(x1, z1))
	{
		//ViewList::mapData[x][z] = 0;
		//ViewList::mapData[x1][z1] = 1;
		//printf("(%d,%d)=(%d,%d)\n",x,z, x1, z1);
		ViewList::mapDataEx[getIndex(x, z)] &= ~BK_MASTER;
		ViewList::mapDataEx[getIndex(x1, z1)] |= BK_MASTER;
		return true;
	}
	else
	{

	}
	return false;
}


void hatch::init(short id, short x, short z, int radius, int cnt,char *name)
{
	_MapObjectID = id;
	_x = x;
	_z = z;
	_radius = radius;
	_cnt = cnt;
	_flag = 1;
	strcpy_s(_name, name);
}
float calcFightLen(masterData* master)
{
	return ViewList::fightLen + master->fightRadius;
}

char ViewList::isBlock(short x, short z)
{
	if (checkPos(x, z))
	{
		int index = getIndex(x, z);
		if (mapDataEx[index] & BK_ISLAND || mapDataEx[index] & BK_MASTER)
			return true;
		else
			return false;
	}
	return BK_ISLAND;
}
void ViewList::PushEvent(IOCPEvent* e)
{
	_iocpEventQueue.PushEvent(e);
}
void ViewList::PushEvent(TCPClientEvent* e)
{
	_tcpClientEventQueue.PushEvent(e);
}
void ViewList::masterInit()
{/*
 出生：64，37
 刷怪1：50,23
 刷怪2：31,23
 BOSS：14,12
 */
	//hatchArray[0].init(1, 20, 0, 100, 100,"川桑");
	hatchArray[1].init(CREATURE_MASTER1, 50, 23, 7, 12, "暴暴");
	hatchArray[2].init(CREATURE_MASTER2, 31, 23, 7, 12, "龙龙");
	hatchArray[3].init(CREATURE_MASTERBOSS, 14, 12, 3, 1, "亮亮");
	//hatchArray[4].init(5, -10, -10, 100, 100, "小明");
}
void ViewList::mapInit()
{
	fileHandle f("./config/11.mapo");
	int len = f.size();
	unsigned short version = 0;
	int sizeMap = 0;
	f.readByte((char*)&version, sizeof(unsigned short));
	f.readByte((char*)&mapW, sizeof(unsigned short));
	f.readByte((char*)&mapH, sizeof(unsigned short));
	f.readByte((char*)&sizeMap,sizeof(int));
	printf("len:%d v:%d w:%d h:%d dataSize:%d\n", len, version, mapW, mapH,sizeMap);

	int sizeData = mapW*mapH;
	mapDataEx = new char[sizeData];
	ZeroMemory(mapDataEx, sizeData);
	short *data = new short[sizeData];
	f.readByte((char*)data, sizeData*sizeof(short));
	for (short i = 0; i < mapW; ++i)// x
	{
		for (short j = 0; j < mapH; ++j) // y
		{
			int index = getIndex(i, j); // y*w + x 
			mapDataEx[index] = (char)data[index];
			//printf("%d ",data[index]);
		}
		//printf("\n");
	}
}

void QueueTest()
{
	queue<eventBase> queue;
	for (int i = 0; i < 100; ++i)
	{
		if (i % 2 == 0)
			queue.push_back(new eventBase(i));
		else
			queue.push_front(new eventBase(i));
	}
	eventBase* e;
	while (e = queue.front())
	{
		queue.pop_front();
		//printf("%d ", e->_id);
		delete e;
	}
	//printf("pop end %d", (int)queue.empty());
}

	 void ViewList::Init()
	{
		CriticalSection::Lock lock(m_lock);
		ZeroMemory(playerArray, sizeof(playerData) * 100);
		playerData *db = NULL;
		fileHandle f;
		int len = f.size();
		int cnt = len / sizeof(playerData);
		for (int i = 0; i < cnt; ++i)
		{
			db = &playerArray[i];
			f.read((char**)&db, sizeof(playerData));
			printf("name:%s %s onLine:%d %d %d %d %d\n", db->name, db->pwd, db->onLine, db->hp, db->mp, db->def, db->flag);
		}
		mapInit();
		masterInit();
		_clientEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		//QueueTest();
	 }

	
	

	 int ViewList::regist(const char* name, const char *pwd)
	 {
		 if (strlen(name) == 0 || strlen(pwd) == 0)
		 {
			 return GUGGAME::ERROR_NAME_PWD_NULL;
		 }

		 CriticalSection::Lock lock(m_lock);
		 playerData *db = NULL;
		 bool beFind = find(name, &db);
		 if (beFind)
			 return GUGGAME::ERROR_HAD_REGIST;

		 bool beGet = get(&db);
		 if (true == beGet)
		 {
			 db->flag = 1;
			 db->hp = 1000;
			 db->mp = 1000;
			 db->def = 10;
			 db->onLine = 0;
			 strcpy_s(db->name, name);
			 strcpy_s(db->pwd,pwd);
		 }

		 fileHandle f;
		 f.write((char*)db, sizeof(playerData));
		 f.close();

		 return GUGGAME::OK;
	 }

	 int ViewList::add(int sock, short x, short z, char dir,const char*name,const char* pwd)
	{
		CriticalSection::Lock lock(m_lock);
		
		playerData *db = NULL;
		bool beGet = find(name, &db);
		if (false == beGet )
		{
			printf("not find or has onLine %s %s", name,pwd);
			return  GUGGAME::ERROR_LOGIN_NOT_REGIST;;
		}
		if (isOnLine(*db))
		{
			return GUGGAME::ERROR_LOGIN_ONLINE;
		}

		db->onLine = 1;

		if (true == beGet)
		{
			db->id = MapMgr::generateID(0);
			db->sock = sock;
			db->x = x;
			db->z = z;
			db->dir = dir;
			db->flag = 1;
			db->mapid = 0;
			ZeroMemory(&db->dyncBlock, sizeof(dyncNodeObj) * 30);
			db->dyncBlockTime = 0;
			db->dyncBlockCnt = 0;
		}
		return 0;
	 }

	 char* ViewList::getName(int id)
	 {
		 for (int i = 0; i < 100; ++i)
		 {
			 if (playerArray[i].id == id)
				 return playerArray[i].name;
		 }
		 return 0;
	 }
	 int ViewList::getSock(int id)
	 {
		 for (int i = 0; i < 100; ++i)
		 {
			 if (playerArray[i].id == id)
				 return playerArray[i].sock;
		 }
		 return 0;
	 }

	 int ViewList::getID(SOCKET sock)
	 {
		 for (int i = 0; i < 100; ++i)
		 {
			 if (playerArray[i].sock == sock && isOnLine(playerArray[i]))
				 return playerArray[i].id;
		 }
		 return 0;
	 }
	  void ViewList::remove(SOCKET socke,int errorCode)
	{
		CriticalSection::Lock lock(m_lock);
		DeleteObj del;
		del.id = getID(socke);
		errorCode = errorCode;
	/*	if (errorCode == WSAENOTSOCK)
		{
		}
		else if (errorCode == WSAECONNABORTED)
		{
		}
		else
		{
			// TODO Fixe
			printf("why... fixed this please.");
			return;
		}
	*/
		for (int i = 0; i < 100; ++i)
		{
			if (!isOnLine(playerArray[i])) continue;;

			if (playerArray[i].sock == socke)
			{
				//printf("Vliew::remove self sock=%d error=%d", socke, errorCode);

				playerArray[i].onLine = 0;
				playerArray[i].sock = INVALID_SOCKET;
				playerArray[i].mapid = 0;
			}
			else
			{
				//printf("Vliew::remove Not Self sock=%d error=%d", socke, errorCode);

				SendStruct((SOCKET)playerArray[i].sock,del,1);
			}
		}
	}
	  int ViewList::EnterMap(SOCKET sock,short mapid)
	  {
		  playerData* db = nullptr;
		  int id = getID(sock);
		  bool bfind = find(id, &db);
		  if (false == bfind)
		  {
			  printf("EnterMap not find %d player",sock);
			  return GUGGAME::ERROR_ENTER_MAP_NOT_LOGIN;
		  }

		  db->mapid = mapid;

		  CreateObj c;
		  c.id = db->id;
		  c.type = CREATURE_HERO;
		  c.x = bornX;
		  c.z = bornZ;
		  c.dir = 0;
		  c.hp = db->hp;
		  strcpy_s(c.name, db->name);
		  SendStruct((SOCKET)sock, c, 1);

		  Attr att;
		  att.hp = db->hp;
		  att.mp = db->mp;
		  att.def = db->def;
		  att.id = db->id;
		  SendStruct((SOCKET)sock, att, 1);

		  NotifyMapInfo(db->id);
		  NotifyCreate(db->id, CREATURE_OTHERPLAYER, bornX, bornZ, 0);
		  return 0;
	  }

	  bool ViewList::find(const char* name, playerData** freeData)
	  {
		  for (int i = 0; i < 100; ++i)
		  {
			  if (strlen(name)>0 && strcmp(playerArray[i].name, name) == 0)
			  {
				  *freeData = &playerArray[i];
				  return true;
			  }
		  }
		  return false;
	  }

	  bool ViewList::find(int id, masterData** freeData)
	  {
		  for (int i = 0; i < 500; ++i)
		  {
			  if (masterArray[i].id == id)
			  {
				  *freeData = &masterArray[i];
				  return true;
			  }
		  }
		  return false;
	  }
	  bool ViewList::find(int id,playerData** freeData)
	  {
		  for (int i = 0; i < 100; ++i)
		  {
			  if (playerArray[i].id == id)
			  {
				  *freeData = &playerArray[i];
				  return true;
			  }
		  }
		  return false;
	  }

	  int distance(short x, short z, short x1, short z1)
	  {
		  int nx = abs(x - x1);
		  int nz = abs(z - z1);
		  //return abs(nx) + abs(nz) + 0.3f;
		  //return float(nx*nx + nz*nz);
		  return max(nx, nz);
	  }

	  double distance3D(short x, short z, short x1, short z1)
	  {
		  int nx = (x - x1);
		  int nz = (z - z1);

		  return sqrt(nx*nx + nz*nz);
	  }

	  int len(masterData* master, masterData* target)
	  {
		  if (master == NULL || target == NULL)
		  {
			  printf("calc len failed");
			  return -1;
		  }
		  return distance(target->x, target->z, master->x, master->z);
	  }

	  
	  int len(masterData* master,playerData* target)
	  {
		  if (master == NULL || target == NULL)
		  {
			  printf("calc len failed");
			  return -1;
		  }
		  return distance(target->x, target->z, master->x, master->z);
	  }
	  bool ViewList::getNearestMaster(masterData* master, masterData** freeData)
	  {
		  *freeData = NULL;
		  int tmp = 99999999;
		  for (int i = 0; i < 500; ++i)
		  {
			  if (masterArray[i].flag == 0 || masterArray[i].dead>0) continue;
			  int theLen = len(master, &masterArray[i]);
			  if (theLen>0 && tmp > theLen && theLen <= calcFightLen(master))
			  {
				  tmp = theLen;
				  *freeData = &masterArray[i];
			  }
		  }

		  if (*freeData != NULL) return true;

		  return false;
	  }
	  bool ViewList::getNearestPlayer(masterData* master,playerData** freeData)
	  {
		  *freeData = NULL;
		  int tmp = 99999999;
		  for (int i = 0; i < 100; ++i)
		  {
			  if (!isOnLine(playerArray[i])) continue;
			  int theLen = len(master, &playerArray[i]);
			  if (theLen>0&&tmp > theLen && theLen<=master->radius)
			  {
				  tmp = theLen;
				  *freeData = &playerArray[i];
			  }
		  }

		  if (*freeData != NULL) return true;

		  return false;
	  }

	bool ViewList::get(playerData** freeData)
	{
		for (int i = 0; i < 100; ++i)
		{
			if (playerArray[i].flag == 0)
			{
				*freeData = &playerArray[i];
				return true;
			}
		}
		return false;
	}
	/*
	bool ViewList::get(dbData** freeData)
	{
		for (int i = 0; i < 100; ++i)
		{
			if (dataOFDB[i].flag == 0)
			{
				*freeData = &dataOFDB[i];
				return true;
			}
		}
		return false;
	}
	*/


	void ViewList::NotifyCreate(int id, int type, short x, short z, char dir)
	{
		type = type;
		CriticalSection::Lock lock(m_lock);
		playerData *p = NULL;
		bool bfind = false;
		bfind = find(id, &p);
		if (false == bfind) return;
		
		for (int i = 0; i < 100; ++i)
		{
			//if (!isOnLine(playerArray[i])) continue;
			if (needSend(&playerArray[i])==false) continue;

			// newplayer to other
			if (p->id != playerArray[i].id)
			{
				CreateObj c;
				c.id = id;
				c.type = CREATURE_OTHERPLAYER;
				c.x = x;
				c.z = z;
				c.dir = dir;
				c.hp = p->hp;
				strcpy_s(c.name, p->name);
				SendStruct((SOCKET)playerArray[i].sock, c, 1);
			}
		}

		for (int i = 0; i < 100; ++i)
		{
			if (!needSend(&playerArray[i]) || playerArray[i].id == id) continue;

			// other player to new Player
			CreateObj cc;
			cc.id = playerArray[i].id;
			cc.type = CREATURE_OTHERPLAYER; // player
			cc.x = playerArray[i].x;
			cc.z = playerArray[i].z;
			cc.dir = playerArray[i].dir;
			cc.hp = playerArray[i].hp;
			strcpy_s(cc.name, playerArray[i].name);
			SendStruct((SOCKET)p->sock, cc, 1);
		}

		for (int i = 0; i < 500; ++i)
		{
			if (masterArray[i].flag == 0)continue;

			CreateObj cc;
			cc.id = masterArray[i].id;
			cc.type = masterArray[i].masterType;
			cc.x = masterArray[i].x;
			cc.z = masterArray[i].z;
			cc.dir = masterArray[i].dir;
			cc.hp = masterArray[i].hp;
			strcpy_s(cc.name, masterArray[i].name);
			SendStruct((SOCKET)p->sock, cc, 1);
		}

		NotifyAttrInit(p->id, p->hp, p->mp, p->def);

	}

	  void ViewList::NotifyWalk(int id,short x,short y,char dir)
	  {
		  CriticalSection::Lock lock(m_lock);
		  playerData *p=NULL;
		  masterData *master = NULL;
		  bool bfind = false;
		  bool isPlayer = MapMgr::isPlayer(id);
		  if (isPlayer)
		  {
			  bfind = find(id, &p);
			  if (false == bfind) return;
			  p->x = x; p->z = y;
			  p->dir = dir;
		  }
		  else
		  {
			  bfind = find(id, &master);
			  if (false == bfind) return;
			  master->x = x;
			  master->z = y;
			  master->dir = dir;
		  }
		  
		  for (int i = 0; i < 100;++i)
		  {
			  if (!needSend(&playerArray[i]))continue;
			  if (playerArray[i].mapid <= 0)continue;

			  if (isPlayer&&p->id != playerArray[i].id)
			  {
				  Walk w;
				  w.id = id;
				  w.x = p->x;
				  w.z = p->z;
				  w.dir = p->dir;
				  SendStruct((SOCKET)playerArray[i].sock, w, 1);
			  }
			  else if (!isPlayer)
			  {
				  if (len(master, &playerArray[i])>viewLen*viewLen)
				 	  continue;

				  Walk w;
				  w.id = id;
				  w.x = master->x;
				  w.z = master->z;
				  w.dir = master->dir;
				  SendStruct((SOCKET)playerArray[i].sock, w, 1);
			  }
		  }
	  }

	  void ViewList::NotifyFight(int attack,int target, short action)
	  {
		  CriticalSection::Lock lock(m_lock);
		  for (int i = 0; i < 100; ++i)
		  {
			  if (needSend(&playerArray[i]))
			  {
				  Fight f;
				  f.attackerID = attack;
				  f.targetID = target;
				  f.action = action;
				  SendStruct((SOCKET)playerArray[i].sock, f, 1);
			  }
		  }
	  }


	  void ViewList::NotifyDyncMapObj(int playerID)
	  {
		  playerData* player = nullptr;
		  bool bfind = find(playerID, &player);
		  if (false == bfind) return;
		  for (int i = 0; i < 100; ++i)
		  {
			  if (!needSend(&playerArray[i])) continue;

			  for (int j = 0; j < player->dyncBlockCnt;++j)
			 {
				 CreateObj c;
				 c.id = player->dyncBlock[j].id;
				 if (!MapMgr::isDyncMapObj(c.id))
				 {
					 printf("send dyncmapobj id error %d",c.id);
				 }
				 c.type = CREATURE_DYNCMAPOBJ;
				 c.x = player->dyncBlock[j].x;
				 c.z = player->dyncBlock[j].z; 
				 c.dir = player->dyncBlock[j].dir;
				 c.hp = 100000;
				 SendStruct(playerArray[i].sock, c, 1);
			 }
		  }
	  }

	  void ViewList::NotifyMapInfo(int id)
	  {
		  CriticalSection::Lock lock(m_lock);
		  if (strcmp("guccang", getName(id)) != 0)
			  return;
		  SOCKET sock = getSock(id);
		  for (short i = 0; i < mapW; ++i)
		  {
			  MapInfo info;
			  info.x = i;
			  info.z = -1;
			  info.d = 1;
			  SendStruct(sock, info, 1);

			  info.x = i;
			  info.z = mapH;
			  info.d = 1;
			  SendStruct(sock, info, 1);
			 // Sleep(1);
		  }
		  for (short i = 0; i < mapH; ++i)
		  {

			  MapInfo info;
			  info.x = -1;
			  info.z = i;
			  info.d = 1;
			  SendStruct(sock, info, 1);

			  info.x = mapW;
			  info.z = i;
			  info.d = 1;
			  SendStruct(sock, info, 1);
			  Sleep(1);
		  }
			  int cnt = 0;
			  for (int i = 0; i < 100; ++i)
			  {
				  if (!needSend(&playerArray[i]) || playerArray[i].id != id) continue;

				  for (short x = 0; x < mapW; ++x)
				  {
					  for (short z = 0; z < mapH; ++z)
					  {
						  if ((mapDataEx[getIndex(x,z)] & BK_ISLAND) == 1)
						  {
							  MapInfo info;
							  info.x = x;
							  info.z = z;
							  info.d = BK_ISLAND;
							  SendStruct(sock, info, 1);
							 // cnt++;
							 // Sleep(1);
						  }
					  }
				  }
			  }
			  printf("send block info:%d",cnt);
	  }

	  void ViewList::NotifyAttrChg(int id ,int  type, int num,short delay)
	  {
		  CriticalSection::Lock lock(m_lock);
		  //if (MapMgr::isPlayer(id))
		  {
			  for (int i = 0; i < 100; ++i)
			  {
				  if (!needSend(&playerArray[i])) continue;

				  // notify hp chang
				  AttrChg att;
				  att.id = id;
				  att.type = (short)type;
				  att.num = (short)num;
				  att.delay = delay;
				  SendStruct(playerArray[i].sock, att, 1);
			  }
		  }
		  //else // master
		  {

		  }
	  }

	  void ViewList::NotifyMasterAttrInit(int id)
	  {
		  CriticalSection::Lock lock(m_lock);
		  masterData* master = NULL;
		  bool bFind = find(id, &master);
		  if (false == bFind) return;

		  for (int i = 0; i < 100; ++i)
		  {
			  if (!needSend(&playerArray[i])) continue;

			  Attr att;
			  att.id = id;
			  att.hp = master->hp;
			  att.mp = master->mp;
			  att.def = master->def;
			  SendStruct(playerArray[i].sock, att, 1);
		  }
	  }

	  void ViewList::NotifyAttrInit(int id,int hp,int mp,int def)
	  {
		  CriticalSection::Lock lock(m_lock);
		  for (int i = 0; i < 100; ++i)
		  {
			  if (!needSend(&playerArray[i])) continue;

			  // notify self hp chang
			  Attr att;
			  att.id = id;
			  att.hp = hp;
			  att.mp = mp;
			  att.def = def;
			  SendStruct((SOCKET)playerArray[i].sock, att, 1);
		  }

		  for (int i = 0; i < 100; ++i)
		  {
			  // notify other to you
			  if (!needSend(&playerArray[i])) continue;
			  if (playerArray[i].id == id) continue;

			  Attr att;
			  att.id = playerArray[i].id;
			  att.hp = playerArray[i].hp;
			  att.mp = playerArray[i].mp;
			  att.def = playerArray[i].def;
			  SendStruct((SOCKET)getSock(id), att, 1);
		  }

		  for (int i = 0; i < 500; ++i)
		  {
			  if (masterArray[i].flag == 0) continue;

			  // master attr init
			  Attr att;
			  att.id = masterArray[i].id;
			  att.hp = masterArray[i].hp;
			  att.mp = masterArray[i].mp;
			  att.def = masterArray[i].def;
			  SendStruct((SOCKET)getSock(id), att, 1);
		  }
	  }


	  void ViewList::NotifyJump(int id, short x, short z, char dir)
	  {
		  CriticalSection::Lock lock(m_lock);
		  for (int i = 0; i < 100; ++i)
		  {
			  if (!needSend(&playerArray[i])) continue;
			  JumpInMap jim;
			  jim.id = id;
			  jim.x = x;
			  jim.z = z;
			  jim.dir = dir;
 			  SendStruct((SOCKET)(playerArray[i].sock),jim,1);
		  }
	  }

	  void ViewList::areoDamage(short x, short z, char radius, int demage,int attackID,short action)
	  {
		  CriticalSection::Lock lock(m_lock);
		  int dis = (int)radius;
		  int num = 0;
		  playerData* player = NULL;
		  bool find = ViewList::find(attackID, &player);
		  if (!find)
		  {
			  printf("areo damage not find player....why....\n");
			  return;
		  }
		  for (int i = 0; i < 500; ++i)
		  {
			  if (masterArray[i].flag != 1 || masterArray[i].dead>0) continue;
			  if (distance3D(x, z, masterArray[i].x, masterArray[i].z) <= dis)
			  {
				  num++;
				  masterArray[i].target = player;
				  NotifyFight(attackID, masterArray[i].id, action);
				  attrchg(masterArray[i].id, HP, demage);
			  }
		  }
		  printf("Area Demage master num:%d\n",num);
		  for (int i = 0; i < 100; ++i)
		  {
			  if (!needSend(&playerArray[i])) continue;
			  if (attackID == playerArray[i].id) continue;

			  if (distance(x, z, playerArray[i].x, playerArray[i].z) <= dis)
			  {
				  NotifyFight(attackID, playerArray[i].id, action);
				  attrchg(playerArray[i].id, HP, demage);
			  }
		  }
	  }

	  int setDyncBlockData(playerData* player,int index,short x,short z)
	  {
		  if (!checkPos(x, z) || getBlock(x,z)==BK_ISLAND || index>=30) return index;
		  player->dyncBlock[index].id = MapMgr::generateID(ENUM_MAPOBJ_DYNCMAPOBJ);
		  player->dyncBlock[index].x = x;
		  player->dyncBlock[index].z = z;
		  player->dyncBlock[index].dir = player->dir;
		  setBlock(x, z, BK_MASTER);
		  player->dyncBlockCnt++;
		  index++;
		  return index;
	  }
	
	  void ViewList::addDyncBlock(int id)
	  {
		  CriticalSection::Lock lock(m_lock);
		  // sync player data ， sync dync block
		  printf("addDyncBlock %d",id);
		  playerData* player = NULL;
		  bool bfind = find(id,&player);
		  if (false == bfind) return;
		  if (!needSend(player)) return;
		  if (player->dir < 0 || player->dir>8) return;
		  if (player->dyncBlockCnt > 0) return;


		   char r = 5;
		   char len = 13;
		   int blockTime = 10000; // ms
		   direction dirV = DirectionVertical[player->dir];
		   direction dir = Direction[player->dir];
		  short x1 = (short)ceil(player->x + 0.7f*r*dir.x);
		  short z1 = (short)ceil(player->z + 0.7f*r*dir.z);
		  short x = 0, z = 0;
		  int index = 0;
		  index = setDyncBlockData(player,index, x1, z1);
		  index = setDyncBlockData(player,index, x1 + 1, z1);
		  for (short i = 1; i<len; ++i)
		  {
			  //right
			  x = x1 + dirV.x*i;
			  z = z1 + dirV.z*i;
			  //if (getBlock(x,z) == BK_ISLAND) continue;
			  index = setDyncBlockData(player, index, x, z);
			  index = setDyncBlockData(player, index, x+1, z);

			  //left
			  x = x1 - dirV.x*i;
			  z = z1 - dirV.z*i;
			  index = setDyncBlockData(player, index, x, z);
			  index = setDyncBlockData(player, index, x + 1, z);
		  }
		  player->dyncBlockTime =  blockTime;
		  NotifyDyncMapObj(player->id);
	  }

	  void ViewList::attrchg(int id, int type, int num,short delay)
	  {
		  CriticalSection::Lock lock(m_lock);
		  bool isPlayer = MapMgr::isPlayer(id);
		  if (isPlayer)
		  {
			  playerData *player = NULL;
			  bool bfind = find(id, &player);
			  if (false == bfind) return;

			  if (type == HP)
			  {
				  if (player->shield > 0) // shiled
				  {
					  player->shield += num;
					  if (player->shield < 0) player->shield = 0;
					  NotifyAttrChg(id, SHIELD, player->shield,delay);
				  }
				  else
				  {
					  player->hp += num;
					  if (player->hp < 0) player->hp = 0;
					  NotifyAttrChg(id, HP, num,delay);
				  }
			 }
			  if (type == MP)
			  {
				  player->mp += num;
				  if (player->mp < 0) player->mp = 0;
				  NotifyAttrChg(id, MP, num,delay);
			  }

			  if (type == SHIELD)
			  {
				  NotifyAttrChg(id, SHIELD, player->shield,delay);
			  }

			  if (player->hp <= 0)
			  {
				  player->hp = 150000;
				  player->mp = 150000;
				  player->def = 100;
				  clearMoveFlag(player->x, player->z);
				  player->x = bornX;
				  player->z = bornZ;
				  NotifyJump(id, player->x,player->z, player->dir);
				  NotifyAttrInit(id, player->hp, player->mp, player->def);
			  }
		  }
		  else // master
		  {
			  masterData *master = NULL;
			  bool bfind = find(id, &master);
			  if (false == bfind) return;

			  if (type == HP)
			  {
				  master->hp += num;
				  NotifyAttrChg(id, HP, num,delay);
			  }
			  if (type == MP)
			  {
				  master->mp += num;
				  NotifyAttrChg(id, MP, num,delay);
			  }

			  if (master->hp <= 0)
			  {
				  master->hp = 600;
				  master->mp = 200;
				  master->def = 50;
				  master->dead = 1;
				  clearMoveFlag(master->x, master->z);
			  }
		  }
	  }

	  void InitMaster(masterData* master,int spawnId,hatch* sp)
	  {
		  int radius = sp->_radius;
		  master->masterType = sp->_MapObjectID;
		  master->spawnid = spawnId;
		  if (master->dead==0)
			 master->id = MapMgr::generateID(1);
		  master->radius = radius;
		  master->hp = 500*master->masterType;
		  master->mp = 100;
		  master->def = 10; 
		  master->dir = 0;
		  master->dead = 0;
		  master->x = sp->_x + (short)random(radius);
		  master->z = sp->_z + (short)random(radius);
		  master->bx = master->x;
		  master->bz = master->z;
		  act_2_utf8(sp->_name, 5, master->name, 16);
		  master->flag = 1;
		  master->state = 0;
		  master->target = NULL;
		  master->blockcnt = 0;
		  master->astart = 0;
		  master->pathNum = 0;
		  master->fightRadius = 0;// (short)(master->masterType*0.5f);
	  }

	  bool ViewList::get(masterData** freeData)
	  {
		  for (int i = 0; i < 500; ++i)
		  {
			  if (masterArray[i].flag == 0)
			  {
				  *freeData = &masterArray[i];
				  return true;
			  }
		  }
		  return false;
	  }
	  void resetMaster(masterData* master)
	  {
		  master->blockcnt = 0;
		  master->astart = 0;
		  master->pathNum = 0;
		 // master->target = NULL;
	  }
	  void walkToTarget(masterData* master)
	  {
		  if (master->target == NULL || !needSend((master->target)))
		  {
			  master->target = NULL;
			  master->state = 0;
			  return;
		  }

		  int theLne = len(master, master->target);
		  if (theLne<=calcFightLen(master))
		  {
			  resetMaster(master);
			  master->state = 2;
			  return;
		  }
		  if (master->pathNum>master->astart)
		  {
			  int index = master->astart;
			  short sx = master->path[index].x;
			  short sz = master->path[index].z;
			  if (abs(master->x - sx) > 5 ||
				  abs(master->z - sz) > 5)
			  {
				  printf("some error on path find...");
			  }
			  if (Move(master->x, master->z, sx, sz))
				  {
					  master->x = sx;
					  master->z = sz;
					  master->astart++;
					  ViewList::NotifyWalk(master->id, master->x, master->z, master->dir);
				  }
			 else
				  {
					  if (master->blockcnt++>5)
					  {
						  resetMaster(master);
						  master->state = 0;
						  //printf("astart failed\n");
					  }
				  }
			  return;
			  }

	
		  int x = master->target->x - master->x;
		  int z = master->target->z - master->z;

		  short posX = master->x, posZ = master->z;
		  char dir = -1;
		  short detaX = 0, detaZ = 0;
		  if (x >= ViewList::fightLen)
		  {
			  detaX = 1;
			  dir  = 0;
		  }
		  else if (x <= -ViewList::fightLen)
		  {
			  detaX = -1;
			  dir = 1;
		  }
		  if (z >= ViewList::fightLen)
		  {
			  detaZ = 1;
			  dir = 2;
		  }
		  else if (z <= -ViewList::fightLen)
		  {
			  detaZ = -1;
			  dir = 3;
		  }

		  if (dir < 0)
		  {
			  printf("not important walkToTarget。");
			  master->state = 0;
		  }
		  // 或许永远到达不了。因为没有真正的路径算法。隔堵墙就sb了哈哈哈。
		  // 简单的解决走不了问题。
		  // master沿着player朝向一直移动，乳沟阻碍了，就绕过去。绕5次还不行就，进入初始状态了。
		  //
		  posX = master->x + detaX;
		  posZ = master->z + detaZ;
		
		  if (Move(master->x,master->z,posX,posZ))
		  {
			  if (abs(master->x - posX) > 5 ||
				  abs(master->z - posZ) > 5)
			  {
				  printf("some error on path find...");
			  }
			  master->x = posX;
			  master->z = posZ;
			  master->dir = dir;
			  ViewList::NotifyWalk(master->id, master->x, master->z, master->dir);
		  }
		  else
		  {
			  pathNode path[50];
			  ZeroMemory(path, sizeof(int) * 50);
			  int pathNum =  findPath((master->x), (master->z), (master->target->x), (master->target->z), path);
			  if (pathNum < 10)
			  {
				  master->astart = 0;
				  master->pathNum = pathNum;
				  for (int i = 0; i < pathNum; ++i)
				  {
					  master->path[i].x = path[i].x;
					  master->path[i].z = path[i].z;
				  }
			  }
			  else
			  {
				  resetMaster(master);
				  master->state = 0;
			  }
		  }
		 
		
	  }

	  void AI(masterData* master)
	  {
		  if (master->dead>0) return;
		  char randNum = 0;
		  switch (master->state)
		  {
		  case 0: // stand
		  {
			  randNum = random(5);
			  if (randNum == 0)
				  master->state = 0;
			  else if (randNum == 2)
				  master->state = 2;
			  else master->state = 1;
		  }break;
		  case 1: // walk
		  {
			  randNum = random(5);
			  short x = master->x, z=master->z;
			  short bx = master->bx, bz = master->bz;
			  int dis = distance(x, z, bx, bz);
			  short detax=0, detaz=0;
			  if (dis > master->radius)
			  {
				  detax = (bx - x)>0?1:-1;
				  detaz = (bz - z)>0?1:-1;
			  }
			  else
			  {
				 // detax = (x - bx)>0?1:-1;
				 // detaz = (z - bz)>0?1:-1;
				  if (0 == randNum)
					  detax =  1;
				  else if (1 == randNum)
					  detax =  -1;
				  else if (2 == randNum)
					   detaz =  -1;
				  else if (3 == randNum)
					  detaz =  1;
			  }
			  x = master->x + detax;
			  z = master->z + detaz;
			  /*
			
			  */
			  if (4==randNum) master->state = 2;
			  

			  if (randNum<4)
			  {
				  if (Move(master->x, master->z, x, z)) // map data check
				  {
					  if (abs(master->x - x) > 5 ||
						  abs(master->z - z) > 5)
					  {
						  printf("some error on path find...");
					  }
					  master->x = x;
					  master->z = z;
					  ViewList::NotifyWalk(master->id, master->x, master->z, randNum);
				  }
			  }
		  }break;
		  case 2: // fight
		  {
			  // find a player
			  // walk and fight
			  if (master->target == NULL)
			  {
				  playerData* target = NULL;
				  bool getFinght = ViewList::getNearestPlayer(master, &target);
				  if (getFinght)
				  {
					  master->target = target;
					  //ViewList::NotifyFight(master->id, target->id, 0);
					  //ViewList::attrchg(target->id, HP, -random(10));
				  }
				  else
				  {
					  master->state = 0;
				  }
			  }
			  else
			  {
				  if (!needSend((master->target)))
				  {
					  master->target = NULL;
					  master->state = 1; // walk
				  }
				  else
				  {
					  int tmp = len(master, master->target);
					  if (tmp > 0)
					  {
						  if (tmp <=calcFightLen(master))
						  {
							  ViewList::NotifyFight(master->id, master->target->id, 0);
							  ViewList::attrchg(master->target->id, HP, -random(10));
						  }
						  else
						  {
							  master->state = 3;
						  }

					  }
				  }
			  }
		  }break;
		  case 3:
		  {
			  if (master->target == NULL)
			  {
				  printf("why master-target is null");
				  master->state = 0; // stand
				  resetMaster(master);
				  master->target = NULL;
				  break;
			  }
			  if (!needSend((master->target)))
			  {
				  master->state = 0; // stand
				  resetMaster(master);
				  master->target = NULL;
				  break;
			  }
			  walkToTarget(master);
		  }break;
		  case 4: // fight with master
		  {
			  /*	  masterData* target = NULL;
					bool getFinght = ViewList::getNearestMaster(master, &target);
					if (getFinght)
					{
					master->target = target;
					ViewList::NotifyFight(master->id, target->id, 0);
					ViewList::attrchg(target->id, HP, -random(100));
					}
					else
					{
					master->state = 0;
					}*/
		  }
		  default:
			  break;
		  }
	  }

	  
	  bool ViewList::check(SOCKET socket)
	  {
		  for (int i = 0; i < 100; ++i)
		  {
			  if (playerArray[i].sock == socket && ::isOnLine(playerArray[i]))
				  return true;
		  }
		  return false;
	  }

	  void ViewList::NotifyRemoveObj(int id)
	  {
		  DeleteObj del;
		  del.id = id;
		  for (int i = 0; i < 100; ++i)
		  {
			  if (!needSend(&playerArray[i]))continue;

			  SendStruct(playerArray[i].sock, del, 1);
		  }
	  }

	  void ViewList::Update(float t)
	  {
		
		  _iocpEventQueue.OnAccept((void*)&playerArray);
		  _tcpClientEventQueue.OnAccept((void*)&playerArray);
	
	
		  for (int i = 0; i < 10; ++i)
		  {
			  if (hatchArray[i]._flag == 0) continue;

			  if (hatchArray[i]._spawnCnt < hatchArray[i]._cnt)
			  {
				  hatchArray[i]._spawnCnt++;
				  masterData* master = NULL;
				  bool beGet = get(&master);
				  if (beGet)
				  {
					  InitMaster(master,i,&hatchArray[i]);
				  }
				  continue;
			  }
		  }
		  static float theTime = 0;
		  float els = t - theTime;
		  if (els< AIUpdateTime)
		  {
		  }
		  else
		  {
			  theTime = t;
			  CriticalSection::Lock lock(m_lock);
			  // master update
			  for (int i = 0; i < 500; ++i)
			  {
				  if (masterArray[i].flag != 1) continue;
				  if (masterArray[i].dead > 0)
				  {
					  if (masterArray[i].dead++<masterArray[i].masterType*10)
						  continue;
					  int spawnid = masterArray[i].spawnid;
					  InitMaster(&masterArray[i], spawnid ,&hatchArray[spawnid]);
					  masterArray[i].hp = 700*masterArray[i].masterType;
					  masterArray[i].mp = 300;
					  masterArray[i].def = 90;
					  masterArray[i].dead = 0;
					  NotifyMasterAttrInit(masterArray[i].id);
					  NotifyJump(masterArray[i].id, masterArray[i].x, masterArray[i].z, 0);
				  }
				  else
				  {
					  masterData *master = &masterArray[i];
					  // AI Fight Moving
					  // find the player
					  // fight the palyer
					  // moving around.
					  AI(master);
				  }

				  // playerUpdate
				  for (int i = 0; i < 100;++i)
				  {
					  if (playerArray[i].flag == 0) continue;

					  if (playerArray[i].dyncBlockCnt > 0)
					  {
						  if ((playerArray[i].dyncBlockTime -= els) < 0)
						  {
							  // delete dyncBlock
							  for (int bcnt = 0; bcnt < playerArray[i].dyncBlockCnt;++bcnt)
							  {
								  dyncNodeObj &dno = playerArray[i].dyncBlock[bcnt];
								  if (!MapMgr::isDyncMapObj(dno.id))
								  {
									  printf("why~~~~~~....dyncNodeObj Id Error:%d", dno.id);
									  continue;
								  }
								  clearDyncBlockFlag(dno.x, dno.z);
								  NotifyRemoveObj(playerArray[i].dyncBlock[bcnt].id);
							  }
							  playerArray[i].dyncBlockCnt = 0;
							  playerArray[i].dyncBlockTime = 0;
							  ZeroMemory(&playerArray[i].dyncBlock, sizeof(dyncNodeObj) * 30);
						  }
					  }
					
				  }
				 
			  }
		  }
		
	  }


	  void utf8_2_act(const char* src, size_t src_len, char* des, size_t des_len)
	  {
		  *des = 0;
		  wchar_t temp[1024];
		  int wl = ::MultiByteToWideChar(CP_UTF8, 0, src, (int)src_len, temp, 1024);
		  if (wl > 0)
		  {
			  int ml = ::WideCharToMultiByte(CP_ACP, 0, temp, wl, des, (int)des_len, 0, false);
			  des[ml] = 0;
		  }
	  }
	  void act_2_utf8(const char* src, size_t src_len, char* des, size_t des_len)
	  {
		  *des = 0;
		  wchar_t temp[1024];
		  int wl = ::MultiByteToWideChar(CP_ACP, 0, src, (int)src_len, temp, 1024);
		  if (wl > 0)
		  {
			  int ml = ::WideCharToMultiByte(CP_UTF8, 0, temp, wl, des, (int)des_len, 0, false);
			  des[ml] = 0;
		  }
	  }