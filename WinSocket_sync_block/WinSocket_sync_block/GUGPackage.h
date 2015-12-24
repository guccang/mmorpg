#pragma once

namespace GUGGAME 
{
	enum MSG_ID
	{
		MSGID_HEADER = 0,
		MSGID_Error = 1,
		MSGID_MAPINFO = 2,

		MSGID__START_ = 999,
		MSGID_TEST_STRING = 1000, // STRING...
		MSGID_TEST_STRUCTURE = 1001, // STRUCT...
		MSGID_TEST_ARRAY = 1002,// ARRAY...
		MSGID_CONNECTED = 1003,
		MSGID_ATTRCHG = 1004,
		MSGID_ATTR = 1005,
		MSGID_JUMPINMAP = 1006,

		//LOGIC
		MSGID_WALK = 2000,
		MSGID_FIGHT = 2001,
		MSGID_CREATE = 2002,
		MSGID_DELETEOBJ = 2003,
		MSGID_ENETRMAP = 2004,

		// Login
		MSGID_LOGIN = 3000,
		MSGID_REGIST = 3001,
	};

	enum ERROR_CODE
	{
		OK = 0,
		ERROR_LOGIN_NOT_REGIST = 1,
		ERROR_LOGIN_ONLINE = 2,
		ERROR_NAME_PWD_NULL = 3,
		ERROR_NOT_ENOUGH_MP = 4,
		ERROR_FIGHT_TARGET_NULL = 5,
		ERROR_HAD_REGIST=6,
		ERROR_FIGHT_TARGET_DEAD = 7,
		ERROR_ENTER_MAP_NOT_LOGIN = 8,
	};

	const int HEAD_LEN = 4;
	size_t HeadLen();
	void MakeHead(char* buf, unsigned short len);
	int MakeHeadEx(char* buf, unsigned short len);
	unsigned short FindHead(const char* buf);
	unsigned short FindHeadEx(const char* buf);

#pragma pack(push)
#pragma pack(1)
	template<class T>  inline public T&  strToStream(T& stream,const char* str, int size)
	{
		stream << str;
		int l = strlen(str) + 1;
		for (int i = 0; i < size - l; ++i)
		{
			stream << '\0';
		}
		return stream;
	}
	template<class T> inline public T&  streamToStr(T& stream, char* str, int size)
	{
		stream >> str;
		int l = strlen(str) + 1;
		char c;
		for (int i = 0; i < size - l; ++i)
		{
			stream >> c;
		}
		return stream;
	}

	struct HeartBeat
	{
		int  no;
		char info[20];
	};
	template<class T> inline T& operator<<(T& stream, const HeartBeat& data)
	{
		stream << data.no;
		return stream;
	}
	template<class T> inline T& operator>>(T& stream, HeartBeat& data)
	{
		stream >> data.no;
		return stream;
	}


	struct Head
	{
		short head;
		short len;
	};
	template<class T> inline T& operator<<(T& stream, const Head& data)
	{
		stream << data.head << data.len;
		return stream;
	}
	template<class T> inline T& operator>>(T& stream, Head& data)
	{
		stream >> data.head >> data.len;
		return stream;
	}
	
	struct test02
	{
		unsigned int no;
		char dest[10];
		int id;
		char name[20];
		char sex5[5];

		test02()
		{
			no = 1001;
		}
	};
	template<class T> inline T& operator<<(T& stream, const test02& data)
	{
		stream <<  data.no;
		strToStream(stream, (char*)data.dest, 10) << data.id;
		strToStream(stream, (char*)data.name, 20);
		strToStream(stream, (char*)data.sex5, 5);
		return stream;
	}
	template<class T> inline T& operator>>(T& stream, test02& data)
	{
		stream >> data.no;
		streamToStr(stream, (char*)data.dest, 10) >> data.id;
		streamToStr(stream, (char*)data.name, 20);
		streamToStr(stream, (char*)data.sex5, 5);
		return stream;
	}

