#pragma  once
#include "criticalSection.h"
class hatch;
enum ENUM_ATTR
{
	HP = 0,
	MP = 1,
};

struct playerData
{
	int onLine;
	int id;
	int sock;
	int x;
	int z;
	int dir;
	int hp;
	int mp;
	int attack;
	int def;
	char name[16];
	char pwd[32];
	int flag; // 1 use
};

struct masterData
{
	int id;
	int spawnid;
	int x;
	int z;
	int dir;
	int hp;
	int mp;
	int attack;
	int def;
	char name[16];
	int flag;
	int dead;
};

class ViewList
{

public:
	static void Init();
	
	static int regist(const char* name,const char *pwd);
	static int add(int sock, int x, int y, int dir,const char*name,const char*pwd);
	static void remove(int socke);
	static void attrchg(int id, int type, int num);
	static void NotifyJump(int id,int x,int y,char dir);
	static void NotifyAttrInit(int id,int hp,int mp,int def);
	static void NotifyMasterAttrInit(int id);
	static void NotifyCreate(int id, int type, int x, int y, int dir);
	static void NotifyWalk(int id,int x,int y,int dir);
	static void NotifyFight(int id, int target,short action);
	static void NotifyAttrChg(int id,short attr, short num);
	static void NotifyMasterCreate(int id, int x, int y, int dir);
	static bool find(int id, playerData** freeData);
	static bool get(playerData** freeData);
	static bool get(masterData** freeData);
	static bool find(const char* name, playerData** freeData);
	static bool find(int id, masterData** freeData);
	static int getID(int sock);
	static int getSock(int id);
	static void Update(float t);
	static void masterInit();
	//static bool get(dbData** freeData);
private:
	static playerData playerArray[100];
	static masterData masterArray[500];
	static CriticalSection m_lock;
	static hatch hatchArray[10];
};



// generate master.
class hatch
{
public:
	hatch();
	void init(int id,int x,int z,int radius,int cnt,char* name);
	int _MapObjectID;
	int _x;
	int _z;
	int _radius;
	int _cnt;
	int _spawnCnt;
	int  _flag;
	char _name[16];
};