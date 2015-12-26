using UnityEngine;
using System.Collections;
using System.Collections.Generic;


public class NetMgr  {

	static NetMgr _ins = null;
	JFSocket _net = null;
  	NetPerformance performance;

	public void Dump()
	{
		if(performance!=null)
			performance.Dump();
	}

	void Init()
	{
		performance = new NetPerformance();
	}

	public void perfact(uint msgid)
	{
		performance.perfact(msgid);
	}

	public static NetMgr getSingleton()
	{
		if(_ins == null)
		{
			_ins = new NetMgr();
			_ins.Init();
		}
		return _ins;
	}

	public List<JFPackage.IPackage> msgData()
	{
		if(isConnected())
			return _net.worldPackages;
		return null;
	}

	public bool isConnected()
	{
		if(null != _net)
			return _net.isConnected();
		else
			return false;
	}

	public void disConnect()
	{
		if(_net!=null)
		{
			_net.Closed();
			if(PlayerSys.getSingleton().getHero()!=null)
					PlayerSys.getSingleton().getHero().NotifyRemove("all");
			_net = null;
		}
	}

	public void connect(string ip,int port,System.Action<object> connectedCB)
	{
		if(false == isConnected())
		{
			_net = JFSocket.getInstance();
			if(connectedCB == null)
				_net.connect(ip,port,OnConnected);
			else
				_net.connect(ip,port,connectedCB);
		}
	}

	public void sendMsg(JFPackage.IPackage pag)
	{
		if(isConnected())
		{
			_net.SendMessage(pag,OnSendSuccess);
			NetPerformance.timeDelta td = new NetPerformance.timeDelta();
			performance.MsgDelta[pag.ID] = td;
			td.start();
		}
	}

	void OnConnected(object o)
	{
		GameDebug.Log("connected");
	}

	void OnSendSuccess(bool bsuccess)
	{
					
	}
}
