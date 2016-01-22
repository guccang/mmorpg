using UnityEngine;
using System.Collections;
using System;
using System.Threading;
using System.Text;
using System.Net;
using System.Net.Sockets;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Formatters.Binary;

public class threadInfo
{
	public delegate int processMsg(List<JFPackage.IPackage> msgs,System.Action<bool> cbAction);
	public processMsg Handle;
	public AutoResetEvent recvEndEvent;
}

public class JFSocket  {

	private threadInfo threadInfoData;

	private Socket clientSocket;
	
	private static JFSocket instance;
	
	List<JFPackage.IPackage> _worldPackages;
	List<JFPackage.IPackage> _worldPackages2;
	List<JFPackage.IPackage> _curPackages;

	public List<JFPackage.IPackage> worldPackages
	{
		get
		{
			lock(_curPackages)
			{
				if(_curPackages.Equals(_worldPackages) && _worldPackages2.Count == 0)
				{
					_curPackages = _worldPackages2;
					return _worldPackages;
				}
				else
				{
					_curPackages = _worldPackages;
					return _worldPackages2;
				}

			}
		}
	}

	WaitHandle []waitHandls ;
	bool bRecb = true;
	
	public static JFSocket getInstance()
	{
		//if(instance == null)
		{
			instance = new JFSocket();
		}
		return instance;
	}

	public void setProcessHandle(threadInfo.processMsg handle)
	{
		threadInfoData.Handle = handle;
	}

	JFSocket()
	{
		waitHandls 	=  new WaitHandle[1];
		waitHandls[0] = new AutoResetEvent(false);

		threadInfoData = new threadInfo();
		threadInfoData.Handle = null;
		threadInfoData.recvEndEvent =(AutoResetEvent) waitHandls[0];

		_worldPackages = new List<JFPackage.IPackage>();
		_worldPackages2 = new List<JFPackage.IPackage>();
		_curPackages = _worldPackages;
		clientSocket = null;
	}

	public bool isConnected()
	{

		return (clientSocket!=null && clientSocket.Connected) ;
	}

	public void disconnect()
	{
		Closed();
	}

	public  void connect(string ip,int port,System.Action<object> cbConnected)
	{
		clientSocket = new Socket(AddressFamily.InterNetwork,SocketType.Stream,ProtocolType.Tcp);
		
		IPAddress ipAddr = IPAddress.Parse(ip);
		
		IPEndPoint ipEndPoint = new IPEndPoint(ipAddr,port);
		
		IAsyncResult result = clientSocket.BeginConnect(ipEndPoint,new AsyncCallback(connectCallBack),cbConnected);
		
		bool success = result.AsyncWaitHandle.WaitOne(5000,true);
		
		if(!success)
		{
			Closed();
			GameDebug.Log("connect Time Out");
		}
		else
		{
			Thread thread = new Thread(new ParameterizedThreadStart(ReceiveSorket));
			thread.IsBackground = true;
			thread.Start((object)threadInfoData );
		}
	}

	private void connectCallBack(IAsyncResult asyncConnect)
	{
		System.Action<object> cbConnected = (System.Action<object>)asyncConnect.AsyncState;
		if(cbConnected!=null)
			cbConnected((object)clientSocket);

		JFPackage.PAG_HEAD h = new JFPackage.PAG_HEAD();
		h.no = (uint)JFPackage.MSG_ID.CONNECTED;
		_curPackages.Add(h);
	}

	public void ReceiveSorket(object parm)
	{
		threadInfo info = ((threadInfo)parm);
		AutoResetEvent handleEvent = info.recvEndEvent;

		while(bRecb)
		{
			if(!clientSocket.Connected)
			{
				GameDebug.Log("Failedd to clientSocket serer. whyyyyyy");
				clientSocket.Close();
				break;
			}
			try
			{
				SplitPackage(JFRecvPackage.RecvPackage(clientSocket),0);
			}
			catch(Exception e)
			{
				GameDebug.LogError("Failed to clientSocket error."+e);
				if(clientSocket!=null && clientSocket.Connected)
						clientSocket.Close();
				break;
			}
		}

		handleEvent.Set();
	}

