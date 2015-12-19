#include "stdafx.h"
#include "socketTools.h"
#include <winsock2.h>
#include <stdio.h>

bool WSALibriaryInit()
{
	WSADATA wsaData;
	int error = 0;

	// init winsock
	if ((error = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0)
	{
		printf("winsock init failed, error code:%d", error);
		return false;
	}

	return true;
}

bool WSALibriaryRelease()
{
	// clean
	if (WSACleanup() == SOCKET_ERROR)
	{
		printf("winsock clean up failed,error code:%d", WSAGetLastError());
		return false;
	}
	return true;
}

SOCKET getSocket()
{
	SOCKET listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSock == INVALID_SOCKET)
	{
		printf("socket create falied.error %d", WSAGetLastError());
		return INVALID_SOCKET;
	}
	return listenSock;
}

 bool doListen(SOCKET listenSock, int clientMax)
{
	if (listen(listenSock, clientMax)  == SOCKET_ERROR)
	{
		printf("listen error %d", WSAGetLastError());
		return false;
	}
	return true;
}

bool doBind(SOCKET listenSock, unsigned short port)
{
	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(ADDR_ANY);
	serverAddr.sin_port = htons(port);
	int error = bind(listenSock, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (error == SOCKET_ERROR)
	{
		printf("bind error %d", WSAGetLastError());
		return false;
	}
	return true;
}