using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class Player : Creature {
	class delayAttr
	{
		public string _id;
		public short type;
		public short num;
		public short delay;
	}

	List<delayAttr> _delayList;
	Dictionary<string,Creature>  _viewList;
	public Vector3 cameraX;
	AI _ai;
	Creature target;
	GameObject mainCamera;
	Map.position _island;

	enum ENUM_SKILL_TYPE
	{
		SKILL_NORMAL=0,				// 普通攻击
		SKILL_TRAIL=1,				  //  带拖尾的
		SKILL_AREA_TARGET=2,  // 选中目标，范围
		SKILL_AREA_SELF=3, 	// 自身 范围
		SKILL_AREA_ISLAND=4, // 点击地面 范围
		SKILL_SHILED=5,	// 护盾
		SKILL_DYNCBLOCK=6, // 动态阻挡
	}
	public Creature Target
	{
		get
		{
			return target;
		}
	}

	public override void Init (int id, Map.position pos)
	{
		base.Init (id, pos);
		_viewList = new Dictionary<string,Creature>();
		_delayList = new List<delayAttr>();
		_ai = new AI();
		_ai.Init();
		mainCamera = GameObject.FindGameObjectWithTag("MainCamera");
		if(null == mainCamera)
			GameDebug.LogError("Error not find MainCamera tag");
		cameraX.x = 0;
		cameraX.y = 8;
		cameraX.z = -10;
		_island = Map.invalidPos;
	}

	public AI HeroAI()
	{
		return _ai;
	}

	// Use this for initialization
	void Start () {

	}

	void cameraMove()
	{
		//mainCamera.transform.LookAt(this.transform.position);
		Vector3 pos =  Vector3.Lerp(mainCamera.transform.position,this.transform.position,0.5f);
		pos += cameraX;
		mainCamera.transform.position = pos;
	}

	void DelayAttrAction(short t)
	{
		delayAttr da = null;
		for(int i=0;i<_delayList.Count;++i)
		{
			da = _delayList[i];
			_delayList[i].delay -=  t;
			if(_delayList[i].delay<=0)
			{
				NotifyAttrChg(da._id,da.type,da.num);
			}
		}

		_delayList.RemoveAll((o)=>{return o.delay<=0;});
	}

	void DrawCritical()
	{
		DrawGrahical.getSingleton().DrawCircle(transform.position.x,transform.position.z,9,this.gameObject);
	}
	// Update is called once per frame
	void Update () {
		_ai.AutoMove();
		DelayAttrAction((short)(Time.deltaTime*1000));
		cameraMove();
		DrawCritical();
		if(target && target.isDead())
		{
			//chgTraget(null);
		}

		if(Input.GetKeyUp(KeyCode.Alpha9))
		{
			JFPackage.PAG_FIGHT fightMSG = new JFPackage.PAG_FIGHT(ID,-1,999);
			fightMSG._target = target.ID;
			NetMgr.getSingleton().sendMsg(fightMSG);
		}

		// norma attack
		if(Input.GetKeyUp(KeyCode.Space) && target!=null)
		{
			JFPackage.PAG_FIGHT fightMSG = new JFPackage.PAG_FIGHT(ID,-1,(short)ENUM_SKILL_TYPE.SKILL_NORMAL);
			fightMSG._target = target.ID;
			NetMgr.getSingleton().sendMsg(fightMSG);
		}

		// magic attack
		if(Input.GetKeyUp(KeyCode.Alpha1) && target!=null)
		{
			if(mp>=10)
			{
				JFPackage.PAG_FIGHT fightMSG = new JFPackage.PAG_FIGHT(ID,-1,(short)ENUM_SKILL_TYPE.SKILL_TRAIL);
				fightMSG._target = target.ID;
				NetMgr.getSingleton().sendMsg(fightMSG);
			}
			else
			{
				GameDebug.LogError("Use skill:NOT ENOUGH MP.");
			}
		}
		if(Input.GetKeyUp(KeyCode.Alpha2)&&target!=null)
		{
			if(mp>=30)
			{
				JFPackage.PAG_FIGHT fightMSG = new JFPackage.PAG_FIGHT(ID,-1,(short)ENUM_SKILL_TYPE.SKILL_AREA_TARGET);
				fightMSG._target = target.ID;
				NetMgr.getSingleton().sendMsg(fightMSG);
			}
			else
			{
				GameDebug.LogError("Use skill:NOT ENOUGH MP.");
			}
		}
		if(Input.GetKeyUp(KeyCode.Alpha3))
		{
			if(mp>=30)
			{
				JFPackage.PAG_FIGHT fightMSG = new JFPackage.PAG_FIGHT(ID,-1,(short)ENUM_SKILL_TYPE.SKILL_AREA_SELF);
				NetMgr.getSingleton().sendMsg(fightMSG);
			}
			else
			{
				GameDebug.LogError("Use skill:NOT ENOUGH MP.");
			}
		}
		if(Input.GetKeyUp(KeyCode.Alpha4))
		{
			if(mp>=30&&_island!=Map.invalidPos)
			{
				JFPackage.PAG_FIGHT fightMSG = new JFPackage.PAG_FIGHT(ID,-1,(short)ENUM_SKILL_TYPE.SKILL_AREA_ISLAND);
				fightMSG._param01 = (short)_island._x;
				fightMSG._param02 = (short)_island._z;
				NetMgr.getSingleton().sendMsg(fightMSG);
			}
			else
			{
				GameDebug.LogError("Use skill:NOT ENOUGH MP. not click island");
			}
		}
		if(Input.GetKeyUp(KeyCode.Alpha5))
		{
			if(mp>=50)
			{
				JFPackage.PAG_FIGHT fightMSG = new JFPackage.PAG_FIGHT(ID,-1,(short)ENUM_SKILL_TYPE.SKILL_SHILED);
				fightMSG._target = ID;
				NetMgr.getSingleton().sendMsg(fightMSG);
			}
			else
			{
				GameDebug.LogError("Use skill: not enough mp");
			}
		}
		if(Input.GetKeyUp(KeyCode.Alpha6))
		{
			if(mp>=50)
			{
				JFPackage.PAG_FIGHT fightMSG = new JFPackage.PAG_FIGHT(ID,-1,(short)ENUM_SKILL_TYPE.SKILL_DYNCBLOCK);
				fightMSG._target = ID;
				NetMgr.getSingleton().sendMsg(fightMSG);
			}
			else
			{
				GameDebug.LogError("Use skill: not enough mp");
			}
		}
		// select target
		_island = MousePickUp.pickIsland(_island);
		Creature c =  MousePickUp.pick(target);
		chgTraget(c);
	}

	void chgTraget(Creature c)
	{
		if(c==null)
		{
			if(target!=null)
				target.chgColor("normal");
			target = null;
			return;
		}

		if(c.ID == this.ID)
			return;

		if(target!=null)
		{
			if(c.ID == target.ID)
			{
				if(!target.isDead())target.chgColor("target");
				return;
			}
			target.chgColor("normal");
		}
		target = c;
		target.chgColor("target");
	}

	bool Contain(string id)
	{
		return _viewList.ContainsKey(id);
	}

	public Creature getCreature(string id)
	{
		if(Contain(id))
		{
			return _viewList[id];
		}
		return null;
	}

	public void NotifyMove(string playerId,Map.position pos)
	{
		if(Contain(playerId))
		{
			//GameDebug.LogError("other move:"+pos._dir);
			Creature player = _viewList[playerId];
			player.ctl.SetMove(pos);
		}
		else if(int.Parse(playerId) == PlayerSys.getSingleton().getHero().ID)
		{
			PlayerSys.getSingleton().getControl().NotifyPos(pos);
		}
		else
		{
			GameDebug.LogError("playerid not find"+playerId);
		}
	}

	public void NotifyFight(int playerId,int targetid ,int action)
	{
		if(!Contain(playerId.ToString()) ||!Contain(targetid.ToString())) 		return;
		Creature c= _viewList[playerId.ToString()];
		Creature t = _viewList[targetid.ToString()];
		if(action == 0 || action == 1)
		{
				c.fight((short)action,c,t);
		}
		else if(2==action||3==action)
		{
			area ar = t.gameObject.GetComponent<area>();
			if(null == ar)
			{
				ar = t.gameObject.AddComponent<area>();
			}
			ar.Init(targetid.ToString());
		}
	}

	public void CreateBlock(Map.position pos,byte block)
	{
		GameObject obj =  ResMgr.getSingleton().getBlock();
		obj.transform.position = Map.getWorldPos(pos);
		obj.transform.name = "bolck"+pos._x+":"+pos._z;
		obj.transform.parent = PlayerSys.getSingleton().getIsland(); 
	}

	public void NotifyMapInfo(short x,short z,byte block)
	{
		CreateBlock(new Map.position(x,z),block);
	}

	public void NotifyJumpInMap(string playerId,Map.position pos)
	{
		if(!Contain(playerId)) 
		{
			GameDebug.LogError("jump in map error"+playerId);
			return;
		}

		Creature c = _viewList[playerId];
		c.transform.position = Map.getWorldPos(pos);
	}


	public void NotifyAdd(short type,string playerId,Map.position pos,int hp ,string name)
	{
		if(Contain(playerId)) 
		{
			GameDebug.LogError("add type"+type+":"+playerId);
			return;
		}
		if((int)MAPOBJ.HERO==type)
		{
			_viewList.Add(playerId,this);
			return;
		}
		Creature c= PlayerSys.getSingleton().createCreature(type,int.Parse(playerId),pos,hp,name);
		_viewList.Add(c.ID.ToString(),c);
	}
	public void AttrInit(string id,int hp,int mp,int def)
	{
		if(!Contain(id))
		{
			GameDebug.LogError("attrinit error :"+id);
			return;
		}
		Creature c = _viewList[id];
		c.hp = hp;
		c.mp = mp;
		c.def = def;
		if(c.ID!=ID)
			c.chgColor("normal");
		else 
		{
				c.chgColor("hero");
		}
	}
	public void NotifyAttrChgDelay(string playerid,short type,short num,short delay)
	{
		delayAttr da = new delayAttr();
		da._id = playerid;
		da.type = type;
		da.num = num;
		da.delay = delay;
		_delayList.Add(da);
	}
	public void NotifyAttrChg(string playerid,short type,short num)
	{
		if(!Contain(playerid))
		{
			GameDebug.LogError("attr chg error:"+type+":"+playerid);
			return;
		}

		Creature c = _viewList[playerid];
		c.chgAttr(type,num);
	}
	public void NotifyRemove(string playerId)
	{
		if(playerId == "all")
		{
			foreach(var v in _viewList.Values)
			{
				Creature p = v;
				Destroy(p.gameObject);
			}
			_viewList.Clear();
			Destroy(this.gameObject);
			return;
		}

		if(Contain(playerId))
		{
			Creature p = _viewList[playerId];
			Destroy(p.gameObject);
			_viewList.Remove(playerId);
		}
	}
}