	private void SplitPackage(byte[] bytes,int index)
	{
		if(bytes == null)
		{
			return ;
		}
		do
		{
			byte[] head = new byte[JFPackage.HEAD_LENGTH];
			int headLengthIndex = index;
			Array.Copy(bytes,index,head,0,JFPackage.HEAD_LENGTH);
			short totalLength = BitConverter.ToInt16(head,0);
			short len = BitConverter.ToInt16(head,2);
			uint no = BitConverter.ToUInt32(head,4);
			int dataLen = getPackageLenght(no);
			if(totalLength>0)
			{
				byte[] data = new byte[dataLen];
				try
				{
				Array.Copy(bytes,headLengthIndex,data,0,totalLength);
				}
				catch(System.Exception e)
				{
					GameDebug.LogError(e+" header"+totalLength+":"+no+":"+dataLen);
				}
				getPackage(totalLength,len,no,data);
				index = headLengthIndex + totalLength;
			}
			else
			{
				GameDebug.LogError("totalLenght:"+totalLength);
			}
		}while(false);
	}

	int getPackageLenght(uint no)
	{
		int len = 0;
		if(no == (uint)JFPackage.MSG_ID.LISTKILL)
		{
			len = JFTools.size(typeof(JFPackage.PAG_ListKill));
		}
		else if(no == (uint)JFPackage.MSG_ID.TEST_STRING)
		{
			len =  JFTools.size(typeof(JFPackage.PAG_STRING));
		}
		else if(no == (uint)JFPackage.MSG_ID.TEST_STRUCTURE)
		{
			len =   JFTools.size(typeof(JFPackage.PAG_STRUCTURE));
		}
		else if(no == (uint)JFPackage.MSG_ID.TEST_ARRAY)
		{
			len =   JFTools.size(typeof(JFPackage.PAG_ARRAY));
		}
		else if(no == (uint)JFPackage.MSG_ID.WALK)
		{
			len = JFTools.size(typeof(JFPackage.PAG_WALK));
		}
		else if(no == (uint)JFPackage.MSG_ID.FIGHT)
		{
			len = JFTools.size(typeof(JFPackage.PAG_FIGHT));
		}
		else if(no == (uint)JFPackage.MSG_ID.CREATE)
		{
			len = JFTools.size(typeof(JFPackage.PAG_CREATE));
		}
		else if(no == (uint)JFPackage.MSG_ID.DELETE)
		{
			len = JFTools.size(typeof(JFPackage.PAG_DELETE));
		}
		else if(no == (uint)JFPackage.MSG_ID.REGIST)
		{
			len = JFTools.size(typeof(JFPackage.PAG_REGIST));
		}
		else if(no == (uint)JFPackage.MSG_ID.Error)
		{
			len = JFTools.size(typeof(JFPackage.PAG_ERROR));
		}
		else if(no == (uint)JFPackage.MSG_ID.ATTRCHG)
		{
			len = JFTools.size(typeof(JFPackage.PAG_ATTRCHG));
		}
		else if(no == (uint)JFPackage.MSG_ID.ATTR)
		{
			len = JFTools.size(typeof(JFPackage.PAG_ATTR));
		}
		else if(no == (uint)JFPackage.MSG_ID.JUMPINMAP)
		{
			len = JFTools.size(typeof(JFPackage.PAG_JUMPINMAP));
		}
		else if(no == (uint)JFPackage.MSG_ID.MAPINFO)
		{
			len = JFTools.size(typeof(JFPackage.PAG_MAPINF));
		}
		else if(no == (uint)JFPackage.MSG_ID.ENTRPMAP)
		{
			len = JFTools.size(typeof(JFPackage.PAG_EnetrMap));
		}
		else if(no == (uint)JFPackage.MSG_ID.RELIVE)
		{
			len = JFTools.size((typeof(JFPackage.PAG_RELIVE)));
		}
		else if(no == (uint)JFPackage.MSG_ID.REGIST)
		{
			len = JFTools.size((typeof(JFPackage.PAG_REGIST)));
		}
		else if(no == (uint)JFPackage.MSG_ID.NOTIFY)
		{
			len = JFTools.size((typeof(JFPackage.PAG_Notify)));
		}
		else
		{
			GameDebug.LogError("NO Find MSD in getPackageLenght.");
		}
		return len;
	}

