#include "stdafx.h"
#include "TcpReceiver.h"
#include "viewListTTT.h"
#include "GUGPackage.h"
#include "ioTools.h"
#include "MapMgr.h"
#include "Event.h"
#include<stdio.h>
#include<stdlib.h>

#include "AStart.h"
#define randomDW(x) (-rand()%x + (short)(x*0.5))
#define random(x) (rand()%x)

using namespace GUGGAME;
void act_2_utf8(const char* src, size_t src_len, char* des, size_t des_len);
enum MAPOBJ
{
	HERO=0,
	OTHERPLAYER=1,
	MASTER=2,
};


char ViewList::mapData[512][512];
ViewList::clientQueue ViewList::_tcpClientEventQueue;
ViewList::iocpQueue	ViewList::_iocpEventQueue;
HANDLE ViewList::_clientEvent;
playerData ViewList::playerArray[100];
masterData ViewList::masterArray[500];
hatch ViewList::hatchArray[10];
CriticalSection ViewList::m_lock;
float ViewList::fightLen = 2.0f;

short mapSize = 512;
short mapCoor = (short)(mapSize * 0.5)-1;
short minCoor = -mapCoor;
short maxCoor = mapCoor;
short viewLen = 200; // 距离大于viewLen*viewLen的时候就不发送移动包

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

