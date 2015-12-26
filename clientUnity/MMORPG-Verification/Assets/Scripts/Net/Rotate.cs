using UnityEngine;
using System.Collections;

public class Rotate : MonoBehaviour {
	public float _speed = 30.0f;
	// Use this for initialization
	void Start () {
	
	}
	
	// Update is called once per frame
	void Update () {
		float r = Time.time * _speed;
		if(r>360f)
			r %= 360f;
		this.transform.rotation = Quaternion.Euler(r,r,r);
	}
}