	void getPackage(int header,int Lenght,uint no,byte[] data)
	{
		if(no == (uint)JFPackage.MSG_ID.LISTKILL)
		{
			JFPackage.PAG_ListKill wp = new JFPackage.PAG_ListKill();
			wp = (JFPackage.PAG_ListKill)JFTools.BytesToStruct(data,wp.GetType());
			_curPackages.Add(wp);
		}
		else if(no == (uint)JFPackage.MSG_ID.NOTIFY)
		{
			JFPackage.PAG_Notify wp = new JFPackage.PAG_Notify();
			wp = (JFPackage.PAG_Notify)JFTools.BytesToStruct(data,wp.GetType());
			_curPackages.Add(wp);
		}
	  else if(no == (uint)JFPackage.MSG_ID.RELIVE)
	  {
					JFPackage.PAG_RELIVE wp = new JFPackage.PAG_RELIVE();
					wp = (JFPackage.PAG_RELIVE)JFTools.BytesToStruct(data,wp.GetType());
					_curPackages.Add(wp);
	  }
	  else	if(no == (uint)JFPackage.MSG_ID.ENTRPMAP)
		{
			JFPackage.PAG_EnetrMap wp = new JFPackage.PAG_EnetrMap();
			wp = (JFPackage.PAG_EnetrMap)JFTools.BytesToStruct(data,wp.GetType());
			_curPackages.Add(wp);
		}
		else if(no == (uint)JFPackage.MSG_ID.MAPINFO)
		{
			JFPackage.PAG_MAPINF wp = new JFPackage.PAG_MAPINF();
			wp = (JFPackage.PAG_MAPINF)JFTools.BytesToStruct(data,wp.GetType());
			_curPackages.Add(wp);
		}
		else if(no == (uint)JFPackage.MSG_ID.JUMPINMAP)
		{
			JFPackage.PAG_JUMPINMAP wp = new JFPackage.PAG_JUMPINMAP();
			wp = (JFPackage.PAG_JUMPINMAP)JFTools.BytesToStruct(data,wp.GetType());
			_curPackages.Add(wp);
		}
		else if(no == (uint)JFPackage.MSG_ID.ATTR)
		{
			JFPackage.PAG_ATTR wp = new JFPackage.PAG_ATTR();
			wp = (JFPackage.PAG_ATTR)JFTools.BytesToStruct(data,wp.GetType());
			_curPackages.Add(wp);
		}
		else if(no == (uint)JFPackage.MSG_ID.ATTRCHG)
		{
			JFPackage.PAG_ATTRCHG wp = new JFPackage.PAG_ATTRCHG();
			wp = (JFPackage.PAG_ATTRCHG)JFTools.BytesToStruct(data,wp.GetType());
			_curPackages.Add(wp);
		}
		else if(no == (uint)JFPackage.MSG_ID.TEST_STRING)
		{
			JFPackage.PAG_STRING wp = new JFPackage.PAG_STRING();
			wp = (JFPackage.PAG_STRING)JFTools.BytesToStruct(data,wp.GetType());
			_curPackages.Add(wp);
		}
		else if(no == (uint)JFPackage.MSG_ID.TEST_STRUCTURE)
		{
			JFPackage.PAG_STRUCTURE wp = new JFPackage.PAG_STRUCTURE();
			wp = (JFPackage.PAG_STRUCTURE)JFTools.BytesToStruct(data,wp.GetType());
			_curPackages.Add(wp);
		}
		else if(no == (uint)JFPackage.MSG_ID.TEST_ARRAY)
		{
			JFPackage.PAG_ARRAY wp = new JFPackage.PAG_ARRAY();
			wp = (JFPackage.PAG_ARRAY)JFTools.BytesToStruct(data,wp.GetType());
			_curPackages.Add(wp);
		}
		else if(no == (uint)JFPackage.MSG_ID.WALK )
		{
			JFPackage.PAG_WALK wp = new JFPackage.PAG_WALK();
			wp = (JFPackage.PAG_WALK)JFTools.BytesToStruct(data,wp.GetType());
			_curPackages.Add(wp);
		}
		else if(no == (uint)JFPackage.MSG_ID.FIGHT)
		{
			JFPackage.PAG_FIGHT wp = new JFPackage.PAG_FIGHT();
			wp= (JFPackage.PAG_FIGHT)JFTools.BytesToStruct(data,wp.GetType());
			_curPackages.Add(wp);
		}
		else if(no == (uint)JFPackage.MSG_ID.CREATE)
		{
			JFPackage.PAG_CREATE wp = new JFPackage.PAG_CREATE();
			wp = (JFPackage.PAG_CREATE)JFTools.BytesToStruct(data,wp.GetType());
			_curPackages.Add(wp);
		}
		else if(no == (uint)JFPackage.MSG_ID.DELETE)
		{
			JFPackage.PAG_DELETE wp = new JFPackage.PAG_DELETE();
			wp = (JFPackage.PAG_DELETE)JFTools.BytesToStruct(data,wp.GetType());
			_curPackages.Add(wp);
		}
		else if(no == (uint)JFPackage.MSG_ID.REGIST)
		{
			JFPackage.PAG_REGIST wp = new JFPackage.PAG_REGIST();
			wp = (JFPackage.PAG_REGIST)JFTools.BytesToStruct(data,wp.GetType());
			_curPackages.Add(wp);
		}
		else if(no == (uint)JFPackage.MSG_ID.Error)
		{
			JFPackage.PAG_ERROR wp = new JFPackage.PAG_ERROR();
			wp = (JFPackage.PAG_ERROR)JFTools.BytesToStruct(data,wp.GetType());
			_curPackages.Add(wp);
		}
		else
		{
			GameDebug.LogError("getPackage error "+ no);
		}
	}

