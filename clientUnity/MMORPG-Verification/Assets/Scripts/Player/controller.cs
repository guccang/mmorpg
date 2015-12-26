using UnityEngine;
using System.Collections;

public class controller : MonoBehaviour {

	public GameObject player;

	Map.position needMoveTo;
	float speed = 2.0f;
	float f = 0;
	Vector3 fromPos=Vector3.zero,toPos=Vector3.zero;
	Map.Direction dir;

	/*
		0 : normal
		2: sendMsg
		3: move
	 */
	public int state = 0; 
	public void SetNormalSate()
	{
		state = 0;
	}
	public void setPerMoveState()
	{
		state = 1;
	}
	public void setMoveState()
	{
		state = 2;
	}
	public void setAfterMoveState()
	{
		state = 3;
	}

	// Use this for initialization
	void Start () {
	}

	public void Init()
	{
		needMoveTo = Map.getCellPos(player.transform.position);
		SetNormalSate();
	}

	public void NotifyPos(Map.position pos)
	{
		setAfterMoveState();
		if(needMoveTo != pos) 
		{
			needMoveTo = pos;
			//toPos = Map.getWorldPos(needMoveTo);
		}
	}

	 public void SetMove(Map.position pos)
	{
		setMoveState();
		if(pos != needMoveTo)
		{
			if(Mathf.Abs(pos._x - needMoveTo._x) >5 ||
			   Mathf.Abs(pos._z - needMoveTo._z) > 5)
			{
				//GameDebug.LogError("this is error on server....");
			}
			f = 0;
			needMoveTo = pos;
			fromPos = player.transform.position;
			toPos = Map.getWorldPos(needMoveTo);
		}
	}

	public Map.position curPos
	{
		get
		{
			Map.position pos =  Map.getCellPos(player.transform.position);
			pos._dir = needMoveTo._dir;
			return pos;
		}
	}
	// Update is called once per frame
	void Update () {
		DoMove();
	}

	public bool isMoving()
	{
		return f!=0;
	}

	void DoMove()
	{
		if(fromPos!=toPos)
		{
			f += Time.deltaTime * speed;
			player.transform.position = Vector3.Lerp(fromPos,toPos,f);	
			if(f>=1.0f)
				fromPos = toPos;
		}
		else
		{
			f=0;
			SetNormalSate();
		}
	}

	int playerID()
	{
		return (int)PlayerSys.getSingleton().getHero().ID;
	}

	public void moveUp()
	{
		Map.position pos = Map.getCellPos(player.transform.position);
		pos += Map.Direction2Position[(int)Map.Direction.UP];
		JFPackage.PAG_WALK wakl = new JFPackage.PAG_WALK(playerID(), pos._x,pos._z,(byte)Map.Direction.UP);
		NetMgr.getSingleton().sendMsg(wakl);
		setPerMoveState();
		SetMove(pos);
	}
	public void moveDown()
	{
		Map.position pos = Map.getCellPos(player.transform.position);
		pos += Map.Direction2Position[(int)Map.Direction.DOWN];
		JFPackage.PAG_WALK wakl = new JFPackage.PAG_WALK(playerID(),pos._x,pos._z,(byte)Map.Direction.DOWN);
		NetMgr.getSingleton().sendMsg(wakl);
		setPerMoveState();
		SetMove(pos);
	}
	public void moveLeft()
	{
		Map.position pos = Map.getCellPos(player.transform.position);
		pos += Map.Direction2Position[(int)Map.Direction.LEFT];
		JFPackage.PAG_WALK wakl = new JFPackage.PAG_WALK(playerID(),pos._x,pos._z,(byte)Map.Direction.LEFT);
		NetMgr.getSingleton().sendMsg(wakl);
		setPerMoveState();
		SetMove(pos);
	}
	public void moveRight()
	{
		Map.position pos = Map.getCellPos(player.transform.position);
		pos += Map.Direction2Position[(int)Map.Direction.RIGHT];
		JFPackage.PAG_WALK wakl = new JFPackage.PAG_WALK(playerID(),pos._x,pos._z,(byte)Map.Direction.RIGHT);
		NetMgr.getSingleton().sendMsg(wakl);
		setPerMoveState();
		SetMove(pos);
	}
	public void moveStop()
	{

	}

	public void moveAuto(Map.position pos)
	{
		JFPackage.PAG_WALK wakl = new JFPackage.PAG_WALK(playerID(),pos._x,pos._z,(byte)Map.Direction.LEFT);
		NetMgr.getSingleton().sendMsg(wakl);
		setPerMoveState();
		SetMove(pos);
	}
}
