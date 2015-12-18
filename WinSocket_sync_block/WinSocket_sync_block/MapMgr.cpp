#include "stdafx.h"
#include "MapMgr.h"
#include <windows.h>

static long PlayerID = 1000;
static long MasterID = 100000;

bool MapMgr::isMaster(int id)
{
	return id >= MasterID;
}

bool MapMgr::isPlayer(int id)
{
	return (id > 0 && id <= PlayerID);
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
}