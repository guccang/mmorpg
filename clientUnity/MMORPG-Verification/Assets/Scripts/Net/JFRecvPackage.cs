using UnityEngine;
using System.Collections;
using System.Net.Sockets;
public class JFRecvPackage  {

	static byte []PackageContext = new byte[1024*10];

	public static byte []  RecvPackage(Socket sock)
	{
		int len;
		JFPackage.PAG_HEAD head = new JFPackage.PAG_HEAD();
		while(true)
		{
			len = recveSize(sock,PackageContext,JFPackage.HEAD_LENGTH,0);
			if(len<=0)
			{
				GameDebug.Log("head recveSize:"+len);
				break;
			}
			head = (JFPackage.PAG_HEAD)JFTools.BytesToStruct(PackageContext,head.GetType());
			//GameDebug.Log("head.header:"+head.header+":"+head.no);
			
			if(head.header>0)
			{
				len = recveSize(sock,PackageContext,head.header-JFPackage.HEAD_LENGTH,JFPackage.HEAD_LENGTH);
				//GameDebug.Log("body recveSize:"+len);
				if(len<=0)
				{
					GameDebug.Log("body recveSize:"+len);
					break;
				}
				return PackageContext;
			}
			else
			{
				GameDebug.Log("head.header <= 0:"+head.no);
			}
		}
		return null;
	}

	static int recveSize(Socket s,byte[] b,int iLen,int offset)
	{
		byte[] p = b;
		int len = iLen;
		int ret = 0;
		int retrunLen = 0;
		while(len>0)
		{
			ret = s.Receive(p,offset+iLen-len,iLen-retrunLen,0);
			if(ret<=0)
			{
				GameDebug.Log("Socket.Receive <= 0.");
				return ret;
			}
			len -= ret;
			retrunLen += ret;
		}
		return retrunLen;
	}

}