	struct test03Inner
	{
		char dest[10];
		int id;
		char name[20];
		char sex5[5];
	};
	template<class T> inline T& operator<<(T& stream, const test03Inner& data)
	{
		strToStream(stream, (char*)data.dest, 10) << data.id;
		strToStream(stream, (char*)data.name, 20);
		strToStream(stream, (char*)data.sex5, 5);
		return stream;
	}
	template<class T> inline T& operator>>(T& stream, test03Inner& data)
	{
		streamToStr(stream, (char*)data.dest, 10) >> data.id;
		streamToStr(stream, (char*)data.name, 20);
		streamToStr(stream, (char*)data.sex5, 5);
		return stream;
	}
	struct test03
	{
		unsigned int no;
		unsigned int size;
		test03Inner data[5];

		test03()
		{
			no = 1002;
		}

	};
	template<class T> inline T& operator<<(T& stream, const test03& data)
	{
		//short head = sizeof(data) - (5 - data.size) * sizeof(test03Inner);
		stream << data.no << data.size;
		for (int i = 0; i < (int)data.size; ++i)
		{
			stream << data.data[i];
		}
		return stream;
	}
	template<class T> inline T& operator>>(T& stream, test03& data)
	{
		stream >> data.no>>data.size;
		for (int i = 0; i < (int)data.size; ++i)
		{
			stream >> data.data[i];
		}
		return stream;
	}
	struct Pakcage64
	{
	public:
		unsigned int no;
		Pakcage64()
		{
			no = 1000;
		}
		char data[64];
	public:
		int size()
		{
			return sizeof(Pakcage64);
		}
		char* buf()
		{
			return data;
		}
	};
	template<class T> inline T& operator<<(T& stream, const Pakcage64& data)
	{
		stream << data.no;
		strToStream(stream, data.data, 64);
		return stream;
	}
	template<class T> inline T& operator>>(T& stream, Pakcage64& data)
	{
		stream >> data.no;
		streamToStr(stream, data.data, 64);
		return stream;
	}

	struct Walk
	{
		unsigned int no;
		int id;
		short x;
		short z;
		char  dir;
		Walk()
		{
			no = 2000;
		}
	};
	template<class T> inline T& operator<<(T& stream, const Walk& data)
	{
		stream << data.no << data.id << data.x << data.z << data.dir;
		return stream;
	}
	template<class T> inline T& operator>>(T& stream, Walk& data)
	{
		stream >> data.no >> data.id >> data.x >> data.z >> data.dir;
		return stream;
	}

	struct Fight
	{
		unsigned int no;
		int attackerID;
		int targetID;
		short action;
		short parm01;
		short parm02;
		Fight()
		{
			no = 2001;
		}
	};
	template<class T> inline T& operator<<(T& stream, const Fight& data)
	{
		stream << data.no << data.attackerID << data.targetID << data.action << data.parm01 << data.parm02;
		return stream;
	}
	template<class T> inline T& operator>>(T& stream, Fight& data)
	{
		stream >> data.no >> data.attackerID >>  data.targetID >>data.action >> data.parm01 >> data.parm02;
		return stream;
	}

	struct Login
	{
		unsigned int no;
		char name[16];
		char pwd[32];
		Login()
		{
			no = 3000;
		}
	};
	template<class T> inline T& operator<<(T& stream, const Login& data)
	{
		stream << data.no;
		strToStream(stream, data.name, 16);
		strToStream(stream, data.pwd, 32);
		return stream;
	}
	template<class T> inline T& operator>>(T& stream, Login& data)
	{
		stream >> data.no;
		streamToStr(stream, data.name, 16);
		streamToStr(stream,data.pwd,32);
		return stream;
	}

	struct Register
	{
		unsigned int no;
		char name[16];
		char pwd[32];
		Register()
		{
			no = 3001;
		}
	};
	template<class T> inline T& operator<<(T& stream, const Register& data)
	{
		stream << data.no;
		strToStream(stream, data.name, 16);
		strToStream(stream, data.pwd, 32);
		return stream;
	}
	template<class T> inline T& operator>>(T& stream, Register& data)
	{
		stream >> data.no;
		streamToStr(stream, data.name, 16);
		streamToStr(stream, data.pwd, 32);
		return stream;
	}


