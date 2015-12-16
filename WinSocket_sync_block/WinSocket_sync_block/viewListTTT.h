#pragma  once
#include "criticalSection.h"
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

struct dbData
{
	int flag; // 1 use
	bool onLine;
	char name[16];
	char pwd[32];
	int hp;
	int mp;
	int attack;
	int def;
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
	static void NotifyCreate(int sock, int type, int x, int y, int dir);
	static void NotifyWalk(int sock,int x,int y,int dir);
	static void NotifyFight(int sock, int target,short action);
	static void NotifyAttrChg(int sock,short attr, short num);
	static bool find(int id, playerData** freeData);
	static bool get(playerData** freeData);
	static bool find(const char* name, playerData** freeData);
	//static bool get(dbData** freeData);
private:
	static playerData dataArray[100];
	static CriticalSection m_lock;
	static dbData dataOFDB[100];
};