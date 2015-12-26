using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;

public class NetPerformance  {
	public class timeDelta
	{
		Stopwatch watch;
		public void start(){watch.Reset(); watch.Start();}
		public void stop(){watch.Stop();}
		public long ms(){return watch.ElapsedMilliseconds;}
		public timeDelta()
		{
			watch = new Stopwatch();
		}
	}
	public class msgInfo 
	{
		public uint id;
		public long delta; // ms
	}

	public class cmp : IComparer<msgInfo>
	{
		public int Compare(msgInfo o1,msgInfo o2)
		{
			return (int)o1.id - (int)o2.id;
		}
	}
	public Dictionary<uint,timeDelta> MsgDelta;
	public List<msgInfo> msgDeltaResult;

	public NetPerformance()
	{
			MsgDelta = new Dictionary<uint,timeDelta>();
			msgDeltaResult = new List<msgInfo>();
	}

	public void perfact(uint msgid)
	{
		if(MsgDelta.ContainsKey(msgid)==false)
		{
			/*
			if(msgid!=1003&&
			   msgid!=2002&&
			   msgid!=2003&&
			   msgid!=1) // not send ,just recv
					GameDebug.LogError("stopwatch error:"+msgid);
					*/
			return;
		}
		MsgDelta[msgid].stop();
		msgInfo mi = new msgInfo();
		mi.id = msgid;
		mi.delta = MsgDelta[msgid].ms();
		msgDeltaResult.Add(mi);
	}

	public void Dump()
	{
		localIO.Init("performance"+System.DateTime.Now.ToString("HHMMss")+".txt");
		localIO.Write("performance process.\r\n");
		Dictionary<uint,long>  avg = new Dictionary<uint, long>();
		Dictionary<uint,int>  cnt = new Dictionary<uint, int>();
		msgDeltaResult.Sort(new cmp());
		for(int i=0;i<msgDeltaResult.Count; ++i)
		{
			uint id = msgDeltaResult[i].id;
			long delta = msgDeltaResult[i].delta;
			localIO.Write(id + ":" + ":" +  delta+"\r\n");
			if(avg.ContainsKey(id)==false)
				avg.Add(id, delta);
			else
				avg[id]+=delta;

			if(cnt.ContainsKey(id)==false)
				cnt.Add(id,1);
			else
				cnt[id] += 1;
		}
		
		foreach(var k in avg.Keys)
		{
			localIO.Write("\r\navg: msg"+k+":"+(float)avg[k]/(float)cnt[k]);
		}

		localIO.close();
	}

}
