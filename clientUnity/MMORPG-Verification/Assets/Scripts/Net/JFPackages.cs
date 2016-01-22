using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;
public class JFPackage
{
	public enum ERROR_CODE
	{
		OK = 0,
		ERROR_LOGIN_NOT_REGIST = 1, // 没有这个帐户
		ERROR_LOGIN_ONLINE = 2,		// 不能重复登录
		ERROR_NAME_PWD_NULL = 3,	// 名字密码为空
		ERROR_NOT_ENOUGH_MP = 4,// 没有足够的mp
		ERROR_FIGHT_TARGET_NULL = 5, // 攻击目标不存在
		ERROR_HAD_REGIST=6,	// 账号已经被注册了
		ERROR_FIGHT_TARGET_DEAD = 7, // 目标死亡
		ERROR_ENTER_MAP_NOT_LOGIN = 8, // 没有登录
	}

	public  static   short HEAD_LENGTH = 8;

	public enum MSG_ID
	{
		HEADER = 0,
		Error = 1,
		MAPINFO =2,

		_START_ = 999,
		TEST_STRING = 1000, // STRING...
		TEST_STRUCTURE = 1001, // STRUCT...
		TEST_ARRAY = 1002,// ARRAY...
		CONNECTED = 1003,
		ATTRCHG = 1004,
		ATTR = 1005, 
		JUMPINMAP = 1006,

		//LOGIC
		WALK = 2000,
		FIGHT = 2001,
		CREATE=2002,
		DELETE=2003,
		ENTRPMAP = 2004,
		RELIVE = 2005,
		NOTIFY=2006,
		LISTKILL = 2007,
		// Login
		LOGIN = 3000,
		REGIST=3001,
	}

	public enum ERRORCODE
	{
		OK = 0,
		ERROR_LOGIN_NOT_REGIST = 1,
		ERROR_LOGIN_ONLINE=2,
	}
	
	public interface  IPackage
	{
		short HEAD{get;}
		short LEN{get;}
		uint ID{get;}
	}


	//结构体序列化
	[System.Serializable]
	//4字节对齐 iphone 和 android上可以1字节对齐
	[StructLayout(LayoutKind.Sequential, CharSet=CharSet.Ansi,Pack = 1)]
	public struct PAG_HEAD :IPackage
	{
		public short   header;
		public short   len;
		public uint     no;
		public  short HEAD{get{return header;}}
		public  short LEN{get{return 0;}}
		public uint ID{get{ return no;}}
	}

	//结构体序列化
	[System.Serializable]
	//4字节对齐 iphone 和 android上可以1字节对齐
	[StructLayout(LayoutKind.Sequential, CharSet=CharSet.Ansi,Pack = 1)]
	public struct PAG_STRUCTURE : IPackage
	{
		public short   header;
		public short   len;
		public uint     no;
		public  short HEAD{get{return JFTools.size(this.GetType());}}
		public  short LEN{get{return 0;}}
		public uint ID{get{ return (uint)MSG_ID.TEST_STRUCTURE;}}

		[MarshalAs(UnmanagedType.ByValArray,SizeConst=10)]
		public byte[] dest;
		public int id;
		[MarshalAs(UnmanagedType.ByValArray,SizeConst=20)]
		public byte[] name;
		[MarshalAs(UnmanagedType.ByValArray,SizeConst=5)]
		public byte[] sex5;

		public PAG_STRUCTURE(byte[] _dest,int _id,byte[] _name,byte[] _sex5)
		{
			header = JFTools.size(typeof(JFPackage.PAG_STRUCTURE));
			len = 0;
			no = (uint)MSG_ID.TEST_STRUCTURE;

			id = _id;
			dest = _dest;
			name = _name;
			sex5 = _sex5;
		}
	}

	//结构体序列化
	[System.Serializable]
	//4字节对齐 iphone 和 android上可以1字节对齐
	[StructLayout(LayoutKind.Sequential, CharSet=CharSet.Ansi,Pack = 1)]
	public struct PAG_ArrayInner 
	{
		[MarshalAs(UnmanagedType.ByValArray,SizeConst=10)]
		public byte[] dest;
		public int id;
		[MarshalAs(UnmanagedType.ByValArray,SizeConst=20)]
		public byte[] name;
		[MarshalAs(UnmanagedType.ByValArray,SizeConst=5)]
		public byte[] sex5;

