#pragma once
//////////////////////////////////////////////////////////////////////////
// 2015/12/16
// 1:��Ϸ�е�ʵ�壬ֻҪ����Ϸ���и���Ķ��������̳���
// 2:ͳһ��Ϊ����
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