using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class AI {
	public List<Map.position> pathNode;

	int curPosIndex ;
	float autoFightTime = 0;
	public void AutoFight(float t)
	{
		autoFightTime+=t;
		if(autoFightTime<=1.5f) return;

	    autoFightTime = 0;
		Player hero = PlayerSys.getSingleton().getHero();
		Creature master = hero.getMater(3.0f);
		if(master!=null && master.hp>0)
		{
			hero.chgTraget(master);
			JFPackage.PAG_FIGHT fightMSG = new JFPackage.PAG_FIGHT(hero.ID,-1,(short)Player.ENUM_SKILL_TYPE.SKILL_TRAIL);
			fightMSG._target = master.ID;
			NetMgr.getSingleton().sendMsg(fightMSG);
		}
	}

	public void 	AutoMove()
	{
		Player hero = PlayerSys.getSingleton().getHero();
		if(pathNode.Count!=0 && curPosIndex<pathNode.Count)
		{
			if(hero.ctl.state == 0)
			{
				Map.position pos = pathNode[curPosIndex++];
				//GameDebug.Log("Pos:("+pos._x+":"+pos._z+")");
				hero.ctl.moveAuto(pos);
			}
		}
		//if(curPosIndex>=pathNode.Count)
		//	reset();
	}

	public void Init()
	{
		pathNode = new List<Map.position>();
		/*
		 * 	
		int len = 2;
		//+up
		for(int i=0; i<len;++i)
		{
			pathNode.Add(new Map.position(0,i));
		}
		// +down
		for(int i=len-1; i>=0;--i)
		{
			pathNode.Add(new Map.position(0,i));
		}

		// - up
		for(int i=0; i<len;++i)
		{
			pathNode.Add(new Map.position(0,-i));
		}
		// -down
		for(int i=len-1; i>=0;--i)
		{
			pathNode.Add(new Map.position(0,-i));
		}

		
		// +left
		for(int i=0; i<len;++i)
		{
			pathNode.Add(new Map.position(i,0));
		}
		// +right
		for(int i=len-1; i>=0;--i)
		{
			pathNode.Add(new Map.position(i,0));
		}

		
		// -left
		for(int i=0; i<len;++i)
		{
			pathNode.Add(new Map.position(-i,0));
		}
		// -right
		for(int i=len-1; i>=0;--i)
		{
			pathNode.Add(new Map.position(-i,0));
		}


		curPosIndex = 0;
		*/
	}

	public void reset()
	{
		curPosIndex = 0;
	}
}
