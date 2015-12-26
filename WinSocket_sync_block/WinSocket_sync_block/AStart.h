#pragma once
#include "GUCQueue.h"
#include "viewListTTT.h"
namespace GUGGAME
{
struct pathNode;

struct asNode
{
	short x;
	short z;
	short f;
	short h;
	short g;
	asNode* _next;
};

inline short abs(short x)
{
	return x < 0 ? -x : x;
}

void setNode(short sx,short sz,short nx, short nz, short tx, short tz,asNode& node)
{
	node.x = nx;
	node.z = nz;
	node.h = abs(tx - nx) + abs(tz - nz);
	node.g = abs(sx - nx) + abs(sz - nz);
	node.f = node.h + node.g;
}

bool noFind(queue<asNode> *que, int nx, int nz)
{
	asNode *theNode = que->front();
	while (theNode)
	{
		if (theNode->x == nx && theNode->z == nz)
		{
			return false;
		}
		theNode = theNode->_next;
	}
	return true;
}
bool notFind(queue<asNode> *open,queue<asNode> *close,int nx,int nz)
{
	if (noFind(open, nx, nz))
	{
		if (noFind(close, nx, nz))
			return true;
	}
	return false;
}

void insert(queue<asNode> *open, asNode* node)
{
			asNode *pre = open->front();
			if (NULL == pre)
			{
				open->push_front(node);
				return;
			}
			if (node->f <= pre->f)
			{
				open->push_front(node);
				return;
			}
			asNode *nx = pre->_next;
			if (NULL == nx)
			{
				open->push_back(node);
				return;
			}
			while (nx)
			{
					if (node->f <= nx->f)
					{
						pre->_next = node;
						node->_next = nx;
						break;
					}
					else
					{
						pre = nx;
						nx = nx->_next;
					}
			}
			
}

void AddNode(asNode* curNode,asNode *nexNode,short tx,short tz,queue<asNode>&open)
{
	setNode(curNode->x, curNode->z,nexNode->x, nexNode->z, tx, tz, *nexNode);
	insert(&open, nexNode);
}

inline bool isNotBlock(short x,short z)
{
	return !ViewList::isBlock(x,z);
}

int  findPath(short x, short z,short tx,short tz, pathNode* path)
{
	const int size = 30;
	asNode nodeArray[size];
	int index = 0;
	queue<asNode> open;
	queue<asNode> close;

	asNode &src = nodeArray[index++];
	src.x = x; 
	src.z = z;
	open.push_back(&src);

	asNode* node = NULL;
	while (node = open.front())
	{
		open.pop_front();
		if (noFind(&close,node->x,node->z))	close.push_back(node);
		if (node->x == tx && node->z == tx ||index>=size-8)
			break;
		//getCanMove(*node);
		short nx, nz;
		asNode *newNode = NULL;
		nx = node->x + 1;
		nz = node->z;
		if (notFind(&open, &close, nx, nz) && isNotBlock(nx, nz))
		{
			newNode = &nodeArray[index++];
			setNode(node->x, node->z, nx, nz, tx, tz, *newNode);
			insert(&open, newNode);
		}
		nx = node->x - 1;
		nz = node->z;
		if (notFind(&open, &close, nx, nz) && isNotBlock(nx, nz))
		{
			newNode = &nodeArray[index++];
			setNode(node->x, node->z, nx, nz, tx, tz, *newNode);
			insert(&open, newNode);
		}
		nx = node->x;
		nz = node->z + 1;
		if (notFind(&open, &close, nx, nz) && isNotBlock(nx, nz))
		{
			newNode = &nodeArray[index++];
			setNode(node->x, node->z, nx, nz, tx, tz, *newNode);
			insert(&open, newNode);
			
		}
		nx = node->x;
		nz = node->z - 1;
		if (notFind(&open, &close, nx, nz) && isNotBlock(nx, nz))
		{
			newNode = &nodeArray[index++];
			setNode(node->x, node->z, nx, nz, tx, tz, *newNode);
			insert(&open, newNode);
		}
		nx = node->x + 1;
		nz = node->z + 1;
		if (notFind(&open, &close, nx, nz) && isNotBlock(nx, nz))
		{
			newNode = &nodeArray[index++];
			setNode(node->x, node->z, nx, nz, tx, tz, *newNode);
			insert(&open, newNode);
		}
		nx = node->x - 1;
		nz = node->z - 1;
		if (notFind(&open, &close, nx, nz) && isNotBlock(nx, nz))
		{
			newNode = &nodeArray[index++];
			setNode(node->x, node->z, nx, nz, tx, tz, *newNode);
			insert(&open, newNode);
		}
		nx = node->x + 1;
		nz = node->z - 1;
		if (notFind(&open, &close, nx, nz) && isNotBlock(nx, nz))
		{
			newNode = &nodeArray[index++];
			setNode(node->x, node->z, nx, nz, tx, tz, *newNode);
			insert(&open, newNode);
		}
		nx = node->x - 1;
		nz = node->z + 1;
		if (notFind(&open, &close, nx, nz) && isNotBlock(nx, nz))
		{
			newNode = &nodeArray[index++];
			setNode(node->x, node->z, nx, nz, tx, tz, *newNode);
			insert(&open, newNode);
		}

	}

	int i = 0;
	while ((node = close.front())&&i<size-1)
	{
		close.pop_front();
		path[i].x = node->x;
		path[i].z = node->z;
		i++;
	}
	if (i>20)
		printf("path node : %d\n", i);

	return i;
}

}
