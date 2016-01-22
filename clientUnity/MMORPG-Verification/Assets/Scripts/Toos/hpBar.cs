using UnityEngine;  
using System.Collections;  

public class hpBar : MonoBehaviour {  

	public Transform obj;
	public Transform ui;
	public Vector3 Fomat ;
	public GUITexture textureHp,textureBk;
	public int valueHP=0;
	public int maxHP = 100;
	void Start()
	{
	}

	void Update()
	{
		Vector3 newFomat = Fomat / Vector3.Distance(obj.position,Camera.main.transform.position);
		textureHp.transform.position = WorldToUI(obj.position);
		textureBk.transform.position = textureHp.transform.position;
		//ui.localScale = newFomat;
		int hp = (int)(((float)valueHP/(float)maxHP)*maxHP)-56;
		GameDebug.Log("info: HP MaxHP:"+maxHP+":"+hp);
		SetGUITextureWidth(textureHp,0);	
		SetGUITextureWidth(textureBk,100);
		if(Input.GetKey(KeyCode.W))
			obj.Translate(Vector3.forward);
		if(Input.GetKey(KeyCode.S))
			obj.Translate(Vector3.back);
	}

private void SetGUITextureWidth(GUITexture mTexture,int mValue)
  {
	mTexture.pixelInset=new Rect(mTexture.pixelInset.x,mTexture.pixelInset.y,
	mValue,mTexture.pixelInset.height);
}

	public static Vector3 WorldToUI(Vector3 point)
	{
		Vector3 pt = Camera.main.WorldToScreenPoint(point);
		pt = new Vector3(pt.x/Screen.width,pt.y/Screen.height,0);
		pt.z = 0;
		return pt;
	}
}  