#include "stdafx.h"
#include "TcpReceiver.h"
#include "GUGPackage.h"
#include "viewListTTT.h"
#include "Event.h"
#include<stdio.h>
#include<stdlib.h>
#define random(x) (rand()%x)
#include "MapMgr.h"
#include <math.h>

void clearStream(LEUD::StreamFix &stream,  size_t seekLen);
int heroMinAtt = 445 ;
int heroMaxAtt = 506;

int recvProcess(LPPER_IO_DATA perIOData, int recvLen )
{
	LEUD::StreamFix &stream = *(perIOData->stream);
	memcpy(stream.m_Buffer + stream.m_WritePos, perIOData->dataBuf.buf, recvLen);
	stream.m_WritePos += recvLen;

	int msgLen = 0;
	while ((msgLen = BreakMessage(stream)) > 0)
	{
		if ((int)stream.size() >= msgLen)
		{
			LEUD::StreamFix *newStream = new LEUD::StreamFix(new char[msgLen], msgLen, msgLen);
			memcpy(newStream->m_Buffer, stream.m_Buffer, msgLen);
			stream.m_ReadPos += msgLen;
			int seekLen = msgLen;
			clearStream(stream, seekLen);

			// input queue
			TCPClientEvent *e = new TCPClientEvent(perIOData->client, newStream);
			ViewList::PushEvent(e);
		}
		else
		{
			break;
		}
	}
	return 0;
}

bool needTarget(short action)
{
	if (action == SKILL_AREA_SELF ||
		action == SKILL_AREA_ISLAND ||
		action == SKILL_SHILED)
		return false;

	return true;
}

