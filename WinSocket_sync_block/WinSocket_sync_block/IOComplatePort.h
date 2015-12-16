#pragma  once
#include <WinSock2.h>
#include <Windows.h>
#include <MSWSOCK.h>
#include "StreamFix.h"


#define DATA_BUF_SIZE 1024
enum OPERATOR_TYPE
{
	RECV_POSTED,
	SEND_POSTED,
	ACCEPT_POSTED,
};

typedef struct _PER_HANDLE_DATA
{
	SOCKET client;
	SOCKADDR_STORAGE clientAddr;
}PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

typedef struct
{
	OVERLAPPED overLapped;
	SOCKET listen;
	SOCKET client;
	SOCKADDR_STORAGE clientAddr;
	WSABUF dataBuf;
	char buf[DATA_BUF_SIZE];
	int operationType;
	LPFN_ACCEPTEX lpfnAcceptEx;
	LPFN_GETACCEPTEXSOCKADDRS  lpfnGetAcceptExSockAddrs;
	DWORD recvBytesAccept;
	HANDLE hCompPort;
	LEUD::StreamFix *stream;
	char *streamBuf;
}PER_IO_DATA, *LPPER_IO_DATA;


void recv_post(LPPER_IO_DATA perIOData, DWORD bytesTransferred);
void send_post(LPPER_IO_DATA perIOData, DWORD bytesTransferred);
void accept_post(LPPER_IO_DATA perIOData, DWORD bytesTransferred);

void IOComplatePort();
void IOComplatePortEx();