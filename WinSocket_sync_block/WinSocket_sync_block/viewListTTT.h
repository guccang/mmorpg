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
	 struct direction
	 {
		 short x;
		 short z;
	 };
	 struct dyncNodeObj
	 {
		 int id;
		 short x;
		 short z;
		 char dir;
	 };
	 enum ENUM_DIRECTION
	 {
		 UP,
		 UP_RIGHT,
		 RIGHT,
		 RIGHT_DOWN,
		 DOWN,
		 DOWN_LELF,
		 LEFT,
		 LEFT_UP,
	 };
	
 }
class hatch;
extern short bornX;
extern short bornZ;
extern direction Direction[8];

enum ENUM_ATTR
{
	HP = 0,
	MP = 1,
	SHIELD = 2,
};
enum ENUM_SKILL_TYPE
{
	SKILL_NORMAL=0,
	SKILL_TRAIL=1,
	SKILL_AREA_TARGET=2,
	SKILL_AREA_SELF=3,
	SKILL_AREA_ISLAND=4,
	SKILL_SHILED=5,
	SKILL_DYNCBLOCK = 6,
};
class mapObj
{

};

struct playerData : public mapObj
{
	int onLine;
	int id;
	SOCKET sock;
	short x;
	short z;
	char dir;
	int hp;
	int mp;
	int attack;
	int def;
	char name[16];
	char pwd[32];
	int shield; //����
	int flag; // 1 use
	int mapid;
	dyncNodeObj dyncBlock[30];
	int dyncBlockTime;
	char dyncBlockCnt;
};

struct masterData : public mapObj
{
	int id;
	short masterType;
	int spawnid;
	short x;
	short z;
	short bx; // born x ,z
	short bz;
	int radius; 
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
	pathNode path[10];
	int pathNum;
	short fightRadius;
};

class ViewList
{
	class GUGGAME::TCPClientEvent;
public:
	static float fightLen;
	static void Init();
	
	static int regist(const char* name,const char *pwd);
	static int add(int sock, short x,short y, char dir,const char*name,const char*pwd);
	static void remove(SOCKET socke,int errorCode);
	static void attrchg(int id, int type, int num,short delay=0);
	static void addDyncBlock(int id);
	static void attrchgAreo(int* targets, int num, int type, int demage);
	static void NotifyRemoveObj(int id);
	static void NotifyJump(int id,short x,short y,char dir);
	static void NotifyAttrInit(int id,int hp,int mp,int def);
	static void NotifyMasterAttrInit(int id);
	static void NotifyCreate(int id, int type, short x, short y, char dir);
	static void NotifyWalk(int id,short x,short y,char dir);
	static void NotifyFight(int id, int target,short action);
	static void NotifyAttrChg(int id,int attr, int num,short delay=0);
	static void NotifyMasterCreate(int id, short x, short y, char dir);
	static void NotifyMapInfo(int id);
	static void NotifyDyncMapObj(int playerID);
	static bool getNearestPlayer(masterData* master,playerData** freeData);
	static bool getNearestMaster(masterData* master, masterData** freeData);
	static void areoDamage(short x, short z, char radius,int demage,int attackID,short action);
	static bool find(int id, playerData** freeData);
	static int EnterMap(SOCKET sock,short mapid);
	static bool get(playerData** freeData);
	static bool get(masterData** freeData);
	static bool find(const char* name, playerData** freeData);
	static bool find(int id, masterData** freeData);
	static int getID(SOCKET sock);
	static int getSock(int id);
	static char* getName(int id);
	static void Update(float t);
	static void masterInit();
	static void mapInit();
	static bool check(SOCKET socket);
	static void PushEvent(TCPClientEvent* e);
	static void PushEvent(IOCPEvent* e);
	static char isBlock(short x, short z);
	static short  getSkillDelayTime(short skillID);
	//static bool get(dbData** freeData);
	static char *mapDataEx;
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
	void init(short id,short x,short z,int radius,int cnt,char* name);
	short _MapObjectID;
	short _x;
	short _z;
	int _radius;
	int _cnt;
	int _spawnCnt;
	int  _flag;
	char _name[16];
};