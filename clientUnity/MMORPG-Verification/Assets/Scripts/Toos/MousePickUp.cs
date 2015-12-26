using UnityEngine;
using System.Collections;

public class MousePickUp 
{
	public static Map.position pickIsland(Map.position pos)
	{
		if(Input.GetMouseButtonUp(0))
		{
			Ray ray = Camera.main.ScreenPointToRay(Input.mousePosition);
			RaycastHit hit;
			
			if (Physics.Raycast(ray, out hit))
			{
				if(hit.transform.tag=="island")
					return Map.getCellPos(hit.transform.position);
			}
		}
		return pos;
	}
	public static Creature pick(Creature target)
	{
		if(Input.GetMouseButtonUp(0))
		{
			Ray ray = Camera.main.ScreenPointToRay(Input.mousePosition);
			RaycastHit hit;
			
			if (Physics.Raycast(ray, out hit))
			{

					Creature c = hit.transform.GetComponent<Creature>();
					return c;
				//if(c!=null)
				//{
				//	GameDebug.Log(hit.transform.name);
				//	return c;
				//}
			}
		}
		return target;
	}
}
