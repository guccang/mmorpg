using UnityEngine;
using System.Collections;
using unityNetCPlusPlus;

public class CPlusPlus : MonoBehaviour {

	// Use this for initialization
	void Start () {
		var c = new Class1();
		GameDebug.Log(c.Add(1,5));
	}
	
	// Update is called once per frame
	void Update () {
	
	}
}
