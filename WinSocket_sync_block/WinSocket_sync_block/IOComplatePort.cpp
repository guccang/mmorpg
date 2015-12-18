#include "stdafx.h"

#include "IOComplatePort.h"
#include "socketTools.h"
#include <stdio.h>
#include "StreamFix.h"
#include "GUGPackage.h"
#include "TcpReceiver.h"
#include "viewListTTT.h"
#include "ioTools.h"

using namespace GUGGAME;

DWORD WINAPI serverWorkerThread(LPVOID lpParam);

LPPER_IO_DATA getPerIOData();

void clearSession(LPPER_IO_DATA  perIOData);


void IOComplatePortEx()
{
	ViewList::Init();
	printf("=========================\nGAME SERVER VERIFY V1.1\n=========================\n");
	SOCKET s, sclient;
	HANDLE hCompPort;
	LPFN_ACCEPTEX lpfnAcceptEx = NULL;
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	GUID GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;
	LPFN_GETACCEPTEXSOCKADDRS lpfnGetAcceptExSockAddrs = NULL;

	DWORD	dwBytees = 0;
	int buflen = 1024;
	SYSTEM_INFO systemInfo;
	HANDLE threads[10];

	//WSAEVENT holdEvent = WSACreateEvent();

	hCompPort = CreateIoCompletionPort(
		INVALID_HANDLE_VALUE,
		NULL,
		(ULONG_PTR)0,
		0);

	// 2 获得cpu个数
	GetSystemInfo(&systemInfo);

	// 3 创建工作线程，并且与完成端口绑定
	for (int i = 0; i < (int)systemInfo.dwNumberOfProcessors; ++i)
	//for (int i = 0; i <1; ++i)
	{
		threads[i] = CreateThread(NULL, 0, serverWorkerThread, hCompPort, 0, NULL);

		//CloseHandle(threads[i]);
	}

	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 


	if (false == doBind(s, SERVER_PORT))
		return;

	if (false == doListen(s, 200))
		return;

	// load AcceptEx Function
	  if (SOCKET_ERROR == WSAIoctl(s,
		  SIO_GET_EXTENSION_FUNCTION_POINTER,
		  &GuidGetAcceptExSockAddrs,
            sizeof(GuidGetAcceptExSockAddrs), &lpfnGetAcceptExSockAddrs, sizeof(lpfnGetAcceptExSockAddrs),
            &dwBytees, NULL, NULL))
        {
			printf(" wsaioctl error:%d",WSAGetLastError());
			return;
        }
	  if (SOCKET_ERROR == WSAIoctl(s,
		  SIO_GET_EXTENSION_FUNCTION_POINTER,
		  &GuidAcceptEx,
		  sizeof(GuidAcceptEx),
		  &lpfnAcceptEx,
		  sizeof(lpfnAcceptEx),
		  &dwBytees,
		  NULL,
		  NULL
		  ))
	  {
		  printf(" wsaioctl error:%d", WSAGetLastError());
		  return;
	  }

	sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	LPPER_IO_DATA perIOData = getPerIOData();
	memset(perIOData, 0, sizeof(PER_IO_DATA));
	perIOData->dataBuf.buf = perIOData->buf;
	perIOData->dataBuf.len = DATA_BUF_SIZE;
	perIOData->client = sclient;
	perIOData->operationType = ACCEPT_POSTED;
	perIOData->lpfnAcceptEx = lpfnAcceptEx;
	perIOData->lpfnGetAcceptExSockAddrs = lpfnGetAcceptExSockAddrs;
	perIOData->listen = s;
	perIOData->hCompPort = hCompPort;
	accept_post(perIOData,0);

	CreateIoCompletionPort(
		(HANDLE)s,
		hCompPort,
		(ULONG_PTR)0,
		0
		);

	//WaitForSingleObject(holdEvent, INFINITE);
}

