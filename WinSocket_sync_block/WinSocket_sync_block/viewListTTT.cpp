#include "stdafx.h"
#include "TcpReceiver.h"
#include "viewListTTT.h"
#include "criticalSection.h"
#include "GUGPackage.h"
#include "ioTools.h"
#include "MapMgr.h"
#include<stdio.h>
#include<stdlib.h>
#define random(x) (rand()%x)
using namespace GUGGAME;
void act_2_utf8(const char* src, size_t src_len, char* des, size_t des_len);
enum MAPOBJ
{
	HERO=0,
	OTHERPLAYER=1,
	MASTER=2,
};

playerData ViewList::playerArray[100];
masterData ViewList::masterArray[500];
hatch ViewList::hatchArray[10];
CriticalSection ViewList::m_lock;

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

void hatch::init(int id, int x, int z, int radius, int cnt,char *name)
{
	_MapObjectID = id;
	_x = x;
	_z = z;
	_radius = radius;
	_cnt = cnt;
	_flag = 1;
	strcpy_s(_name, name);
}

void ViewList::masterInit()
{
	hatchArray[0].init(1, -20, 0, 10, 5,"¥®…£");
	hatchArray[1].init(1, 0, 0, 10, 5, "±©±©");
	hatchArray[2].init(1, 20, 0, 10, 5, "¡˙¡˙");
	hatchArray[3].init(1, -20, -20, 10, 5, "¡¡¡¡");

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

		masterInit();
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

	 int ViewList::add(int sock, int x, int z, int dir,const char*name,const char* pwd)
	{
		CriticalSection::Lock lock(m_lock);

		playerData *db = NULL;
		bool beGet = find(name, &db);
		if (false == beGet )
		{
			printf("not find or has onLine %s", name);
			return  GUGGAME::ERROR_LOGIN_NOT_REGIST;;
		}
		if (db->onLine==1)
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

	 int ViewList::getID(int sock)
	 {
		 for (int i = 0; i < 100; ++i)
		 {
			 if (playerArray[i].sock == sock)
				 return playerArray[i].id;
		 }
		 return 0;
	 }
	  void ViewList::remove(int socke)
	{
		CriticalSection::Lock lock(m_lock);
		DeleteObj del;
		del.id = getID(socke);
		for (int i = 0; i < 100; ++i)
		{
			if (playerArray[i].sock == socke)
			{
				playerArray[i].onLine = 0;
			}
			else
			{
				if (playerArray[i].onLine == 1)
				{
					SendStruct((SOCKET)playerArray[i].sock,del,1);
				}
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

	void ViewList::NotifyCreate(int id, int type, int x, int z, int dir)
	{
		CriticalSection::Lock lock(m_lock);
		playerData *p = NULL;
		bool bfind = false;
		bfind = find(id, &p);
		if (false == bfind) return;
		
		for (int i = 0; i < 100; ++i)
		{
			if (playerArray[i].onLine == 0) continue;

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
			if (playerArray[i].onLine==0 || playerArray[i].id==id) continue;

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

	  void ViewList::NotifyWalk(int id,int x,int y,int dir)
	  {
		  CriticalSection::Lock lock(m_lock);
		  playerData *p=NULL;
		  bool bfind = find(id, &p);
		  if (false == bfind) return;
		  p->x = x; p->z = y; 
		  if (dir!=4)
			p->dir = dir;
		
		  for (int i = 0; i < 100;++i)
		  {
			  if (p->id != playerArray[i].id && playerArray[i].onLine==1)
			  {
				  Walk w;
				  w.id = id;
				  w.x = p->x;
				  w.z = p->z;
				  w.dir = p->dir;
				  SendStruct((SOCKET)playerArray[i].sock, w, 1);
			  }
		  }
	  }

	  void ViewList::NotifyFight(int attack,int target, short action)
	  {
		  CriticalSection::Lock lock(m_lock);
		  for (int i = 0; i < 100; ++i)
		  {
			  if (playerArray[i].onLine == 1)
			  {
				  Fight f;
				  f.attackerID = attack;
				  f.targetID = target;
				  f.action = action;
				  SendStruct((SOCKET)playerArray[i].sock, f, 1);
			  }
		  }
	  }

	  void ViewList::NotifyAttrChg(int id ,short type, short num)
	  {
		  CriticalSection::Lock lock(m_lock);
		  //if (MapMgr::isPlayer(id))
		  {
			  for (int i = 0; i < 100; ++i)
			  {
				  if (playerArray[i].onLine == 0) continue;

				  // notify hp chang
				  AttrChg att;
				  att.id = id;
				  att.type = type;
				  att.num = num;
				  SendStruct(playerArray[i].sock, att, 1);
			  }
		  }
		  //else // master
		  {

		  }
	  }

	  void ViewList::NotifyMasterAttrInit(int id)
	  {
		  masterData* master = NULL;
		  bool bFind = find(id, &master);
		  if (false == bFind) return;

		  for (int i = 0; i < 100; ++i)
		  {
			  if (playerArray[i].onLine==0) continue;

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
			  if (playerArray[i].onLine == 0 ) continue;

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
			  if (playerArray[i].onLine == 0) continue;
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


	  void ViewList::NotifyJump(int id, int x, int z, char dir)
	  {
		  CriticalSection::Lock lock(m_lock);
		  for (int i = 0; i < 100; ++i)
		  {
			  if (playerArray[i].onLine == 0) continue;
			  JumpInMap jim;
			  jim.id = id;
			  jim.x = x;
			  jim.z = z;
			  jim.dir = dir;
 			  SendStruct((SOCKET)(playerArray[i].sock),jim,1);
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
				  player->hp = 1500;
				  player->mp = 500;
				  player->def = 100;
				  NotifyJump(id, 9, 9, 0);
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
			  }
		  }
	  }

	  void InitMaster(masterData* master,int spawnId,hatch* sp)
	  {
		  int radius = sp->_radius;
		  master->spawnid = spawnId;
		  if (master->dead==0)
			 master->id = MapMgr::generateID(1);
		  master->hp = 300;
		  master->mp = 100;
		  master->def = 10; 
		  master->dir = 0;
		  master->dead = 0;
		  master->x = sp->_x+random(radius);
		  master->z = sp->_z+random(radius);
		  act_2_utf8(sp->_name, 5, master->name, 16);
		  master->flag = 1;
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

	  void ViewList::Update(float t)
	  {
		  for (int i = 0; i < 10; ++i)
		  {
			  if (hatchArray[i]._flag == 0) continue;

			  if (hatchArray[i]._spawnCnt++ < hatchArray[i]._cnt)
			  {
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
		  if (t - theTime < 50)
		  {
		  }
		  else
		  {
			  theTime = t;

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

				  masterData *master = &masterArray[i];
				  // AI Fight Moving
				  // find the player
				  // fight the palyer
				  // moving around.
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