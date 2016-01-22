using UnityEngine;
using System.Collections;

public class Creature : MonoBehaviour
{
	public enum MAPOBJ
	{
		HERO=0,
		OTHERPLAYER=1,
		MASTER1=2,
		MASTER2=3,
		MASTERBOSS=4,
		DYNCBLOCK=5,
	}

	public enum ENUM_ATTR
	{
		HP = 0,	
		MP = 1,
		SHILED=2, // 返回的当前护盾值,其他属性返回的是改变值
		DEF = 3,
	}
	protected int _id;
	public controller ctl;

	public string _name;
	public int hp;
	public int mp;
	public int maxHp;
	public int maxMp;
	public int def;
	public int shiled;
	public string color;

	public int ID
	{
		get{
			return _id;
		}
	}

	void Start()
	{
	}

	public bool isDead()
	{
		return hp <=0;
	}

	public bool isMaster(int id)
	{
		return id >= 100000;
	}

	public bool isPlayer(int id)
	{
		return id>=1000 && id<100000;
	}

	public virtual void Init(int id,Map.position pos)
	{
		if(this.gameObject.GetComponent<controller>()==null)
		{
			ctl = this.gameObject.AddComponent<controller>();
			ctl.player = this.gameObject;
			ctl.Init();
		}
		_id = id;
		this.transform.position = Map.getWorldPos(pos);
	}

	public virtual void fight(short action,Creature attack,Creature target)
	{
	//	GameDebug.Log("fight:"+ID);

		GameObject bulletObj = ResMgr.getSingleton().getBullet();
		bulletObj.transform.position = this.transform.position;
		bullet b =  bulletObj.GetComponent<bullet>();
		b.Init(attack,target,(int)action);
	}

	public void chgColor(string col)
	{
		if(col == "target")
			gameObject.GetComponent<Renderer>().material = ResMgr.getSingleton().getMatTarget();
		if(col == "hero")
			gameObject.GetComponent<Renderer>().material = ResMgr.getSingleton().getmat();
		if(col == "normal")
		   gameObject.GetComponent<Renderer>().material = ResMgr.getSingleton().getMatNormal();
		if(col == "dead")
			gameObject.GetComponent<Renderer>().material = ResMgr.getSingleton().getMatDead();
		if(col == "area")
			gameObject.GetComponent<Renderer>().material = ResMgr.getSingleton().getMatArea();
		if(col == "green")
			gameObject.GetComponent<Renderer>().material  = ResMgr.getSingleton().getGreenMat();
		color = col;
	}

	public void chgAttr(short type,short num)
	{
		if(type == (short)ENUM_ATTR.HP)
		{
			//GameDebug.Log("attr hp chg:"+num);
			hp += num;
			if(hp<=0)
			{
				chgColor("dead");
				hp = 0;
			}
		}
		else if((short)ENUM_ATTR.MP==type)
		{
			mp += num;
		}
		else if((short)ENUM_ATTR.SHILED == type)
		{
			shiled = num;
		}
		else if((short)ENUM_ATTR.DEF == type)
		{
			def += num;
		}
		else
		{
			GameDebug.LogError("chg attr error :"+type+":"+num);
		}
	}
}