void hatch::init(int id, short x, short z, int radius, int cnt,char *name)
{
	_MapObjectID = id;
	_x = x;
	_z = z;
	_radius = radius;
	_cnt = cnt;
	_flag = 1;
	strcpy_s(_name, name);
}
float calcFightLen()
{
	return ViewList::fightLen*ViewList::fightLen * 2;
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
{
	hatchArray[0].init(1, -20, 0, 100, 100,"川桑");
	hatchArray[1].init(2, 0, 0, 20, 100, "暴暴");
	hatchArray[2].init(3, 20, 0, 100, 100, "龙龙");
	hatchArray[3].init(4, -20, -20, 100, 100, "亮亮");
	hatchArray[4].init(5, -10, -10, 100, 100, "小明");
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
		ZeroMemory(mapData, sizeof(char) * mapSize * mapSize);
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

		masterInit();
		_clientEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		//QueueTest();
	 }

	 bool canMove(int x, int z)
	 {
		 return ViewList::mapData[x][z] == 0;
	 }

	 short clampMap(short x)
	 {
		 if (x < minCoor) x = minCoor;
		 if (x > maxCoor) x = maxCoor;
		 return x+mapCoor;
	 }

	 void coord(short &nx,short &nz)
	 {
		 nx = clampMap(nx);
		 nz = clampMap(nz);
	 }
	 void deCoord(short &nx, short &nz)
	 {
		 nx  -= mapCoor;
		 nz  -= mapCoor;
	 }
	 void clearMoveFlag(short x, short z)
	 {
		 coord(x, z);
		 ViewList::mapData[x][z] = 0;
	 }
	 bool Move(short x, short z,short x1,short z1)
	 {
		 if (x == x1&&z == z1) return true;
		 coord(x, z);
		 coord(x1, z1);
		 if (canMove(x1, z1))
		 {
			 ViewList::mapData[x][z] = 0;
			 ViewList::mapData[x1][z1] = 1;
			 //printf("(%d,%d)=(%d,%d)\n",x,z, x1, z1);
			 return true;
		 }
		 else
		 {
			
		 }
		 return false;
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
			 db->mp = 100;
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
		}

		CreateObj c;
		c.id = db->id;
		c.type = HERO;
		c.x = x;
		c.z = z;
		c.dir = dir;
		c.hp = db->hp;
		strcpy_s(c.name, db->name);
		SendStruct((SOCKET)sock, c, 1);

		Attr att;
		att.hp = db->hp;
		att.mp = db->mp;
		att.def = db->def;
		att.id = db->id;
		SendStruct((SOCKET)sock, att, 1);

		NotifyCreate(db->id,OTHERPLAYER, x, z, dir);
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
			 if (playerArray[i].sock == sock)
				 return playerArray[i].id;
		 }
		 return 0;
	 }
	  void ViewList::remove(SOCKET socke,int errorCode)
	{
		CriticalSection::Lock lock(m_lock);
		DeleteObj del;
		del.id = getID(socke);
		if (errorCode == WSAENOTSOCK)
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

		for (int i = 0; i < 100; ++i)
		{
			if (!isOnLine(playerArray[i])) continue;;

			if (playerArray[i].sock == socke)
			{
				playerArray[i].onLine = 0;
				playerArray[i].sock = INVALID_SOCKET;
			}
			else
			{
				SendStruct((SOCKET)playerArray[i].sock,del,1);
			}
		}
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

	  float distance(short x, short z, short x1, short z1)
	  {
		  int nx = x - x1;
		  int nz = z - z1;
		  return float(nx*nx + nz*nz);
	  }

	  float len(masterData* master, masterData* target)
	  {
		  if (master == NULL || target == NULL)
		  {
			  printf("calc len failed");
			  return -1;
		  }
		  return distance(target->x, target->z, master->x, master->z);
	  }

	  
	  float len(masterData* master,playerData* target)
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
		  float tmp = 99999999.0f;
		  for (int i = 0; i < 500; ++i)
		  {
			  if (masterArray[i].flag == 0 || masterArray[i].dead>0) continue;
			  float theLen = len(master, &masterArray[i]);
			  if (theLen>0 && tmp > theLen && theLen <= calcFightLen())
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
		  float tmp = 99999999.0f;
		  for (int i = 0; i < 100; ++i)
		  {
			  if (!isOnLine(playerArray[i])) continue;
			  float theLen = len(master, &playerArray[i]);
			  if (theLen>0&&tmp > theLen && theLen<=calcFightLen())
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
			if (!isOnLine(playerArray[i])) continue;

			// newplayer to other
			if (p->id != playerArray[i].id)
			{
				CreateObj c;
				c.id = id;
				c.type = OTHERPLAYER;
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
			if (!isOnLine(playerArray[i]) || playerArray[i].id == id) continue;

			// other player to new Player
			CreateObj cc;
			cc.id = playerArray[i].id;
			cc.type = OTHERPLAYER; // player
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
			cc.type = MASTER;
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
			  if (dir != 4)
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
			  if (!isOnLine(playerArray[i]))continue;

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
			  if (isOnLine(playerArray[i]))
			  {
				  Fight f;
				  f.attackerID = attack;
				  f.targetID = target;
				  f.action = action;
				  SendStruct((SOCKET)playerArray[i].sock, f, 1);
			  }
		  }
	  }

	  void ViewList::NotifyAttrChg(int id ,int  type, int num)
	  {
		  CriticalSection::Lock lock(m_lock);
		  //if (MapMgr::isPlayer(id))
		  {
			  for (int i = 0; i < 100; ++i)
			  {
				  if (!isOnLine(playerArray[i])) continue;

				  // notify hp chang
				  AttrChg att;
				  att.id = id;
				  att.type = (short)type;
				  att.num = (short)num;
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
			  if (!isOnLine(playerArray[i])) continue;

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
			  if (!isOnLine(playerArray[i])) continue;

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
			  if (!isOnLine(playerArray[i])) continue;
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
			  if (!isOnLine(playerArray[i])) continue;
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
		  float dis = (float)radius*radius;
		  int num = 0;
		  playerData* player = NULL;
		  bool find = ViewList::find(attackID, &player);
		  if (!find)
		  {
			  printf("areo damage not find player....why....");
			  return;
		  }
		  for (int i = 0; i < 500; ++i)
		  {
			  if (masterArray[i].flag != 1 || masterArray[i].dead>0) continue;
			  if (distance(x, z, masterArray[i].x, masterArray[i].z) <= dis)
			  {
				  num++;
				  masterArray[i].target = player;
				  NotifyFight(attackID, masterArray[i].id, action);
				  attrchg(masterArray[i].id, HP, demage);
			  }
		  }
		  printf("master num:%d\n",num);
		  for (int i = 0; i < 100; ++i)
		  {
			  if (!isOnLine(playerArray[i])) continue;
			  if (attackID == playerArray[i].id) continue;

			  if (distance(x, z, playerArray[i].x, playerArray[i].z) <= dis)
			  {
				  NotifyFight(attackID, playerArray[i].id, action);
				  attrchg(playerArray[i].id, HP, demage);
			  }
		  }
	  }
	
	  void ViewList::attrchg(int id, int type, int num)
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
				  player->hp += num;
				  if (player->hp < 0) player->hp = 0;
				  NotifyAttrChg(id, HP, num);
			  }
			  if (type == MP)
			  {
				  player->mp += num;
				  if (player->mp < 0) player->mp = 0;
				  NotifyAttrChg(id, MP, num);
			  }

			  if (player->hp <= 0)
			  {
				  player->hp = 150000;
				  player->mp = 150000;
				  player->def = 100;
				  clearMoveFlag(player->x, player->z);
				  player->x = randomDW(10);
				  player->z = randomDW(10);
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
				  NotifyAttrChg(id, HP, num);
			  }
			  if (type == MP)
			  {
				  master->mp += num;
				  NotifyAttrChg(id, MP, num);
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
		  master->spawnid = spawnId;
		  if (master->dead==0)
			 master->id = MapMgr::generateID(1);
		  master->radius = radius;
		  master->hp = 300;
		  master->mp = 100;
		  master->def = 10; 
		  master->dir = 0;
		  master->dead = 0;
		  master->x = sp->_x + (short)randomDW(radius);
		  master->z = sp->_z+(short)randomDW(radius);
		  master->bx = master->x;
		  master->bz = master->z;
		  act_2_utf8(sp->_name, 5, master->name, 16);
		  master->flag = 1;
		  master->state = 0;
		  master->target = NULL;
		  master->blockcnt = 0;
		  master->astart = 0;
		  master->pathNum = 0;
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
		  if (master->target == NULL || !isOnLine(*(master->target)))
		  {
			  master->target = NULL;
			  master->state = 0;
			  return;
		  }

		  float theLne = len(master, master->target);
		  if (theLne<calcFightLen())
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
			  master->x = posX;
			  master->z = posZ;
			  master->dir = dir;
			  ViewList::NotifyWalk(master->id, master->x, master->z, master->dir);
		  }
		  else
		  {
			  pathNode path[512];
			  ZeroMemory(path, sizeof(int) * 512);
			  int pathNum =  findPath(clampMap(master->x), clampMap(master->z), clampMap(master->target->x), clampMap(master->target->z), ViewList::mapData, path);
			  if (pathNum < 50)
			  {
				  master->astart = 0;
				  master->pathNum = pathNum;
				  for (int i = 0; i < pathNum; ++i)
				  {
					  master->path[i].x = path[i].x;
					  master->path[i].z = path[i].z;
					  deCoord(master->path[i].x,master->path[i].z);
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
			  float dis = distance(x, z, bx, bz);
			  short detax=0, detaz=0;
			  if (dis > master->radius*master->radius)
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
					  ViewList::NotifyFight(master->id, target->id, 0);
					  ViewList::attrchg(target->id, HP, -random(10));
				  }
				  else
				  {
					  master->state = 0;
				  }
			  }
			  else
			  {
				  if (!isOnLine(*(master->target)))
				  {
					  master->target = NULL;
					  master->state = 1; // walk
				  }
				  else
				  {
					  float tmp = len(master, master->target);
					  if (tmp > 0)
					  {
						  if (tmp <=calcFightLen())
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
				  master->target = NULL;
				  break;
			  }
			  if (!isOnLine(*(master->target)))
			  {
				  master->state = 0; // stand
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

		  static float theTime = t;
		  if (t - theTime < 7)
		  {
		  }
		  else
		  {
			  theTime = t;
			  CriticalSection::Lock lock(m_lock);
			  for (int i = 0; i < 500; ++i)
			  {
				  if (masterArray[i].flag != 1) continue;
				  if (masterArray[i].dead > 0)
				  {
					  if (masterArray[i].dead++<5)
						  continue;
					  int spawnid = masterArray[i].spawnid;
					  InitMaster(&masterArray[i], spawnid ,&hatchArray[spawnid]);
					  masterArray[i].hp = 600;
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