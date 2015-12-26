using UnityEngine;
using System.Collections;

public class area : MonoBehaviour {
	/*
the call of OnEnable is fast than Init...why...  
because: Init function 
first : set vals
laster: this.enabled = true;
	 */
	public void Init(string id)
	{
		_id = id;
		this.enabled = true;
	}

	void Awake()
	{
	//	this.enabled = false; 
	}
	// Use this for initialization
	void Start () {

	}

	void OnEnable()
	{
		Creature c = PlayerSys.getSingleton().getHero().getCreature(_id);
		if(c!=null&&false==c.isDead())
		{
			_beforeColor = c.color;
			c.chgColor("area");
			t = 1.0f;
		}
		else
		{
			this.enabled = false;
		}
	}

	float t=1.0f;
	string _id = "";
	string _beforeColor = "";

	// Update is called once per frame
	void Update () {
		t -= Time.deltaTime;
		if(t<=0)
		{
			Creature c = PlayerSys.getSingleton().getHero().getCreature(_id);
			if(c!=null&&false==c.isDead())
			{
				c.chgColor(_beforeColor);
			}
			this.enabled = false;
		}
	}

}