	public void SendMessage(JFPackage.IPackage obj,System.Action<bool> cbSendSuccess)
	{
		if(!clientSocket.Connected)
		{
			clientSocket.Close();
			return;
		}
		try
		{
			Byte[] data = JFTools.StructToBytes(obj);
			//GameDebug.Log("data len"+data.Length);
			IAsyncResult asyncSend = clientSocket.BeginSend(data,0,data.Length,SocketFlags.None,new AsyncCallback(sendCallBack),cbSendSuccess);
			bool success = asyncSend.AsyncWaitHandle.WaitOne(5000,true);
			if(!success)
			{
				clientSocket.Close();
				GameDebug.Log("Time out !");
				cbSendSuccess(false);
			}
		
		}
		catch(Exception e)
		{
			GameDebug.LogError("Send message error:"+e);
		}
	}

	private void sendCallBack(IAsyncResult asyncSend)
	{
		System.Action<bool> cb = (System.Action<bool> )asyncSend.AsyncState;
		cb(true);
	}

	public void Closed()
	{
		if(clientSocket != null && clientSocket.Connected)
		{
			clientSocket.Shutdown(SocketShutdown.Both);
			bRecb = false;
			WaitHandle.WaitAll(waitHandls,5000);
			clientSocket.Close();
			clientSocket = null;
		}
	}
}