void SkillUse(SOCKET client,Fight &f)
{
	if (f.action == 999)
	{
		ViewList::NotifyMapInfo(f.attackerID);
		return;
	}
	playerData *player = NULL;
	playerData *target = NULL;
	masterData *master = NULL;
	bool bfind = ViewList::find(f.attackerID, &player);
	if (bfind)
	{
		if (player->hp <= 0)
		{
			return;
		}
	}

	int errorcode = GUGGAME::OK;
	int targetId = 0;
	short tx=-1, tz=-1;
	if (needTarget(f.action))
	{
		if (MapMgr::isPlayer(f.targetID))
		{
			bfind = ViewList::find(f.targetID, &target);
			if (bfind)
			{
				if (target->hp <= 0)
				{
					errorcode = ERROR_FIGHT_TARGET_DEAD;
				}
				else
				{
					targetId = target->id;
					tx = target->x;
					tz = target->z;
				}
			}
		}
		else
		{
			bfind = ViewList::find(f.targetID, &master);
			if (bfind)
			{
				targetId = master->id;
				tx = master->x;
				tz = master->z;
				if (master->dead > 0)
				{
					errorcode = GUGGAME::ERROR_FIGHT_TARGET_DEAD;
				}
				else
				{
					if (NULL == master->target)
						master->target = player;
					else
					{
						int rNum = random(3);
						if (0 == rNum)
							master->target = player;
					}
				}
			}
		}
	}
	


	if (player != NULL&&
		((needTarget(f.action)&&targetId>0)||needTarget(f.action)==false)&&
		errorcode == GUGGAME::OK)
	{
		if (f.action == SKILL_NORMAL) // normal
		{
			ViewList::NotifyFight(f.attackerID, f.targetID, f.action,f.order);

			int num = ViewList::calcDeamge(heroMinAtt, heroMaxAtt, SKILL_NORMAL);
			ViewList::attrchg(f.attackerID,targetId,f.action, HP, num,0,f.order);
		}
		else if (f.action == SKILL_TRAIL) // magic
		{
			if (player->mp >= 10)
			{
				float speed = 10.0f;
				int num = ViewList::calcDeamge(heroMinAtt, heroMaxAtt, SKILL_TRAIL);
				ViewList::NotifyFight(f.attackerID, f.targetID, f.action,f.order);
				short x = (short)abs(player->x - tx);
				short z = (short)abs(player->z - tz);
				float delay = (float)(sqrt(x*x+z*z)) / speed;
				short dd = (short)(delay * 1000);
				ViewList::attrchg(f.attackerID,player->id,f.action, MP, -10,0,f.order);
				ViewList::attrchg(f.attackerID, targetId, f.action, HP, num, dd, f.order);
			}
			else
			{
				errorcode = GUGGAME::ERROR_NOT_ENOUGH_MP;
			}
		}
		else if (f.action == SKILL_AREA_TARGET) // areo magic
		{
			if (player->mp <= 30)
			{
				errorcode = GUGGAME::ERROR_NOT_ENOUGH_MP;
			}
			else
			{
				if (MapMgr::isMaster(targetId)) // select master
				{
					if (master == NULL)
					{
						printf("master is null why.....not fix error!");
					}
					else
					{
						short areoX = master->x;
						short areoZ = master->z;
						char radius = 2;
						int num = -random(100);
						ViewList::NotifyFight(f.attackerID, f.targetID, f.action,f.order);
						ViewList::attrchg(f.attackerID, f.attackerID, f.action, MP, -30, 0,f.order);
						ViewList::areoDamage(areoX, areoZ, radius, num, f.attackerID, f.action,f.order);
					}
				}
				else if (MapMgr::isPlayer(targetId)) // select player/self
				{
					playerData* targetPlayer = NULL;
					bfind = ViewList::find(f.targetID, &targetPlayer);
					if (bfind)
					{
						short areoX = targetPlayer->x;
						short areoZ = targetPlayer->z;
						char radius = 2;
						int num = -random(200);
						ViewList::NotifyFight(f.attackerID, f.targetID, f.action,f.order);
						ViewList::attrchg(f.attackerID, f.attackerID, f.action, MP, -30,0, f.order);
						ViewList::areoDamage(areoX, areoZ, radius, num, f.attackerID, f.action,f.order);
					}
					else
					{
						printf("select player not find why.... fixe it \n");
					}
				}
			}
		}
		else if (SKILL_AREA_SELF == f.action)
		{
			short areoX = player->x;
			short areoZ = player->z;
			char radius = 2;
			int num = ViewList::calcDeamge(heroMinAtt, heroMaxAtt, SKILL_AREA_SELF);
			ViewList::NotifyFight(f.attackerID, -1, f.action,f.order);
			ViewList::attrchg(f.attackerID, f.attackerID, f.action, MP, -30, 0,f.order);
			ViewList::areoDamage(areoX, areoZ, radius, num, f.attackerID, f.action,f.order);
		}
		else if (SKILL_AREA_ISLAND == f.action)
		{
			short areoX = f.parm01;
			short areoZ = f.parm02;
			char radius = 2;

			int num = ViewList::calcDeamge(heroMinAtt, heroMaxAtt, SKILL_AREA_ISLAND);
			ViewList::NotifyFight(f.attackerID, -1, f.action,f.order,areoX,areoZ);
			ViewList::attrchg(f.attackerID, f.attackerID, f.action, MP, -30,0, f.order);
			ViewList::areoDamage(areoX, areoZ, radius, num, f.attackerID, f.action,f.order);
		}
		else if (SKILL_SHILED == f.action) // shiled
		{
			if (player->mp >= 50)
			{
				ViewList::NotifyFight(f.attackerID, f.targetID, f.action,f.order);
				ViewList::attrchg(f.attackerID,f.attackerID,f.action, MP, -50);
				player->shield = 3000;
				ViewList::attrchg(f.attackerID, f.attackerID, f.action, SHIELD, player->shield, 0,f.order);
			}
		}
		else if (SKILL_DYNCBLOCK == f.action) // dync block
		{
			if (player->mp >= 60)
			{
				ViewList::attrchg(f.attackerID, f.attackerID, f.action, MP, -60,0, f.order);
				// dync block create 
				ViewList::addDyncBlock(f.attackerID);
			}
		}
	}
	else
	{
		if (errorcode == GUGGAME::OK)
			errorcode = GUGGAME::ERROR_FIGHT_TARGET_NULL;
	}

	if (errorcode != GUGGAME::OK)
	{
		ErrorCode ec;
		ec.msgid = (short)f.no;
		ec.errorcode = errorcode;
		SendStruct(client, ec, 1);
	}
}


