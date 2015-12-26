using UnityEngine;
using System.Collections;

public class PlayerSys {

	static PlayerSys _ins = null;
	Player _hero = null;
	GameObject _island = null;

	public static PlayerSys getSingleton()
	{
		if(_ins == null)
		{
			_ins = new PlayerSys();
		}
		return _ins;
	}

	public void Init(Player playerCtr)
	{
		_hero = playerCtr;
		_island = GameObject.FindGameObjectWithTag("island");
		if(null == _island)
		{
			_island = GameObject.Instantiate<GameObject>(new GameObject());
		}
		_island.transform.localPosition = Vector3.zero;
	}

	public Transform getIsland()
	{
		return _island.transform;
	}

	public Player getHero()
	{
		return _hero;
	}

	public controller getControl()
	{
		return _hero.ctl;
	}

	public Creature createCreature(int type,int playerId,Map.position pos,int hp,string name)
	{
		GameObject obj = null;

		if((int)Creature.MAPOBJ.HERO==type || 
		   (int)Creature.MAPOBJ.OTHERPLAYER == type ||
		   (int)Creature.MAPOBJ.DYNCBLOCK == type) 
		{
			obj = ResMgr.getSingleton().getCube();
		}
		else
		{
			obj = ResMgr.getSingleton().getMaster();
		}
		obj.transform.position = Map.getWorldPos(pos);
		if(obj.GetComponent<Creature>()==null)
			obj.AddComponent<Creature>();

		Creature c = obj.GetComponent<Creature>();
		c.Init(playerId,pos);
		c.hp = hp;
		c._name = name;
		if(type == (int)Creature.MAPOBJ.MASTERBOSS) // boss
		{
			c.transform.localScale = new Vector3(1.5f,1.5f,1.5f);
		}
		else if(type == (int)Creature.MAPOBJ.MASTER2)
		{
			c.transform.localScale = new Vector3(1.2f,1.2f,1.2f);
		}
		else if(type == (int)Creature.MAPOBJ.DYNCBLOCK)
		{
			c.chgColor("green");
		}
	   c.transform.name = c._name+c.ID;
		return c;
	}

	public Player createHero(int playerId,Map.position pos,int hp,string name)
	{
		GameObject obj = ResMgr.getSingleton().getCube();

		if(obj.GetComponent<Player>()==null)
			obj.AddComponent<Player>();

		_hero = obj.GetComponent<Player>();
		_hero.Init(playerId,pos);

		Init(_hero);
		//_hero.gameObject.GetComponent<Renderer>().material = ResMgr.getSingleton().getmat();
		_hero.chgColor("hero");
		_hero._name = name;
		_hero.hp = hp;
		_hero.NotifyAdd(0,playerId.ToString(),pos,hp,name);
		return _hero;
	}
}
