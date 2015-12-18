#pragma once
//////////////////////////////////////////////////////////////////////////
// 2015/12/16
// 1:游戏中的实体，只要是游戏中有概念的东西。都继承他
// 2:统一行为管理
//////////////////////////////////////////////////////////////////////////

struct vector2D
{
	int x;
	int z;
};

class GameObject
{
};

class MapObject : public GameObject
{
protected:
	int id;
};

class MovingObj : public MapObject
{
protected:
	vector2D velocity;
	vector2D pos;
};

class Player : public MovingObj
{

};

class Master : public MovingObj
{

};