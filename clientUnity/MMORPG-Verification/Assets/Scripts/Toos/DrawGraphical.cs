using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class DrawGrahical  : Singleton<DrawGrahical>
{
	LineRenderer linerRenderer;
	public GameObject LineGameObject; 
	public void DrawCircle(float x,float z,int radius,GameObject lgo)
	{
		LineGameObject = lgo;
		linerRenderer = LineGameObject.GetComponent<LineRenderer>();
	//	linerRenderer.materials[0] = ResMgr.getSingleton().getMatDead();
		linerRenderer.SetWidth(0.08f,0.08f);
		linerRenderer.SetColors(Color.red,Color.white);

		int fragment = 60;
		float angleDel = 360.0f / fragment;
		List<Vector3> points = new List<Vector3>();		
		for(int i=0; i< fragment;++i)
		{
			float px = x + radius * Mathf.Cos(i*angleDel);
			float pz = z + radius * Mathf.Sin(i*angleDel);
			points.Add(new Vector3(px,0,pz));
		}

		linerRenderer.SetVertexCount(fragment);
		for(int i=0; i<points.Count; ++i)
		{
			linerRenderer.SetPosition(i,points[i]);
		}
	}
}
