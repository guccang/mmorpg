#include "stdafx.h"
#include "TcpReceiver.h"
#include "viewListTTT.h"
#include "criticalSection.h"
#include "GUGPackage.h"
#include "ioTools.h"
using namespace GUGGAME;

playerData ViewList::dataArray[100];
CriticalSection ViewList::m_lock;
dbData	ViewList::dataOFDB[100];

	 void ViewList::Init()
	{
		CriticalSection::Lock lock(m_lock);
		ZeroMemory(dataArray, sizeof(playerData) * 100);
		ZeroMemory(dataOFDB, sizeof(dbData) * 100);
		playerData *db = NULL;
		fileHandle f;
		int len = f.size();
		int cnt = len / sizeof(playerData);
		for (int i = 0; i < cnt; ++i)
		{
			db = &dataArray[i];
			f.read((char**)&db, sizeof(playerData));
			printf("name:%s %s onLine:%d %d %d %d %d\n", db->name, db->pwd, db->onLine, db->hp, db->mp, db->def, db->flag);
		}
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
			 return GUGGAME::OK;

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
			db->id = sock;
			db->x = x;
			db->z = z;
			db->dir = dir;
			db->flag = 1;
		}

		CreateObj c;
		c.id = (short)sock;
		c.type = 0;
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
		att.id = (short)sock;
		SendStruct((SOCKET)sock, att, 1);

		NotifyCreate(sock,1, x, z, dir);
		return 0;
	 }
	  void ViewList::remove(int socke)
	{
		CriticalSection::Lock lock(m_lock);
		DeleteObj del;
		del.id = socke;
		for (int i = 0; i < 100; ++i)
		{
			if (dataArray[i].id == socke)
			{
				dataArray[i].onLine = 0;
			}
			else
			{
				if (dataArray[i].onLine == 1)
				{
					SendStruct((SOCKET)dataArray[i].id,del,1);
				}
			}
		}
	}

	  bool ViewList::find(const char* name, playerData** freeData)
	  {
		  for (int i = 0; i < 100; ++i)
		  {
			  if (strlen(name)>0 && strcmp(dataArray[i].name, name) == 0)
			  {
				  *freeData = &dataArray[i];
				  return true;
			  }
		  }
		  return false;
	  }
	  bool ViewList::find(int sock,playerData** freeData)
	  {
		  for (int i = 0; i < 100; ++i)
		  {
			  if (dataArray[i].id == sock)
			  {
				  *freeData = &dataArray[i];
				  return true;
			  }
		  }
		  return false;
	  }

	bool ViewList::get(playerData** freeData)
	{
		for (int i = 0; i < 100; ++i)
		{
			if (dataArray[i].flag == 0)
			{
				*freeData = &dataArray[i];
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

	void ViewList::NotifyCreate(int sock, int type, int x, int z, int dir)
	{
		CriticalSection::Lock lock(m_lock);
		playerData *p = NULL;
		bool bfind = find(sock, &p);
		if (false == bfind) return;

		for (int i = 0; i < 100; ++i)
		{
			if (dataArray[i].onLine == 0) continue;

			// newplayer to other
			if (p->id != dataArray[i].id)
			{
				CreateObj c;
				c.id = sock;
				c.type = type;
				c.x = x;
				c.z = z;
				c.dir = dir;
				c.hp = p->hp;
				strcpy_s(c.name, p->name);
				SendStruct((SOCKET)dataArray[i].id, c, 1);
			}
		}

		for (int i = 0; i < 100; ++i)
		{
			if (dataArray[i].onLine==0 || dataArray[i].id==sock) continue;

			// other player to new Player
			CreateObj cc;
			cc.id = dataArray[i].id;
			cc.type = type; // player
			cc.x = dataArray[i].x;
			cc.z = dataArray[i].z;
			cc.dir = dataArray[i].dir;
			cc.hp = dataArray[i].hp;
			strcpy_s(cc.name, dataArray[i].name);
			SendStruct((SOCKET)sock, cc, 1);
		}

		NotifyAttrInit(sock, p->hp, p->mp, p->def);

	}

	  void ViewList::NotifyWalk(int sock,int x,int y,int dir)
	  {
		  CriticalSection::Lock lock(m_lock);
		  playerData *p=NULL;
		  bool bfind = find(sock, &p);
		  if (false == bfind) return;
		  p->x = x; p->z = y; 
		  if (dir!=4)
			p->dir = dir;
		
		  for (int i = 0; i < 100;++i)
		  {
			  if (p->id != dataArray[i].id && dataArray[i].onLine==1)
			  {
				  Walk w;
				  w.sock = sock;
				  w.x = p->x;
				  w.z = p->z;
				  w.dir = p->dir;
				  SendStruct((SOCKET)dataArray[i].id, w, 1);
			  }
		  }
	  }

	  void ViewList::NotifyFight(int attack,int target, short action)
	  {
		  CriticalSection::Lock lock(m_lock);
		  for (int i = 0; i < 100; ++i)
		  {
			  if (dataArray[i].onLine == 1)
			  {
				  Fight f;
				  f.attackerID = attack;
				  f.targetID = target;
				  f.action = action;
				  SendStruct((SOCKET)dataArray[i].id, f, 1);
			  }
		  }
	  }

	  void ViewList::NotifyAttrChg(int id ,short type, short num)
	  {
		  CriticalSection::Lock lock(m_lock);
		  for (int i = 0; i < 100; ++i)
		  {
			  if (dataArray[i].onLine == 0) continue;

			  // notify hp chang
			  AttrChg att;
			  att.id = id;
			  att.type = type;
			  att.num = num;
			  SendStruct(dataArray[i].id, att, 1);
		  }
	  }

	  void ViewList::NotifyAttrInit(int id,int hp,int mp,int def)
	  {
		  CriticalSection::Lock lock(m_lock);
		  for (int i = 0; i < 100; ++i)
		  {
			  if (dataArray[i].onLine == 0 ) continue;

			  // notify self hp chang
			  Attr att;
			  att.id = id;
			  att.hp = hp;
			  att.mp = mp;
			  att.def = def;
			  SendStruct((SOCKET)dataArray[i].id, att, 1);
		  }

		  for (int i = 0; i < 100; ++i)
		  {
			  // notify other to you
			  if (dataArray[i].onLine == 0) continue;
			  if (dataArray[i].id == id) continue;

			  Attr att;
			  att.id = dataArray[i].id;
			  att.hp = dataArray[i].hp;
			  att.mp = dataArray[i].mp;
			  att.def = dataArray[i].def;
			  SendStruct((SOCKET)id, att, 1);
		  }
	  }


	  void ViewList::NotifyJump(int id, int x, int z, char dir)
	  {
		  CriticalSection::Lock lock(m_lock);
		  for (int i = 0; i < 100; ++i)
		  {
			  if (dataArray[i].onLine == 0) continue;
			  JumpInMap jim;
			  jim.id = id;
			  jim.x = x;
			  jim.z = z;
			  jim.dir = dir;
 			  SendStruct((SOCKET)(dataArray[i].id),jim,1);
		  }
	  }


	  void ViewList::attrchg(int id, int type, int num)
	  {
		  CriticalSection::Lock lock(m_lock);
		  for (int i = 0; i < 100; ++i)
		  {
			  if (dataArray[i].onLine == 0) continue;
			  if (dataArray[i].id != id) continue;

			  if (type == HP)
			  {
				  dataArray[i].hp += num;
				  NotifyAttrChg(id,HP,num);
			  }
			  if (type == MP)
			  {
				  dataArray[i].mp += num;
				  NotifyAttrChg(id,MP,num);
			  }

			  if (dataArray[i].hp <= 0)
			  {
				  dataArray[i].hp = 1500;
				  dataArray[i].mp = 500;
				  dataArray[i].def = 100;
				  NotifyJump(id,9,9,0);
				  NotifyAttrInit(id,dataArray[i].hp, dataArray[i].mp,dataArray[i].def);
			  }
			  break;
		  }
	  }