void OnAccept(SOCKET client,LEUD::StreamFix &stream)
{
	int msgLen = BreakMessage(stream);
		if ((int)stream.size() == msgLen)
		{
			Head head;
			stream >> head;

			short  mssgid = (short)BreakMessageID(stream);
			static int cnt = 0;
			if (mssgid != 3001 && mssgid != 3000)
			{
				if (!ViewList::check(client))
					return;
			}
			if (mssgid == MSGID_TEST_STRING)
			{
				Pakcage64 pag;
				stream >> pag;
				if (MSGID_TEST_STRING != pag.no)
				{
					printf("whyyyyyyyyyyyyyyyyyyyyyyyy.");
				}
				//printf("recv num %d:%d:%s\n", ++cnt, pag.no, pag.data);
				SendStruct(client, pag, 1);
			}
			else if (MSGID_TEST_STRUCTURE == mssgid)
			{

				//printf("recv num %d\n", ++cnt);
				test02 t02;
				char sss[2];
				sss[0] = 'a';
				sss[1] = 'b';
				try
				{
					stream >> t02;
				}
				catch (...)
				{
					//				 char* buf =	stream.m_Buffer;
				}

				if (MSGID_TEST_STRUCTURE != t02.no)
				{
					printf("whyyyyyyyyyyyyyyyyyyyyyyyy.");
				}
				SendStruct(client, t02, 1);
			}
			else if (MSGID_TEST_ARRAY == mssgid)
			{
				//printf("recv num %d\n", ++cnt);
				test03 t03;
				stream >> t03;
				if (MSGID_TEST_ARRAY != t03.no)
				{
					printf("whyyyyyyyyyyyyyyyyyyyyyyyy.");
				}
				SendStruct(client, t03, 1);
			}
			else if (MSGID_WALK == mssgid)
			{
				Walk w;
				stream >> w;
				//w.sock = (short)perIOData->client;
				if (!ViewList::isDead(ViewList::getID(client)))
				{
					SendStruct(client, w, 1);
					ViewList::NotifyWalk(w.id, w.x, w.z, w.dir);
				}
			}
			else if (MSGID_FIGHT == mssgid)
			{
				Fight f;
				stream >> f;
				SkillUse(client, f);

			}
			else if (MSGID_LOGIN == mssgid)
			{
				Login lg;
				stream >> lg;
				int code = ViewList::add(client, bornX, bornZ, 0, (char*)lg.name, (char*)lg.pwd);
				//if( GUGGAME::OK!=code)
				{
					ErrorCode ec;
					ec.msgid = mssgid;
					ec.errorcode = code;
					SendStruct(client, ec, 1);
				}
			}
			else if (MSGID_REGIST == mssgid)
			{
				Register lg;
				stream >> lg;
				if (lg.bAuto)
				{
					ZeroMemory(&lg, sizeof(Register));
					lg.bAuto = true;
					lg.no = MSGID_REGIST;
					sprintf_s(lg.name, 15, "%s_%d", "yys", random(200000));
					sprintf_s(lg.pwd, "pwdyys_%d",random(100000));
				}
				int error = ViewList::regist(lg.name, lg.pwd);

				if ((lg.bAuto && error!=GUGGAME::OK) ||
					 0==lg.bAuto)
				{
					ErrorCode ec;
					ec.msgid = mssgid;
					ec.errorcode = error;
					SendStruct(client, ec, 1);
				}
				if (lg.bAuto && error==GUGGAME::OK)
				{
					SendStruct(client, lg, 1);
				}
			}
			else if (MSGID_ENETRMAP == mssgid)
			{
				EnterMap em;
				stream >> em;
				//SendStruct(perIOData->client, em, 1);
				int error = ViewList::EnterMap(client, em.mapID);
				if (GUGGAME::OK != error)
				{
					ErrorCode ec;
					ec.msgid = mssgid;
					ec.errorcode = error;
					SendStruct(client, ec, 1);
				}
			}
			else if (MSG_ID_RELIVE == mssgid)
			{
				reLive re;
				stream >> re;

				int id = ViewList::getID(client);
				playerData* player = NULL;
				bool bfind = ViewList::find(id, &player);
				if (bfind&&player->hp<=0)
				{
						ViewList::getCreatureMaxHpMp(0, player->hp, player->mp);
						player->def = 100;
						player->x = reliveX;
						player->z = reliveZ;
						player->shield = 0;
						ViewList::NotifyJump(id, player->x, player->z, player->dir);
						ViewList::NotifyAttrInit(id, player->hp, player->mp, player->def,player->shield);
				}
				else
				{
					ErrorCode ec;
					ec.msgid = mssgid;
					ec.errorcode = GUGGAME::ERROR_RELIVE_ERROR;
					SendStruct(client, ec, 1);
				}
			}
		}
}

