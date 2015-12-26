using UnityEngine;
using System.Collections;

public class GameDebug  {
	public static string lastError = "";
	public static void Log(object str)
	{
		Debug.Log(str);
	}

	public static void LogError(object str)
	{
		Debug.LogError(str);
		lastError = str.ToString();
	}
}
