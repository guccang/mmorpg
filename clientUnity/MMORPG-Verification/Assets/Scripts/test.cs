using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class test : MonoBehaviour {
	int sendSuccess = 0;
	int sendFailed = 0;

	
	string serverIPLocal = "127.0.0.1";
	string serverIPRemote = "182.92.156.9";
	short  serverPort = 5150;
	string regName="";
	string pwd = "";
	string selfInfo = "";
	string selfInfoEx = "";
	string targetInfo = "";
	public controller HeroCtr
	{
		get
		{
			return Hero.ctl;
		}
	}

	public AI HeroAI
	{
		get
		{
			return Hero.HeroAI();
		}
	}

	public Player Hero
	{
		get
		{
			return PlayerSys.getSingleton().getHero();
		}
	}
	void Awake()
	{
		Application.runInBackground = true;
		ResMgr.getSingleton().Init();
	}
	// Use this for initialization
	void Start () {
	}

	void OnConnected()
	{
		GameDebug.Log("connected!");
	}

	void OnApplicationQuit()
	{
		NetMgr.getSingleton().disConnect();
		NetMgr.getSingleton().Dump();
	}

	void InPutCtl()
	{
		if(Input.GetKeyUp(KeyCode.W))
		{
			HeroCtr.moveUp();
		}
		if(Input.GetKeyUp(KeyCode.S))
		{
			HeroCtr.moveDown();
		}
		if(Input.GetKeyUp(KeyCode.A))
		{
			HeroCtr.moveLeft();
		}
		if(Input.GetKeyUp(KeyCode.D))
		{
			HeroCtr.moveRight();
		}

	}
	// Update is called once per frame
	float f= 0;
	int cnt = 0;
	float deta = 0.1f;

	void Update () {
		InPutCtl();

		f += Time.deltaTime;
		if(f>deta &&cnt-->0)
			f -= deta;
		else
			return;

		if(NetMgr.getSingleton().isConnected())
		{
			sendString();
			sendStruct();
			sendArray();
		}
	}

	void FixedUpdate()
	{
		if(NetMgr.getSingleton().isConnected())
				MSGProcess.processMsg(NetMgr.getSingleton().msgData(),OnProcessFinish);
	}

	void OnProcessFinish(bool bSendSuccess)
	{
		if(bSendSuccess) sendSuccess++;
		else sendFailed++;
		//GameDebug.Log("click  sendSuccess:"+sendSuccess);
		//GameDebug.Log("click  sendFailed:"+sendFailed);
	}

	void sendString()
	{
		JFPackage.PAG_STRING _p = new JFPackage.PAG_STRING(StringEncoding.GetBytes("你好PC"));
		NetMgr.getSingleton().sendMsg(_p);
	}

	void sendStruct()
	{
		JFPackage.PAG_STRUCTURE _s = new JFPackage.PAG_STRUCTURE(
			StringEncoding.GetBytes("Hello"),
			cnt,
			StringEncoding.GetBytes("guccang"),
			StringEncoding.GetBytes("What")
			);
		NetMgr.getSingleton().sendMsg(_s);
	}

	void sendArray()
	{
		JFPackage.PAG_ArrayInner inner = new JFPackage.PAG_ArrayInner(
			StringEncoding.GetBytes("ArrayDest"),
			cnt,
			StringEncoding.GetBytes("guccang"),
			StringEncoding.GetBytes("arr")
			);
		List<JFPackage.PAG_ArrayInner> lst = new List<JFPackage.PAG_ArrayInner>();
		lst.Add(inner);lst.Add(inner);
		JFPackage.PAG_ARRAY _a = JFTools.buildArrayStruct<JFPackage.PAG_ArrayInner,JFPackage.PAG_ARRAY>(lst);
		NetMgr.getSingleton().sendMsg(_a);
	}

	void OnConnectedSuccess(object sockObj)
	{
	}

	void OnGUI()
	{
		int buttonW=100,buttonH=30,gapH=35;
		int gapVal = 0;

		if(Hero==null)
		{
			if (GUI.Button(new Rect(0, 0, buttonW, buttonH), "connectLocal"))
			{
				NetMgr.getSingleton().connect(serverIPLocal,serverPort,OnConnectedSuccess);
			}
			if (GUI.Button(new Rect(0, gapVal+=gapH, buttonW, buttonH), "connectRemote"))
			{
				NetMgr.getSingleton().connect(serverIPRemote,serverPort,OnConnectedSuccess);
			}
		
			regName = GUI.TextField(new Rect(buttonW, gapVal+gapH, buttonW, buttonH),regName);
			pwd = GUI.PasswordField(new Rect(buttonW*2, gapVal+gapH, buttonW, buttonH),pwd,'*');
			if (GUI.Button(new Rect(0, gapVal += gapH, buttonW, buttonH), "regist"))
			{
				JFPackage.PAG_REGIST regist = new JFPackage.PAG_REGIST(0,StringEncoding.GetBytes(regName),StringEncoding.GetBytes(pwd));
				NetMgr.getSingleton().sendMsg(regist);
			}
			if (GUI.Button(new Rect(0, gapVal += gapH, buttonW, buttonH), "autoregist"))
			{
				JFPackage.PAG_REGIST regist = new JFPackage.PAG_REGIST(1,StringEncoding.GetBytes(regName),StringEncoding.GetBytes(pwd));
				NetMgr.getSingleton().sendMsg(regist);
			}
			if (GUI.Button(new Rect(0, gapVal += gapH, buttonW, buttonH), "login"))
			{
				JFPackage.PAG_LOGIN login = new JFPackage.PAG_LOGIN(StringEncoding.GetBytes(regName),StringEncoding.GetBytes(pwd));
				NetMgr.getSingleton().sendMsg(login);
			}
			if (GUI.Button(new Rect(0, gapVal += gapH, buttonW, buttonH), "enterMap"))
			{
				JFPackage.PAG_EnetrMap en = new JFPackage.PAG_EnetrMap(11);
				NetMgr.getSingleton().sendMsg(en);
			}
		}
		else
		{
			if (GUI.Button(new Rect(0, 0, buttonW, buttonH), "Again"))
			{
				HeroAI.reset();
			}
		
			if (GUI.Button(new Rect(0, gapVal += gapH, buttonW, buttonH), "disconnected"))
			{
				//if(NetMgr.getSingleton().isConnected())
					NetMgr.getSingleton().disConnect();
			}
			if (GUI.Button(new Rect(0, gapVal += gapH, buttonW, buttonH), "relive"))
			{
				JFPackage.PAG_RELIVE re = new JFPackage.PAG_RELIVE(11);
				NetMgr.getSingleton().sendMsg(re);
			}
			// name,hp,mp,def

			Creature target = Hero.Target;
			selfInfo = Hero._name +":" +Hero.shiled +":"+Hero.hp +  ":"+Hero.mp + ":" + "(" + Hero.ctl.curPos._x + ":" + Hero.ctl.curPos._z + ")";
			selfInfoEx = Hero.maxHp + ":" + Hero.maxMp;
			if(target!=null)
					targetInfo = target._name + ":"+target.shiled +":" + target.maxHp + ":" +target.hp + ":" +target.maxMp+":"+target.mp+"("+ target.ctl.curPos._x+":"+target.ctl.curPos._z+")";

			GUI.Label(new Rect(0,gapVal+=gapH,buttonW*2,buttonH),selfInfo);
			GUI.Label(new Rect(0,gapVal+=gapH,buttonW*2,buttonH),selfInfoEx);
			GUI.Label(new Rect(0,gapVal+=gapH,buttonW*2,buttonH),targetInfo);
		}
		GUI.Label(new Rect(0,gapVal+=gapH,buttonW*3,buttonH*2),GameDebug.lastError);
		
	}
	
}