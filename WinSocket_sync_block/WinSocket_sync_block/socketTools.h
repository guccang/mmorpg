#ifndef SOCKET_TOOLS_H
#define SOCKET_TOOLS_H

#include <WinSock2.h>
// tcp/ip

bool WSALibriaryInit();

bool WSALibriaryRelease();

SOCKET getSocket();

bool doBind(SOCKET listenSock, unsigned short port);

bool doListen(SOCKET listenSock, int clientMax = 64);

#define SERVER_PORT 5150
#endif