using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class MSGProcess {
	public static int ProcessNumTotal = 0;
	static int CreateSize = 0;
	
	static Player Hero
	{
		get{
			return PlayerSys.getSingleton().getHero();
		}
	}
	public static int processMsg(List<JFPackage.IPackage> pages,System.Action<bool> cb)
	{
		try
		{
			//if(pages.Count!=0)
			//	GameDebug.LogError("before Pages count"+pages.Count);
			for(int i=0; i<pages.Count; ++i)
			{
				JFPackage.IPackage pag = pages[i];
				NetMgr.getSingleton().perfact(pag.ID);
				if(pag.ID == (uint)JFPackage.MSG_ID.MAPINFO)
				{
					JFPackage.PAG_MAPINF mapInof = (JFPackage.PAG_MAPINF)pag;
					Hero.NotifyMapInfo(mapInof._x,mapInof._z,mapInof._block);
				}
				else if(pag.ID == (uint)JFPackage.MSG_ID.JUMPINMAP)
				{
					JFPackage.PAG_JUMPINMAP jim = (JFPackage.PAG_JUMPINMAP)pag;
 					PlayerSys.getSingleton().getHero().NotifyJumpInMap(jim._id.ToString(),new Map.position(jim._x,jim._z));
				}
				else if(pag.ID == (uint)JFPackage.MSG_ID.ATTR)
				{
					JFPackage.PAG_ATTR attr = (JFPackage.PAG_ATTR)pag;
					if(Hero!=null)Hero.AttrInit(attr._id.ToString(),attr._hp,attr._mp,attr._def);
					else
						GameDebug.LogError("hero is null");
				}
				else if(pag.ID == (uint)JFPackage.MSG_ID.ATTRCHG)
				{
					JFPackage.PAG_ATTRCHG attr = (JFPackage.PAG_ATTRCHG)pag;
					if(attr._delay<=0)
					{
						Hero.NotifyAttrChg(attr._id.ToString(),attr._type,attr._num);
					}
					else
					{
						Hero.NotifyAttrChgDelay(attr._id.ToString(),attr._type,attr._num,attr._delay);
					}
				}
				else if(pag.ID == (uint)JFPackage.MSG_ID.Error)
				{
					JFPackage.PAG_ERROR error = (JFPackage.PAG_ERROR)pag;
					GameDebug.LogError("msg Error:" +((JFPackage.MSG_ID)error._theMsgid).ToString()+":"+((JFPackage.ERROR_CODE)error._errorCode).ToString());
				}
				else if(pag.ID == (uint)JFPackage.MSG_ID.REGIST)
				{
					JFPackage.PAG_REGIST reg = (JFPackage.PAG_REGIST)pag;
					GameDebug.Log("Register Success!!!"+reg._name+":"+reg._pwd);
				}
				else if(pag.ID == (uint)JFPackage.MSG_ID.DELETE)
				{
					JFPackage.PAG_DELETE del = (JFPackage.PAG_DELETE)pag;
					PlayerSys.getSingleton().getHero().NotifyRemove(del._id.ToString());
				}
				else if(pag.ID == (uint)JFPackage.MSG_ID.CREATE)
				{
					JFPackage.PAG_CREATE create = (JFPackage.PAG_CREATE)pag ;
					Map.position pos = new Map.position(create._x,create._z);
					pos._dir = create._dir;
					CreateSize++;
					GameDebug.Log("CreateSize:"+CreateSize+":("+create._x+":"+create._z+")");
					string name = StringEncoding.GetString(create._name);
					if(create._type == 0)  // hero
					{
						PlayerSys.getSingleton().createHero(create._id,pos,create._hp,name);
					}
					else // others player
					{
						PlayerSys.getSingleton().getHero().NotifyAdd(create._type,create._id.ToString(),pos,create._hp,name);
					}
				}
				else if((uint)JFPackage.MSG_ID.WALK == pag.ID)
				{
					JFPackage.PAG_WALK walk = (JFPackage.PAG_WALK)pag;
					Map.position pos = new Map.position(walk._x,walk._z);
					pos._dir = (int)walk._dir;
					PlayerSys.getSingleton().getHero().NotifyMove(walk._id.ToString(),pos);
				}
				else	if(pag.ID == (uint)JFPackage.MSG_ID.FIGHT)
				{
					JFPackage.PAG_FIGHT fight = (JFPackage.PAG_FIGHT)pag;
					Player hero = PlayerSys.getSingleton().getHero();
					//if(fight._id != PlayerSys.getSingleton().getHero().ID)
					{
						PlayerSys.getSingleton().getHero().NotifyFight(fight._id,fight._target,fight._action);
					}
					//else
					{
					//	hero.fight(fight._action,PlayerSys.getSingleton().getControl().curPos);
					}
				}
				else if(pag.ID == (uint)JFPackage.MSG_ID.CONNECTED)
				{
					//short id = 0;
					//PlayerSys.getSingleton().createHero(id,new Map.position(0,0));
				}
				else if(pag.ID == (uint) JFPackage.MSG_ID.TEST_STRING)
				{
					JFPackage.PAG_STRING pag64 = (JFPackage.PAG_STRING)pag ;
					GameDebug.Log("recv 1000:"+StringEncoding.GetString(pag64.content));
				}
				else if((uint)JFPackage.MSG_ID.TEST_STRUCTURE==pag.ID)
				{
					JFPackage.PAG_STRUCTURE inner = (JFPackage.PAG_STRUCTURE)pag;

					// * Funck 内存对齐，字符串长度问题。byte[] 中如果有0的话，转成string，长度有问题。
					string desc  = StringEncoding.GetString(inner.dest) ;
					string name = StringEncoding.GetString(inner.name) ;
					string sex5 = StringEncoding.GetString(inner.sex5);
					GameDebug.Log("recv 1001:"+ name + ":"+desc+":"+sex5);
				}
				else if( (uint)JFPackage.MSG_ID.TEST_ARRAY == pag.ID)
				{
					JFPackage.PAG_ARRAY array = (JFPackage.PAG_ARRAY)pag;
					GameDebug.Log("recv 1002:"+array._cnt);
					List<JFPackage.PAG_ArrayInner> listStruct = new List<JFPackage.PAG_ArrayInner>();
					JFTools.bytesToArrayStruct(array._inner,array._cnt,listStruct);
					foreach(var v in listStruct)
					{
						GameDebug.Log("Array Info: "+
						              " name:"+StringEncoding.GetString(v.name)+
						              " dest:"+StringEncoding.GetString(v.dest)+
						              " sex5:"+StringEncoding.GetString(v.sex5));
					}
				}
				else
				{
					GameDebug.LogError("PAG ID Not FIND!!!"+pag.ID);
				}
			}

			if(pages.Count != 0)
			{
				ProcessNumTotal += pages.Count;
				//Debug.Log("processMsg call , Num pages process :"+ProcessNumTotal);
			}
		}
		catch (System.Exception e)
		{
			GameDebug.LogError("MSGProcess Error:"+pages.Count+":"+e);
			pages.Clear();
			return -1;
		}

		if(pages.Count!=0)
		{
			//GameDebug.LogError("after Pages count"+pages.Count);
			pages.Clear();
		}
		return 0;
	}
}
