#include "stdafx.h"
#include "MapMgr.h"
#include <windows.h>


enum MAPOBJTYPE
{
	PLAYERID	    = 1000,
	MASTERID		= 100000,
	DYNCMAPOBJID      = 1000000,
};

static long PlayerID = PLAYERID;
static long DyncMapObjID = DYNCMAPOBJID;
static long MasterID = MASTERID;

bool MapMgr::isMaster(int id)
{
	return (id >= MASTERID && id<DYNCMAPOBJID);
}

bool MapMgr::isPlayer(int id)
{
	return (id >= PLAYERID &&id<MASTERID);
}

bool MapMgr::isDyncMapObj(int id)
{
	return (id >= DYNCMAPOBJID );
}

int MapMgr::generateID(int type)
{
	if( ENUM_MAPOBJ_PLAYER == type)
	{
		return InterlockedExchangeAdd(&PlayerID,1);
	}
	else
	if (ENUM_MAPOBJ_MASTER == type)
	{
		return InterlockedExchangeAdd(&MasterID, 1);
	}
	else
	if (ENUM_MAPOBJ_DYNCMAPOBJ == type)
	{
		return InterlockedExchangeAdd(&DyncMapObjID, 1);
	}
	else
	{
		printf("map generate id error:%d",type);
	}
	return -1;
}

