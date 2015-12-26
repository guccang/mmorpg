using UnityEngine;
using System.Collections;
#if UNITY_EDITOR
using UnityEditor;
#endif

public class ResMgr  : Singleton<ResMgr>
{
	Object objBLock;
	 Object objCube;
	Object objMaster;
	 Material materialHero;
	Material MaterialTarget;
	Material MaterialNormal;
	Material MaterialDead;
	Material MaterialArea;
	Material MaterialGreen;
	Object bullet;

	public T loadPrefabs<T>(string file) where T : Object
	{
#if UNITY_EDITOR
		string path = "Assets/Resources/" +  file ;
		return (T)AssetDatabase.LoadAssetAtPath(path,typeof(T));
#else
		file = file.Substring(0,file.IndexOf('.'));
		return (T)Resources.Load(file,typeof(T));
#endif
	}


	public void Init()
	{
		if(objBLock == null)
			objBLock = ResMgr.getSingleton().loadPrefabs<Object>("block.prefab");
		if(objMaster==null)
			objMaster = ResMgr.getSingleton().loadPrefabs<Object>("master.prefab");
		if(objCube == null)
			objCube  = ResMgr.getSingleton().loadPrefabs<Object>("Cube.prefab");
		if(materialHero == null)
			materialHero = (Material)ResMgr.getSingleton().loadPrefabs<Material>("heroMat.mat");
		if(bullet == null)
			bullet = ResMgr.getSingleton().loadPrefabs<Object>("bullet.prefab");
		if(MaterialTarget == null)
			MaterialTarget =  ResMgr.getSingleton().loadPrefabs<Material>("targetMat.mat");
		if(MaterialNormal == null)
			MaterialNormal = ResMgr.getSingleton().loadPrefabs<Material>("normalMat.mat");
		if(MaterialDead == null)
			MaterialDead = ResMgr.getSingleton().loadPrefabs<Material>("deadMat.mat");
		if(MaterialArea==null)
			MaterialArea = ResMgr.getSingleton().loadPrefabs<Material>("areaMat.mat");
		if(MaterialGreen == null)
			MaterialGreen = ResMgr.getSingleton().loadPrefabs<Material>("greenMat.mat");
	}

	public GameObject getCube()
	{
		GameObject obj = (GameObject)GameObject.Instantiate(objCube);
		return obj;
	}

	public GameObject getBlock()
	{
		GameObject obj = (GameObject)GameObject.Instantiate(objBLock);
		return obj;
	}

	public Material getmat()
	{
		return materialHero;
	}

	public Material getMatTarget()
	{
		return MaterialTarget;
	}
	public Material getMatNormal()
	{
		return MaterialNormal;
	}
	public Material getMatDead()
	{
		return MaterialDead;
	}
	public Material getMatArea()
	{
		return MaterialArea;
	}
	public Material getGreenMat()
	{
		return MaterialGreen;
	}
	public GameObject getMaster()
	{
		GameObject obj = (GameObject)GameObject.Instantiate(objMaster);
		if(obj.GetComponent<Creature>()==null)
			obj.AddComponent<Creature>();
		return obj;
	}
	public GameObject getBullet()
	{
		GameObject obj = (GameObject)GameObject.Instantiate(bullet);
		if(obj.GetComponent<bullet>()== null)
			obj.AddComponent<bullet>();
		return obj;
	}
}

