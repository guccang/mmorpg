using UnityEngine;
using System.Collections;

public class Singleton<T>  where T : new()
{
	static T _ins = default(T);

	public static T getSingleton()
	{
		if(_ins == null)
		{
			_ins = new T();
		}
		return _ins;
	}
}