void IOComplatePort()
{
	HANDLE CompletionPort;
	SYSTEM_INFO systemInfo;

	// 1 创建完成断藕
	CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	// 2 获得cpu个数
	GetSystemInfo(&systemInfo);

	// 3 创建工作线程，并且与完成端口绑定
	for (int i = 0; i < (int)systemInfo.dwNumberOfProcessors; ++i)
	{
		HANDLE threadHandle;

		threadHandle = CreateThread(NULL, 0, serverWorkerThread, CompletionPort, 0, NULL);

		CloseHandle(threadHandle);
	}

	//4 监听socket
	SOCKET listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	if (false == doBind(listenSocket, SERVER_PORT))
		return;

	if (false == doListen(listenSocket, 100))
		return;


	while (true)
	{
		PER_HANDLE_DATA * perHandleData = NULL;
		SOCKADDR_IN saRemote;
		SOCKET acceptSocket;
		int remotelen;

		//5 接受连接
		remotelen = sizeof(saRemote);
		acceptSocket = WSAAccept(listenSocket, (SOCKADDR*)&saRemote, &remotelen,NULL,NULL);

		//6 单句柄数据 ,将单据并数据放入到overlapped数据中区。
		//perHandleData = (LPPER_HANDLE_DATA)GlobalAlloc(GPTR, sizeof(PER_HANDLE_DATA));
		//printf("Socket number %d connected",acceptSocket);
		//perHandleData->client = acceptSocket;
		//memcpy(&perHandleData->clientAddr, &saRemote, remotelen);

		//7 socket与完成端口绑定
		CreateIoCompletionPort((HANDLE)acceptSocket,
			CompletionPort, 0, 0);

		//8 重叠io投递，通知异步，数据异步
		// 当有数据且数据从内核空间copy到了用户空间WSABUF中的时候，工作线程被调用。
		LPPER_IO_DATA perIOData = getPerIOData();
		memcpy(&perIOData->clientAddr, &saRemote, remotelen);
		perIOData->client = acceptSocket;
		recv_post(perIOData, 0);
		

		//9 重叠io投递,，通知异步，数据异步
		perIOData = (LPPER_IO_DATA)GlobalAlloc(GPTR, sizeof(PER_IO_DATA));
		memcpy(&perIOData->clientAddr, &saRemote, remotelen);
		perIOData->client = acceptSocket;
		send_post(perIOData,0);
	}
}

DWORD WINAPI serverWorkerThread(LPVOID lpParam)
{
	
	HANDLE completionPort = (HANDLE)lpParam;
	DWORD bytesTransferred;
	LPPER_HANDLE_DATA perHandleData;
	LPPER_IO_DATA perIOData;
	DWORD flags = 0;
	int error = 0;

	while (true)
	{
		/*
		当重叠IO投递事件，完成时（此时数据已经从内核空间到用户空间WSABUF中）.
		GetQueuedCompletionStatus返回相关数据。
		perHandleData 数据
		overlapped 数据
		*/
		BOOL ret = GetQueuedCompletionStatus(
			completionPort,
			&bytesTransferred,
			(PULONG_PTR)&perHandleData,
			(LPOVERLAPPED*)&perIOData,
			INFINITE
			);
		if (false == ret)
		{
			printf("GetQueuedCompletionStatus failed %d\n",WSAGetLastError());
			clearSession(perIOData);
			continue;
		}

		if (bytesTransferred == 0 &&
			(perIOData->operationType == RECV_POSTED ||
			perIOData->operationType == SEND_POSTED))
		{
			printf("client closed %d\n", perIOData->client);
			clearSession(perIOData);
			continue;
		}


		//
		if (perIOData->operationType == RECV_POSTED)
		{
			recv_post(perIOData, bytesTransferred);
		}

		if (perIOData->operationType == SEND_POSTED)
		{
			static int cnt = 0;
			if (cnt%100==0)
				printf("send successs %d \n",++cnt);
			GlobalFree(perIOData);
		}

		if (perIOData->operationType == ACCEPT_POSTED)
		{
		

			// do....
			int err = setsockopt(perIOData->client,
				SOL_SOCKET,
				SO_UPDATE_ACCEPT_CONTEXT,
				(char *)&perIOData->listen,
				sizeof(perIOData->listen));
				
			printf("error setsockopt posted:%d \n", WSAGetLastError());

			SOCKADDR_IN remote;
			SOCKADDR_IN local;
			int remoteLen = sizeof(SOCKADDR_IN);
			int localLen = sizeof(SOCKADDR_IN);
			perIOData->lpfnGetAcceptExSockAddrs(
				perIOData->dataBuf.buf,
				0,	
				sizeof(SOCKADDR_IN) + 16,
				sizeof(SOCKADDR_IN) + 16, 
				(LPSOCKADDR*)&local,
				&localLen,
				(LPSOCKADDR*)&remote,
				&remoteLen);

		//	printf("remote form %s:%d\n", inet_ntoa(remote->sin_addr), ntohs(remote->sin_port));
		//	printf("local form %s:%d\n", inet_ntoa(local->sin_addr), ntohs(local->sin_port));

			if (0 != getpeername(perIOData->client, (SOCKADDR*)&remote, &remoteLen))
			{
				printf("getpeername error : %d\n",WSAGetLastError());
			}
		
			LPPER_IO_DATA pid = getPerIOData();
			pid->operationType = ACCEPT_POSTED;
			pid->client = perIOData->client;
			memcpy(&pid->clientAddr, &remote, remoteLen);
			recv_post(pid, 0);

			// new postrec
			SOCKET client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			perIOData->client = client;
			accept_post(perIOData, 0);

		}

	}
	return 0;
}

