using UnityEngine;
using System.Collections;
using System.Text;

public class StringEncoding  {

	static Encoding theEncoding = System.Text.Encoding.UTF8;

	public static byte[] GetBytes(string str)
	{
		return  theEncoding.GetBytes(str);
	}

	public static string GetString(byte[] bytes)
	{
		int len = 0;
		for(int i=0;i<bytes.Length;++i)
		{
			if(bytes[i] != 0)
				len++;
			else
				break;
		}
		return theEncoding.GetString(bytes,0,len);
	}

}