		public PAG_ArrayInner(byte[] _dest,int _id,byte[] _name,byte[] _sex5)
		{
			dest = _dest;
			id = _id;
			name = _name;
			sex5 = _sex5;
		}
	}
	//结构体序列化
	[System.Serializable]
	//4字节对齐 iphone 和 android上可以1字节对齐
	[StructLayout(LayoutKind.Sequential, CharSet=CharSet.Ansi,Pack = 1)]
	public struct PAG_ARRAY : IPackage
	{
		public short   header;
		public short   len;
		public uint     no;
		public  short HEAD{get{return JFTools.size(this.GetType());}}
		public  short LEN{get{return 0;}}
		public uint ID{get{ return (uint)MSG_ID.TEST_ARRAY;}}

		public int _cnt;
		[MarshalAs(UnmanagedType.ByValArray, SizeConst =  78)]
		public byte[] _inner;

		public PAG_ARRAY(byte cnt)
		{
			_cnt = cnt;
			_inner = new byte[0];
			header = JFTools.size(typeof(PAG_ARRAY));
			len = 0;
			no = (uint)MSG_ID.TEST_ARRAY;
		}
	}
	
	//结构体序列化
	[System.Serializable]
	//4字节对齐 iphone 和 android上可以1字节对齐
	[StructLayout(LayoutKind.Sequential, CharSet=CharSet.Ansi,Pack = 1)]
	public struct PAG_STRING :  IPackage
	{
		public short   header;
		public short   len;
		public uint     no;
		public  short HEAD{get{return JFTools.size(this.GetType());}}
		public  short LEN{get{return 0;}}
		public uint ID{get{ return (uint)MSG_ID.TEST_STRING;}}

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 64)]
		public  byte[] content;
		public PAG_STRING(byte[]  _c)
		{
			header = JFTools.size(typeof(PAG_STRING));
			len = 0;
			no = (uint)MSG_ID.TEST_STRING;
			content = _c;
		}
	}; 

	
	[System.Serializable]
	[StructLayout(LayoutKind.Sequential, CharSet=CharSet.Ansi,Pack = 1)]
	public struct PAG_WALK :IPackage
	{
		public short   header;
		public short   len;
		public uint     no;
		public  short HEAD{get{return JFTools.size(typeof(PAG_WALK));}}
		public  short LEN{get{return 0;}}
		public uint ID{get{ return (uint)MSG_ID.WALK;}}

		public int _id;
		public short _x;
		public short _z;
		public byte  _dir;

		public PAG_WALK(int id,int x,int z,byte dir)
		{
			header = JFTools.size(typeof(PAG_WALK));
			len = 0;
			no = (uint)MSG_ID.WALK;

			_id = (short)id;
			_x =(short)x;
			_z = (short)z;
			_dir =dir;
		}
	}

	[System.Serializable]
	[StructLayout(LayoutKind.Sequential, CharSet=CharSet.Ansi,Pack = 1)]
	public struct PAG_FIGHT :IPackage
	{
		public short   header;
		public short   len;
		public uint     no;
		public  short HEAD{get{return JFTools.size(typeof(PAG_FIGHT));}}
		public  short LEN{get{return 0;}}
		public uint ID{get{ return (uint)MSG_ID.FIGHT;}}
		
		public int _id;
		public int _target;
		// 0,1,2 _target 需要目标
		// 3, 区域 ,param01,param02 格子
		public short _action; // 0normal,1magic,2area

		public short _param01;
		public short _param02;
		public int _order;
		public PAG_FIGHT(int id,int target,short action)
		{
			header = JFTools.size(typeof(PAG_FIGHT));
			len = 0;
			no = (uint)MSG_ID.FIGHT;
			
			_id = id;
			_target = target;
			_action = action; 
			_param01 = 0;
			_param02 = 0;
			_order = 0;
		}
	}

	[System.Serializable]
	[StructLayout(LayoutKind.Sequential, CharSet=CharSet.Ansi,Pack = 1)]
	public struct  PAG_LOGIN:IPackage
	{
		public short   header;
		public short   len;
		public uint     no;
		public  short HEAD{get{return JFTools.size(typeof(PAG_LOGIN));}}
		public  short LEN{get{return 0;}}
		public uint ID{get{ return (uint)MSG_ID.LOGIN;}}
		
		[MarshalAs(UnmanagedType.ByValArray,SizeConst=16)]
		public byte[] _name; // name maxLenght 8
		[MarshalAs(UnmanagedType.ByValArray,SizeConst=32)]
		public byte[] _pwd; // pwd maxLenght 16

		public PAG_LOGIN(byte[] name,byte[] pwd )
		{
			header = JFTools.size(typeof(PAG_LOGIN));
			len = 0;
			no = (uint)MSG_ID.LOGIN;

			_name = name;
			_pwd   = pwd;
		}
	}

	[System.Serializable]
	[StructLayout(LayoutKind.Sequential, CharSet=CharSet.Ansi,Pack = 1)]
	public struct  PAG_REGIST:IPackage
	{
		public short   header;
		public short   len;
		public uint     no;
		public  short HEAD{get{return JFTools.size(typeof(PAG_REGIST));}}
		public  short LEN{get{return 0;}}
		public uint ID{get{ return (uint)MSG_ID.REGIST;}}

		public byte _bAuto;
		[MarshalAs(UnmanagedType.ByValArray,SizeConst=16)]
		public byte[] _name; // name maxLenght 8
		[MarshalAs(UnmanagedType.ByValArray,SizeConst=32)]
		public byte[] _pwd; // pwd maxLenght 16
		
		public PAG_REGIST(byte bAuto,byte[] name,byte[] pwd )
		{
			header = JFTools.size(typeof(PAG_REGIST));
			len = 0;
			no = (uint)MSG_ID.REGIST;

			_bAuto =  bAuto;
			_name = name;
			_pwd   = pwd;
		}
	}

	[System.Serializable]
	[StructLayout(LayoutKind.Sequential, CharSet=CharSet.Ansi,Pack = 1)]
	public struct  PAG_CREATE:IPackage
	{
		public short   header;
		public short   len;
		public uint     no;
		public  short HEAD{get{return JFTools.size(typeof(PAG_CREATE));}}
		public  short LEN{get{return 0;}}
		public uint ID{get{ return (uint)MSG_ID.CREATE;}}

		public int  _id;	//  identify
		public short _type; // hero , player,master
		public short _x;	   //  pos x
		public short _z;   //  pos z
		public byte _dir;  // direction
		[MarshalAs(UnmanagedType.ByValArray,SizeConst=16)]
		public byte[] _name; // name maxLenght 8

		public PAG_CREATE(int id,short type,short x,short z,byte dir,int hp,byte[] name)
		{
			header = JFTools.size(typeof(PAG_CREATE));
			len = 0;
			no = (uint)MSG_ID.CREATE;

			_id = id;
			_type = type;
			_x = x;
			_z = z;
			_dir  = dir;
			_name = name;
		}
	}

	
	[System.Serializable]
	[StructLayout(LayoutKind.Sequential, CharSet=CharSet.Ansi,Pack = 1)]
	public struct  PAG_DELETE:IPackage
	{
		public short   header;
		public short   len;
		public uint     no;
		public  short HEAD{get{return JFTools.size(typeof(PAG_DELETE));}}
		public  short LEN{get{return 0;}}
		public uint ID{get{ return (uint)MSG_ID.DELETE;}}
		
		public int  _id;	//  identify
		
		public PAG_DELETE(int id)
		{
			header = JFTools.size(typeof(PAG_DELETE));
			len = 0;
			no = (uint)MSG_ID.DELETE;
			_id = id;
		}
	}


	//结构体序列化
	[System.Serializable]
	//4字节对齐 iphone 和 android上可以1字节对齐
	[StructLayout(LayoutKind.Sequential, CharSet=CharSet.Ansi,Pack = 1)]
	public struct PAG_ERROR : IPackage
	{
		public short   header;
		public short   len;
		public uint     no;
		public  short HEAD{get{return JFTools.size(this.GetType());}}
		public  short LEN{get{return 0;}}
		public uint ID{get{ return (uint)MSG_ID.Error;}}

		public short _theMsgid;
		public int _errorCode;
		public PAG_ERROR(short msgid , int code )
		{
			header = JFTools.size(typeof(JFPackage.PAG_ERROR));
			len = 0;
			no = (uint)MSG_ID.Error;

			_theMsgid = msgid;
			_errorCode = code;
		}
	}

	//结构体序列化
	[System.Serializable]
	//4字节对齐 iphone 和 android上可以1字节对齐
	[StructLayout(LayoutKind.Sequential, CharSet=CharSet.Ansi,Pack = 1)]
	public struct PAG_ATTRCHG : IPackage
	{
		public short   header;
		public short   len;
		public uint     no;
		public  short HEAD{get{return JFTools.size(this.GetType());}}
		public  short LEN{get{return 0;}}
		public uint ID{get{ return (uint)MSG_ID.ATTRCHG;}}
		
		public int _attackID;
		public int _targetID;
		public short _action;
		public short _type;
		public short _num;
		public short _delay;
		public int _order;
		public PAG_ATTRCHG(int attID ,int tarID,short action ,short type , short num ,short delay)
		{
			header = JFTools.size(typeof(JFPackage.PAG_ATTRCHG));
			len = 0;
			no = (uint)MSG_ID.ATTRCHG;

			_attackID = attID;
			_targetID = tarID;
			_action = action;
			_type = type;
			_num = num;
			_delay = delay;
			_order = 0;
		}
	}

	//结构体序列化
	[System.Serializable]
	//4字节对齐 iphone 和 android上可以1字节对齐
	[StructLayout(LayoutKind.Sequential, CharSet=CharSet.Ansi,Pack = 1)]
	public struct PAG_ATTR : IPackage
	{
		public short   header;
		public short   len;
		public uint     no;
		public  short HEAD{get{return JFTools.size(this.GetType());}}
		public  short LEN{get{return 0;}}
		public uint ID{get{ return (uint)MSG_ID.ATTR;}}
		
		public int _id;
		public int _maxHp;
		public int _maxMp;
		public int _maxShiled;
		public int _hp;
		public int _mp;
		public int _shiled;
		public int _def;
		public PAG_ATTR(int id , int maxHp,int maxMp,int hp ,int mp,int def)
		{
			header = JFTools.size(typeof(JFPackage.PAG_ATTR));
			len = 0;
			no = (uint)MSG_ID.ATTR;
			
			_id = id;
			_hp = hp;
			_mp = mp;
			_def = def;
			_maxHp = maxHp;
			_maxMp = maxMp;
			_maxShiled = 0;
			_shiled = 0;
		}
	}

	//结构体序列化
	[System.Serializable]
	//4字节对齐 iphone 和 android上可以1字节对齐
	[StructLayout(LayoutKind.Sequential, CharSet=CharSet.Ansi,Pack = 1)]
	public struct PAG_JUMPINMAP : IPackage
	{
		public short   header;
		public short   len;
		public uint     no;
		public  short HEAD{get{return JFTools.size(this.GetType());}}
		public  short LEN{get{return 0;}}
		public uint ID{get{ return (uint)MSG_ID.JUMPINMAP;}}
		
		public int _id;
		public short _x;
		public short _z;
		public byte _dir;
		public PAG_JUMPINMAP(int id , short x ,short z,byte dir)
		{
			header = JFTools.size(typeof(JFPackage.PAG_JUMPINMAP));
			len = 0;
			no = (uint)MSG_ID.JUMPINMAP;
			
			_id = id;
			_x = x;
			_z = z;
			_dir = dir;
		}
	}



	[System.Serializable]
	[StructLayout(LayoutKind.Sequential, CharSet=CharSet.Ansi,Pack = 1)]
	public struct PAG_MAPINF :IPackage
	{
		public short   header;
		public short   len;
		public uint     no;
		public  short HEAD{get{return JFTools.size(typeof(PAG_MAPINF));}}
		public  short LEN{get{return 0;}}
		public uint ID{get{ return (uint)MSG_ID.MAPINFO;}}

		public short _x;
		public short _z;
		public byte _block;
		
		public PAG_MAPINF(short x,short z,byte block)
		{
			header = JFTools.size(typeof(PAG_MAPINF));
			len = 0;
			no = (uint)MSG_ID.MAPINFO;
		
			_x = x;
			_z = z;
			_block = block;
		}
	}

	
	[System.Serializable]
	[StructLayout(LayoutKind.Sequential, CharSet=CharSet.Ansi,Pack = 1)]
	public struct PAG_EnetrMap :IPackage
	{
		public short   header;
		public short   len;
		public uint     no;
		public  short HEAD{get{return JFTools.size(typeof(PAG_EnetrMap));}}
		public  short LEN{get{return 0;}}
		public uint ID{get{ return (uint)MSG_ID.ENTRPMAP;}}
		
		public short _mapid;
		
		public PAG_EnetrMap(short mapid)
		{
			header = JFTools.size(typeof(PAG_EnetrMap));
			len = 0;
			no = (uint)MSG_ID.ENTRPMAP;
			_mapid = mapid;
		}
	}

	[System.Serializable]
	[StructLayout(LayoutKind.Sequential, CharSet=CharSet.Ansi,Pack = 1)]
	public struct PAG_RELIVE :IPackage
	{
		public short   header;
		public short   len;
		public uint     no;
		public  short HEAD{get{return JFTools.size(typeof(PAG_RELIVE));}}
		public  short LEN{get{return 0;}}
		public uint ID{get{ return (uint)MSG_ID.RELIVE;}}
		
		public PAG_RELIVE(int mapid)
		{
			header = JFTools.size(typeof(PAG_RELIVE));
			len = 0;
			no = (uint)MSG_ID.RELIVE;
		}
	}


	[System.Serializable]
	[StructLayout(LayoutKind.Sequential, CharSet=CharSet.Ansi,Pack = 1)]
	public struct PAG_Notify :  IPackage
	{
		public short   header;
		public short   len;
		public uint     no;
		public  short HEAD{get{return JFTools.size(this.GetType());}}
		public  short LEN{get{return 0;}}
		public uint ID{get{ return (uint)MSG_ID.NOTIFY;}}

		public byte _type; // 0单独  1 广播
		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
		public  byte[] _attack;
		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
		public  byte[] _target;
		public PAG_Notify(byte[]  attack,byte[] target)
		{
			header = JFTools.size(typeof(PAG_Notify));
			len = 0;
			no = (uint)MSG_ID.NOTIFY;
			_type = 0;
			_attack = attack;
			_target = target;
		}
	}; 



	//结构体序列化
	[System.Serializable]
	//4字节对齐 iphone 和 android上可以1字节对齐
	[StructLayout(LayoutKind.Sequential, CharSet=CharSet.Ansi,Pack = 1)]
	public struct killBeKilledData 
	{
		public int _id;
		public short _killed;
		public short _beKilled;
		[MarshalAs(UnmanagedType.ByValArray,SizeConst=16)]
		public byte[] _name;
		
		public killBeKilledData(int id,short killed,short beKilled,byte[] name)
		{
			_id = id;
			_killed = killed;
			_beKilled = beKilled;
			_name = name;
		}
	}
	//结构体序列化
	[System.Serializable]
	//4字节对齐 iphone 和 android上可以1字节对齐
	[StructLayout(LayoutKind.Sequential, CharSet=CharSet.Ansi,Pack = 1)]
	public struct PAG_ListKill : IPackage
	{
		public short   header;
		public short   len;
		public uint     no;
		public  short HEAD{get{return JFTools.size(this.GetType());}}
		public  short LEN{get{return 0;}}
		public uint ID{get{ return (uint)MSG_ID.LISTKILL;}}
		
		public int _cnt;
		[MarshalAs(UnmanagedType.ByValArray, SizeConst =  256)]
		public byte[] _inner;
		
		public PAG_ListKill(byte cnt)
		{
			_cnt = cnt;
			_inner = new byte[0];
			header = JFTools.size(typeof(PAG_ListKill));
			len = 0;
			no = (uint)MSG_ID.LISTKILL;
		}
	}
}
