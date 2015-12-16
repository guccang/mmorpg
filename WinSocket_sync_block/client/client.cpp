// client.cpp : 定义控制台应用程序的入口点。
//
#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <string>
#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")



int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET s;
	SOCKADDR_IN serverAddress;
	int serverPort = 5150;
	
	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	serverAddress.sin_family = PF_INET;
	serverAddress.sin_port = htons(serverPort);
	serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

	std::cout<<"press any key to connect.\n";
	getchar();
	connect(s, (SOCKADDR*)&serverAddress, sizeof(serverAddress));
	
	
	std::string sendInfo;
	char*buf = new char[512];
	do
	{
		std::cout << ("press input sendinfo :") << std::endl;
		gets_s(buf,20);
		send(s, buf, strlen(buf)+1, 0);
		std::cout << "send:" << buf << std::endl;

		char *buf = new char[512];
		recv(s, buf, 512, 0);
		std::cout << ("recv:%s\n", buf);

	} while (strncmp(buf,"exit",512)!=0);

	closesocket(s);
	WSACleanup();

	std::cout << ("press any key to exit.");
	getchar();

	return 0;
}

