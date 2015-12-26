using UnityEngine;
using System.Collections;

public class bullet : MonoBehaviour {

	public void Init(Creature attack,Creature target,int action)
	{
		Vector3 dir = target.transform.position - attack.transform.position;
		dir = dir.normalized;
		_dir = dir;
		_attack = attack;
		_target = target;
		_action = action;
		if(action == 1)
		{
			move = true;
			distanceToTarget = Vector3.Distance (this.transform.position, _target.transform.position);  
			StartCoroutine (Shoot ());  
		}
	}

	Vector3 _dir;
	int _action;
	Creature _attack;
	Creature _target;
 
	void Start()
	{
	}
	// Update is called once per frame
	float deleteTime =4;
	float speed = 10;
	void Update () {
		deleteTime -= Time.deltaTime;

		float  tt = speed * Time.deltaTime;
		if(_action==0) // normal
		{
			this.transform.position += new Vector3(_dir.x * tt,0,_dir.z*tt) ;
			if(_target)
			{
				if(Vector3.Distance(this.transform.position,_target.transform.position)<0.5f)
					Destroy(this.gameObject);
			}
		}
		else if (_action == 1) // magic
		{

		}

		if(deleteTime<0)
			Destroy(this.gameObject);
	}

	bool move =false;
	float distanceToTarget = 0;
	IEnumerator Shoot ()  
	{  
		while (move&&_target!=null) {  
			Vector3 targetPos = _target.transform.position;  
			this.transform.LookAt (targetPos);  
			float angle = Mathf.Min (1, Vector3.Distance (this.transform.position, targetPos) /  distanceToTarget) * 45;  
			this.transform.rotation = this.transform.rotation * Quaternion.Euler (Mathf.Clamp (-angle, -42, 42), 0, 0);  
			float currentDist = Vector3.Distance (this.transform.position, _target.transform.position);  
			//print ("currentDist" + currentDist);  
			if (currentDist < 0.5f)  
				move = false;  
			this.transform.Translate (Vector3.forward * Mathf.Min (speed * Time.deltaTime, currentDist));  
			yield return null;  
		}
		Destroy(this.gameObject);
	}  
}
