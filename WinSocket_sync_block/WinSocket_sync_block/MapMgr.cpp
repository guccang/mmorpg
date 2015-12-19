#include "stdafx.h"
#include "MapMgr.h"
#include <windows.h>


enum MAPOBJTYPE
{
	PLAYERID = 1000,
	MASTERID = 100000,
};

static long PlayerID = PLAYERID;
static long MasterID = MASTERID;


bool MapMgr::isMaster(int id)
{
	return id >= MASTERID;
}

bool MapMgr::isPlayer(int id)
{
	return (id >= PLAYERID &&id<MASTERID);
}

int MapMgr::generateID(int type)
{
	if (0 == type)
	{
		return InterlockedExchangeAdd(&PlayerID,1);
	}

	if (1 == type)
	{
		return InterlockedExchangeAdd(&MasterID, 1);
	}

	return -1;
}