	struct CreateObj
	{
		unsigned int no;
		int id;
		short type;
		short x;
		short z;
		char  dir;
		int   hp;
		char  name[16];
		CreateObj()
		{
			no = 2002;
		}
	};
	template<class T> inline T& operator<<(T& stream, const CreateObj& data)
	{
		stream << data.no << data.id << data.type << data.x << data.z << data.dir << data.hp;
		strToStream(stream, data.name, 16);
		return stream;
	}
	template<class T> inline T& operator>>(T& stream, CreateObj& data)
	{
		stream >> data.no >> data.id >> data.type >> data.x >> data.z >> data.dir >> data.hp;
		streamToStr(stream, data.name, 16);
		return stream;
	}


	struct DeleteObj
	{
		unsigned int no;
		int id;
		DeleteObj()
		{
			no = 2003;
		}
	};
	template<class T> inline T& operator<<(T& stream, const DeleteObj& data)
	{
		stream << data.no << data.id;
		return stream;
	}
	template<class T> inline T& operator>>(T& stream, DeleteObj& data)
	{
		stream >> data.no >> data.id;
		return stream;
	}

	struct ErrorCode 
	{
		unsigned int no;
		short msgid;
		int errorcode;
		ErrorCode()
		{
			no = 1;
		}
	};
	template<class T> inline T& operator<<(T& stream, const ErrorCode& data)
	{
		stream << data.no << data.msgid << data.errorcode;
		return stream;
	}
	template<class T> inline T& operator>>(T& stream, ErrorCode& data)
	{
		stream >> data.no >> data.msgid >> data.errorcode;
		return stream;
	}

	struct AttrChg
	{
		unsigned int no;
		int id;
		short type;
		short num;
		short delay; // ms
		AttrChg()
		{
			no = 1004;
			delay = 0;
		}
	};
	template<class T> inline T& operator<<(T& stream, const AttrChg& data)
	{
		stream << data.no <<  data.id << data.type << data.num << data.delay;
		return stream;
	}
	template<class T> inline T& operator>>(T& stream, AttrChg& data)
	{
		stream >> data.no >> >> data.id >> data.type >> data.num >> data.delay;
		return stream;
	}

	struct Attr
	{
		unsigned int no;
		int id;
		int hp;
		int mp;
		int def;
		Attr()
		{
			no = 1005;
		}
	};
	template<class T> inline T& operator<<(T& stream, const Attr& data)
	{
		stream << data.no << data.id << data.hp << data.mp << data.def;
		return stream;
	}
	template<class T> inline T& operator>>(T& stream, Attr& data)
	{
		stream >> data.no >> >> data.id >> data.hp >> data.mp >> data.def;
		return stream;
	}

	struct JumpInMap
	{
		unsigned int no;
		int id;
		short x;
		short z;
		char dir;
		JumpInMap()
		{
			no = 1006;
		}
	};
	template<class T> inline T& operator<<(T& stream, const JumpInMap& data)
	{
		stream << data.no << data.id << data.x << data.z << data.dir;
		return stream;
	}
	template<class T> inline T& operator>>(T& stream, JumpInMap& data)
	{
		stream >> data.no >> >> data.id >> data.x >> data.z >> data.dir;
		return stream;
	}

	struct MapInfo
	{
		unsigned int no;
		short x;
		short z;
		char  d;
		MapInfo()
		{
			no = MSGID_MAPINFO;
			x = 0;
			z = 0;
			d = 0;
		}
	};
	template<class T> inline T& operator<<(T& stream, const MapInfo& data)
	{
		stream << data.no << data.x << data.z << data.d;
		return stream;
	}
	template<class T> inline T& operator>>(T& stream, MapInfo& data)
	{
		stream >> data.no >> data.x << data.z << data.d;
		return stream;
	}

	struct EnterMap
	{
		unsigned int no;
		short mapID;
		EnterMap()
		{
			no = MSGID_ENETRMAP;
			mapID = 0;
		}
	};
	template<class T> inline T& operator<<(T& stream, const EnterMap& data)
	{
		stream << data.no << data.mapID;
		return stream;
	}
	template<class T> inline T& operator>>(T& stream, EnterMap& data)
	{
		stream >> data.no >> data.mapID;
		return stream;
	}
#pragma pack(pop)

}