void clearStream(LEUD::StreamFix &stream,size_t seekLen)
{
	int offset=0;
	if (seekLen != stream.m_ReadPos)
	{
		printf("seekLen %d:%d", seekLen, stream.m_ReadPos);
	}

	if (stream.m_WritePos >= stream.m_ReadPos)
	{
		memcpy(stream.m_Buffer, stream.m_Buffer+stream.m_ReadPos, stream.m_WritePos - stream.m_ReadPos);
		offset = stream.m_WritePos - stream.m_ReadPos;
	}
	else
	{
		printf("whyyyyyyyyyyyyyyy pos error.");
	}
	stream.m_ReadPos = 0;
	stream.m_WritePos = offset;

	ZeroMemory(stream.m_Buffer + stream.m_WritePos, stream.m_Size - stream.m_WritePos);
}
unsigned int BreakMessageID(LEUD::StreamFix &stream)
{
	char buf[4];
	memcpy(buf, stream.m_Buffer + stream.m_ReadPos, 4);
	return *(unsigned int*)buf;
}

short BreakMessage(LEUD::StreamFix &stream)
{
	if (stream.size() >= GUGGAME::HEAD_LEN)
	{
		unsigned short len = FindHeadEx(stream.m_Buffer);
		if (len > 0)
			return len;
	}
	return 0;
}






int recvSizeNormal(LPPER_IO_DATA perIOData, int bytesTransferred)
{
	char buf[1024];
	ZeroMemory(buf, 1024);
	LEUD::StreamFix stream(buf, 1024, 0);
	LEUD::StreamFix headStream(perIOData->buf, perIOData->dataBuf.len, bytesTransferred);
	Head head;
	headStream >> head;

	static short cnt = 0;
	head.len = cnt++;
	int msgID = 1000;//head.no;
	switch (msgID)
	{
	case 1000:
	{
		Pakcage64 pag;
		headStream >> pag;
		//printf("recv form %s:%d:head-%d,len-%d,no-%d,%s\n", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port),head.head,head.len,head.no, UnicodeToANSI(UTF8ToUnicode(pag.data)));
		//printf("recv form %s:%d:head-%d,len-%d,no-%d,%s\n", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port), head.head, head.len, msgID, (pag.data));
		test02 send02;
		stream << (short)0 << (short)0;

		strcpy_s(send02.name, "ÄãºÃClient");
		strcpy_s(send02.dest, "ÃèÊö");
		strcpy_s(send02.sex5, "fmal");
		stream << send02;
		static int sendNum = 0, sendNumFailed = 0, recvNum = 0;
		MakeHeadEx(stream.m_Buffer, (unsigned short)stream.size());
	//	printf("recv num %d\n", ++recvNum);
		for (int i = 0; i < 1; ++i)
		{
			LPPER_IO_DATA SendperIOData = (LPPER_IO_DATA)GlobalAlloc(GPTR, sizeof(PER_IO_DATA));
			SendperIOData->client = perIOData->client;
			memcpy(SendperIOData->buf, stream.m_Buffer, stream.size());
			send_post(SendperIOData, stream.size());
			sendNum++;
		}
		//printf("send num %d:%d,%f\n", sendNum, sendNumFailed, (float)sendNum / (float)(sendNumFailed + sendNum));
	}break;
	case 1001:
	{
		test03 send03;
		//		printf("recv form %s:%d:head-%d,len-%d,no-%d\n", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port), head.head, head.len, msgID);

		test03Inner send02;

		send02.id = head.len;
		strcpy_s(send02.name, "hello¿Í»§¶Ë");
		strcpy_s(send02.dest, "ÃèÊö");
		strcpy_s(send02.sex5, "male");

		send03.size = 2;
		memcpy((char*)&send03.data[0], &send02, sizeof(send02));
		memcpy((char*)&send03.data[1], &send02, sizeof(send02));

		stream << send03;
		send(perIOData->client, stream.m_Buffer, stream.size(), 0);
	}break;
	case 1002:
	{
		test03 send03;
		headStream >> send03;
		//	printf("recv form %s:%d:head-%d,len-%d,no-%d\n", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port), head.head, head.len, msgID);
		for (int i = 0; i < (int)send03.size; ++i)
		{
			printf("recv array:%d:%s\n", send03.data[i].id, send03.data[i].name);
		}
	}break;
	default:{
		//		printf("recv form %s:%d: msgId Error\n", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
	}break;
	}
	return 0;
}