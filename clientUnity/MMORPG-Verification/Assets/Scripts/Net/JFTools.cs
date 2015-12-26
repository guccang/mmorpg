using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Formatters.Binary;
using System;

public class JFTools
{

	public static S buildArrayStruct<T,S>(List<T> inT) where S : JFPackage.IPackage,new()
	{
		byte[] bytes;
		int sizeOfT = Marshal.SizeOf(typeof(T)) ;
		int SizeOfSrcObjLst = sizeOfT* inT.Count;
		int SizeOfDescObj = Marshal.SizeOf(typeof(S));
		if(SizeOfDescObj<SizeOfSrcObjLst) 
		{
			GameDebug.LogError("Error Desc NOT Enought!!!");
			return default(S);
		}

		S pag = new S();
		IntPtr descBuf = Marshal.AllocHGlobal(SizeOfDescObj);
		try
		{
			// head
			short []headInfo = new short[2];
			headInfo[0] = pag.HEAD;
			headInfo[1] = pag.LEN;
			int []headInfo2 = new int[2];
			headInfo2[0] = (int)pag.ID;
			headInfo2[1] = inT.Count;
			Marshal.Copy(headInfo,0,descBuf,2);
			Marshal.Copy(headInfo2,0,new IntPtr(descBuf.ToInt32()+JFTools.size(typeof(short))*2),2);
			int offset = JFTools.size(typeof(short))*2 + JFTools.size(typeof(uint))*2;
			// body
			for(int i=0; i<inT.Count;++i)
			{
				bytes = StructToBytes(inT[i]);
				Marshal.Copy(bytes,0,new IntPtr(descBuf.ToInt32()+i*sizeOfT+offset),sizeOfT);
			}
			return (S)Marshal.PtrToStructure(descBuf,typeof(S));
		}
		finally
		{
			Marshal.FreeHGlobal(descBuf);
		}
}

	public static void bytesToArrayStruct<T>(byte[] bytes,int size,List<T> outT)
	{
		int iStructSize = Marshal.SizeOf(typeof(T));
		IntPtr buffer = Marshal.AllocCoTaskMem(iStructSize* size);
		Marshal.Copy(bytes,0,buffer,iStructSize*size);
		int iCurOffset = 0;
		for(int i=0;i<size;i++)
		{
		  outT.Add( (T)Marshal.PtrToStructure(new IntPtr(buffer.ToInt32()+iCurOffset),typeof(T) ));
		  iCurOffset += iStructSize;
		}
		Marshal.FreeCoTaskMem(buffer);
	}

	public static short size( Type t)
	{
		return (short)Marshal.SizeOf(t);
	}

	public static int size(object o)
	{
		return Marshal.SizeOf(o);
	}
	//结构体转字节数组
	public static byte[] StructToBytes(object structObj)
	{
		int size =  Marshal.SizeOf(structObj);
		IntPtr buffer = Marshal.AllocHGlobal(size);
		try
		{
			Marshal.StructureToPtr(structObj, buffer, false);
			byte[] bytes = new byte[size];
			Marshal.Copy(buffer, bytes, 0,size);
			return bytes;
		}
		finally
		{
			Marshal.FreeHGlobal(buffer);
		}
	}
	//字节数组转结构体
	public static object BytesToStruct(byte[] bytes, Type strcutType)
	{
		int size = Marshal.SizeOf(strcutType);
		IntPtr buffer = Marshal.AllocHGlobal(size);
		try
		{
			Marshal.Copy(bytes, 0, buffer, size);
			return Marshal.PtrToStructure(buffer, strcutType);
		}
		finally
		{
			Marshal.FreeHGlobal(buffer);
		}
		
	}

	//字节数组转结构体
	public static object BytesToStruct(byte[] bytes, Type strcutType,int dataLen)
	{
		int size = dataLen;//Marshal.SizeOf(strcutType);
		IntPtr buffer = Marshal.AllocHGlobal(size);
		try
		{
			Marshal.Copy(bytes, 0, buffer, size);
			return Marshal.PtrToStructure(buffer, strcutType);
		}
		finally
		{
			Marshal.FreeHGlobal(buffer);
		}
		
	}

	/// <summary>
	/// GB2312转换成UTF8
	/// </summary>
	/// <param name="text"></param>
	/// <returns></returns>
	public static string gb2312_utf8(string text)
	{
		//声明字符集   
		System.Text.Encoding utf8, gb2312;
		//gb2312   
		gb2312 = System.Text.Encoding.GetEncoding("gb2312");
		//utf8   
		utf8 = System.Text.Encoding.GetEncoding("utf-8");
		byte[] gb;
		gb = gb2312.GetBytes(text);
		gb = System.Text.Encoding.Convert(gb2312, utf8, gb);
		//返回转换后的字符   
		return utf8.GetString(gb);
	}
	
	/// <summary>
	/// UTF8转换成GB2312
	/// </summary>
	/// <param name="text"></param>
	/// <returns></returns>
	public static string utf8_gb2312(string text)
	{
		//声明字符集   
		System.Text.Encoding utf8, gb2312;
		//utf8   
		utf8 = System.Text.Encoding.GetEncoding("utf-8");
		//gb2312   
		gb2312 = System.Text.Encoding.GetEncoding("gb2312");
		byte[] utf;
		utf = utf8.GetBytes(text);
		utf = System.Text.Encoding.Convert(utf8, gb2312, utf);
		//返回转换后的字符   
		return gb2312.GetString(utf);
	}
}