LPPER_IO_DATA getPerIOData()
{
	LPPER_IO_DATA perIOData = (LPPER_IO_DATA)GlobalAlloc(GPTR, sizeof(PER_IO_DATA));
	memset(perIOData, 0, sizeof(PER_IO_DATA));
	perIOData->dataBuf.buf = perIOData->buf;
	perIOData->dataBuf.len = DATA_BUF_SIZE;
	return perIOData;
}

void accept_post(LPPER_IO_DATA perIOData, DWORD bytesTransferred)
{
	printf("accept_post：%d:%d\n",perIOData->listen,perIOData->client);

	CreateIoCompletionPort(
		(HANDLE)perIOData->client,
		perIOData->hCompPort,
		(ULONG_PTR)0,
		0
		);

	DWORD recvBytes = 0;
	BOOL bRetVal = perIOData->lpfnAcceptEx(
		perIOData->listen,
		perIOData->client,
		perIOData->buf,
		0,
		sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16,
		&perIOData->recvBytesAccept,
		&perIOData->overLapped
		);
	if (bRetVal == FALSE) {
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			printf("AcceptEx failed with error: %u\n", WSAGetLastError());
			return;
		}
	}
}

void send_post(LPPER_IO_DATA perIOData, DWORD bytesTransferred)
{
	DWORD Flags = 0;
	ZeroMemory(&(perIOData->overLapped), sizeof(OVERLAPPED));
	//strcpy_s(perIOData->buf, "connect success");
	DWORD Bytes = bytesTransferred;
	perIOData->dataBuf.buf = perIOData->buf;
	perIOData->dataBuf.len = bytesTransferred;
	perIOData->operationType = SEND_POSTED;
	INT error = WSASend(perIOData->client, &perIOData->dataBuf, 1, &Bytes, Flags, &(perIOData->overLapped), NULL);
	if (error != 0 && WSAGetLastError() != WSA_IO_PENDING)
	{
		printf("WSASend failed socket=%d error=%d", perIOData->client,WSAGetLastError());
		clearSession(perIOData);
	}
}

//UTF8转成Unicode
wchar_t * UTF8ToUnicode(const char* str)
{
	int    textlen = 0;
	wchar_t * result;
	textlen = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
	result = (wchar_t *)malloc((textlen + 1)*sizeof(wchar_t));
	memset(result, 0, (textlen + 1)*sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, str, -1, (LPWSTR)result, textlen);
	return    result;
}

//Unicode转成ANSI
char * UnicodeToANSI(const wchar_t *str)
{
	char * result;
	int textlen = 0;
	// wide char to multi char
	textlen = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);
	result = (char *)malloc((textlen + 1)*sizeof(char));
	memset(result, 0, sizeof(char) * (textlen + 1));
	WideCharToMultiByte(CP_ACP, 0, str, -1, result, textlen, NULL, NULL);
	return result;
}

void recv_post(LPPER_IO_DATA perIOData, DWORD bytesTransferred)
{
	SOCKADDR_IN *addr = (SOCKADDR_IN*)&(perIOData->clientAddr);
	// 
	if (0 != bytesTransferred)
	{
		try
		{
			recvSize(perIOData, bytesTransferred, *(perIOData->stream));
		}
		catch (...)
		{
			printf("exception recvSize.");
		}

		//recvSizeNormal(perIOData, bytesTransferred);
	}
	
	//
	DWORD flags = 0, recvBytes=0;
	ZeroMemory(&(perIOData->overLapped), sizeof(OVERLAPPED));
	perIOData->dataBuf.len = DATA_BUF_SIZE;
	perIOData->dataBuf.buf = perIOData->buf;
	perIOData->operationType = RECV_POSTED;
	if (0 == bytesTransferred)
	{
		perIOData->streamBuf = new char[DATA_BUF_SIZE * 10];
		perIOData->stream = new LEUD::StreamFix(perIOData->streamBuf, DATA_BUF_SIZE * 10);
	}
	int error = WSARecv(perIOData->client,
		&(perIOData->dataBuf), 1,
		&recvBytes,
		&flags,
		&(perIOData->overLapped),
		NULL);
	if (error != 0 && WSAGetLastError() != WSA_IO_PENDING)
	{
		printf("WSARecv error %d", WSAGetLastError());
		clearSession(perIOData);
	}
}

void clearSession(LPPER_IO_DATA  perIOData)
{
	ViewList::remove((short)perIOData->client);
	closesocket(perIOData->client);
	//GlobalFree(perHandleData);
	GlobalFree(perIOData);
}

