#pragma  once
#include "criticalSection.h"
 namespace GUGGAME
 {
	 class TCPClientEvent;
	 class IOCPEvent;
	 struct pathNode
	 {
		 short x;
		 short z;
	 };
 }
class hatch;
enum ENUM_ATTR
{
	HP = 0,
	MP = 1,
};
class mapObj
{

};
struct playerData : public mapObj
{
	int onLine;
	int id;
	int sock;
	short x;
	short z;
	char dir;
	int hp;
	int mp;
	int attack;
	int def;
	char name[16];
	char pwd[32];
	int flag; // 1 use
};

struct masterData : public mapObj
{
	int id;
	int spawnid;
	short x;
	short z;
	char dir;
	int hp;
	int mp;
	int attack;
	int def;
	char name[16];
	int flag;
	int dead;
	int state;
	playerData* target;
	int blockcnt;
	int astart;
	pathNode path[50];
	int pathNum;
};

class ViewList
{
	class GUGGAME::TCPClientEvent;
public:
	static float fightLen;
	static void Init();
	
	static int regist(const char* name,const char *pwd);
	static int add(int sock, short x,short y, char dir,const char*name,const char*pwd);
	static void remove(int socke,int errorCode);
	static void attrchg(int id, int type, int num);
	static void NotifyJump(int id,short x,short y,char dir);
	static void NotifyAttrInit(int id,int hp,int mp,int def);
	static void NotifyMasterAttrInit(int id);
	static void NotifyCreate(int id, int type, short x, short y, char dir);
	static void NotifyWalk(int id,short x,short y,char dir);
	static void NotifyFight(int id, int target,short action);
	static void NotifyAttrChg(int id,int attr, int num);
	static void NotifyMasterCreate(int id, short x, short y, char dir);
	static bool getNearestPlayer(masterData* master,playerData** freeData);
	static bool getNearestMaster(masterData* master, masterData** freeData);
	static bool find(int id, playerData** freeData);
	static bool get(playerData** freeData);
	static bool get(masterData** freeData);
	static bool find(const char* name, playerData** freeData);
	static bool find(int id, masterData** freeData);
	static int getID(int sock);
	static int getSock(int id);
	static void Update(float t);
	static void masterInit();
	static bool check(SOCKET socket);
	static void PushEvent(TCPClientEvent* e);
	static void PushEvent(IOCPEvent* e);
	//static bool get(dbData** freeData);
	static char mapData[512][512];
private:
	static playerData playerArray[100];
	static masterData masterArray[500];
	static CriticalSection m_lock;
	static hatch hatchArray[10];
	typedef gugQueue<TCPClientEvent> clientQueue;
	typedef gugQueue<IOCPEvent> iocpQueue;
	static clientQueue _tcpClientEventQueue;
	static iocpQueue _iocpEventQueue;

public:
	static HANDLE _clientEvent;
};



// generate master.
class hatch
{
public:
	hatch();
	void init(int id,short x,short z,int radius,int cnt,char* name);
	int _MapObjectID;
	short _x;
	short _z;
	int _radius;
	int _cnt;
	int _spawnCnt;
	int  _flag;
	char _name[16];
};