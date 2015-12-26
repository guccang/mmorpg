using UnityEngine;
using System.Collections;

public class Map {
	public class position
	{
		public int _x,_z;
		public int _dir;
		public position(int x,int z)
		{
			_x = x;
			_z = z;
		}
		public override int GetHashCode()
		{ return base.GetHashCode(); }
		public override bool Equals(object pos)
		{
			position o = (position)pos;
			return (_x == o._x) && (_z==o._z);
		}
		public static bool operator ==(position lhs, position rhs)
		{
			return  lhs.Equals(rhs);
		}
		public static bool operator !=(position lhs, position rhs)
		{
			return !lhs.Equals(rhs);
		}
		
		public static position operator +(position lhs, position rhs)
		{
			lhs._x += rhs._x;
			lhs._z += rhs._z;
			return lhs;
		}
	}

	public enum Direction
	{
		UP,
		UP_RIGHT,
		RIGHT,
		RIGHT_DOWN,
		DOWN,
		DOWN_LELF,
		LEFT,
		LEFT_UP,
	}

	public static position invalidPos = new Map.position(int.MaxValue,int.MaxValue) ;
	/*
	1 4 7
	2 5 8
	3 6 9
	 */
	static int stepOne = 3;
	public static position[] Direction2Position =
	{
		new position(0,stepOne),
		new position(stepOne,stepOne),
		new position(stepOne,0),
		new position(stepOne,-stepOne),
		new position(0,-stepOne),
		new position(-stepOne,-stepOne),
		new position(-stepOne,0),
		new position(-stepOne,stepOne),
	};

	static float detl = 1.0f;

	public static Vector3 getWorldPos(Map.position pos)
	{
		return new Vector3(pos._x*detl,0,pos._z*detl);
	}

	public static position getCellPos(Vector3 cellPos)
	{
		return new Map.position(Mathf.CeilToInt(cellPos.x/detl),Mathf.CeilToInt(cellPos.z/detl));
	}
}
