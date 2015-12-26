// WinSocket_sync_block.cpp : �������̨Ӧ�ó������ڵ㡣
//
// winsock i/o ģ��
// select
// WSAAsyncSelect
// WSAEventSelect
//
//
// ���������ioģ�ͳ��������winsock i/o ģ��
// �����������������ݿ���ͬ�������ݿ����첽

#include "stdafx.h"
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")

#include <windows.h> 
#include "overLapped.h"
#include "IOComplatePort.h"
#include "viewListTTT.h"
#include <exception>
void NonBlock();
void Block();
void Update(float t);
void selectMode();


void WSAAsyncSelectMode();
BOOL InitApplication(HINSTANCE hinstance);
BOOL InitInstance(HINSTANCE hinstance, int nCmdShow);
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
#define  WM_SOCKET (WM_USER+1000)


void WSAEventSelectMode();
const int MAX_CLIENT = 3;// WSA_MAXIMUM_WAIT_EVENTS;
HANDLE ev[64];

struct session
{
	SOCKET client;
	SOCKADDR_IN addr;
};

int _tmain(int argc, _TCHAR* argv[])
{
	printf("=========================\n");
	printf("GAME SERVER VERIFY V2.0\n");
	printf("=========================\n");

	argc = argc;
	argv = argv;

	WSADATA wsaData;
	int error = 0;

	// init winsock
	if ((error = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0)
	{
		printf("winsock init failed, error code:%d",error );
		return -1;
	}

	NonBlock();

	// WinSock Block 
	//block();
	DWORD size = 0;
	HANDLE endEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	ev[size++] = endEvent;
	ev[size++] = CreateEvent(NULL, FALSE, FALSE, NULL);

	// WinSock NonBlock
	float t = 0;
	while (WAIT_OBJECT_0 != WaitForMultipleObjects(size,ev,FALSE,20))
	{
		Update(t+=1);
	}

	// clean
	if (WSACleanup() == SOCKET_ERROR)
	{
		printf("winsock clean up failed,error code:%d",WSAGetLastError());
		return -1;
	}
	return 0;
}

void Update(float t)
{
	//if (t > 200)
	//	SetEvent(ev[0]);
	//printf("Update....%f\n",t);
	try
	{
		ViewList::Update(t);
	}
	catch (...)
	{
		printf("not fixed memroy exception:");
	}
}
/*
1:������ģ��
select
WSAAsyncSelect
WSAEventSelect
*/
void NonBlock()
{
	//selectMode();
	//WSAAsyncSelectMode();
	//WSAEventSelectMode();
	//overLappedEvent();
	//overLappedCallBack();
	//IOComplatePort();
	IOComplatePortEx();
}

void WSAEventSelectMode()
{
	/*
	�����¼��ķ�����ģ��
	�ŵ㣺�Ա�WSAAsyncSelect���Բ�ʹ��windows���ھͿ��Էַ���Ϣ��
	ȱ�㣺һ���߳����֧��WSA_MAXIMUM_WAIT_EVENTS(64)���ͻ�������
	*/
	SOCKET socketArray[MAX_CLIENT];
	WSAEVENT EventArray[MAX_CLIENT], NewEvent;
	WSANETWORKEVENTS NetworkEvents;

	SOCKADDR_IN serverAddr;
	SOCKET acceptSock, listenSock;
	DWORD EventTotal = 0;
	DWORD Index;
	char buf[1024];

	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(5150);

	bind(listenSock, (SOCKADDR*)&serverAddr, sizeof(serverAddr));

	NewEvent = WSACreateEvent();

	WSAEventSelect(listenSock, NewEvent, FD_ACCEPT | FD_CLOSE);

	listen(listenSock, 5);

	socketArray[EventTotal] = listenSock;
	EventArray[EventTotal] = NewEvent;
	EventTotal++;

	HANDLE loopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	while (WAIT_OBJECT_0 != WaitForSingleObject(loopEvent,0))
	{
		// �ȴ�sock�ϵ�event����
		Index = WSAWaitForMultipleEvents(EventTotal,EventArray,FALSE,WSA_INFINITE,FALSE);
		Index -= WSA_WAIT_EVENT_0;

		for (DWORD i = Index; i < EventTotal; ++i)
		{
			// ��ѯ�����¼�
			Index = WSAWaitForMultipleEvents(1, &EventArray[i], FALSE, 0, FALSE);
			if ((Index == WSA_WAIT_TIMEOUT || Index == WSA_WAIT_FAILED)) continue;


			// ����¼�����
			Index = i;
			WSAEnumNetworkEvents(socketArray[Index],EventArray[Index],&NetworkEvents);

			// �������ͷַ�
			if (NetworkEvents.lNetworkEvents & FD_ACCEPT)
			{
				if (NetworkEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
				{
					printf("FD_ACCEPT failed with error %d\n", NetworkEvents.iErrorCode[FD_ACCEPT_BIT]);
					break;
				}

				acceptSock = accept(socketArray[Index], NULL, NULL);
				if (EventTotal > MAX_CLIENT)
				{
					printf("too many connections\n");
					closesocket(acceptSock);
					break;
				}

				NewEvent = WSACreateEvent();
				WSAEventSelect(acceptSock, NewEvent, FD_READ | FD_WRITE | FD_CLOSE);

				EventArray[EventTotal] = NewEvent;
				socketArray[EventTotal] = acceptSock;
				EventTotal++;
				printf("Socket %d connected \n",acceptSock);
			}
			else if (NetworkEvents.lNetworkEvents & FD_READ)
			{
				if (NetworkEvents.iErrorCode[FD_READ_BIT] != 0)
				{
					printf("FD_READ failed with error %d\n", NetworkEvents.iErrorCode[FD_READ_BIT]);
					break;
				}

				int len = recv(socketArray[Index], buf, sizeof(buf), 0);
				printf("recv info:%s\n",buf);

				len = send(socketArray[Index], buf, len, 0);
				printf("send info:%s\n", buf);
			}
			else if (NetworkEvents.lNetworkEvents & FD_WRITE)
			{
				if (NetworkEvents.iErrorCode[FD_WRITE_BIT] !=0 )
				{
					break;
				}
			}
			else if (NetworkEvents.lNetworkEvents & FD_CLOSE)
			{
				if (NetworkEvents.iErrorCode[FD_CLOSE_BIT] != 0)
				{
					printf("FD_CLOSE failed with error %d\n", NetworkEvents.iErrorCode[FD_CLOSE_BIT]);
					//break;
				}
				
				closesocket(socketArray[Index]); // �ر� socket
				WSACloseEvent(EventArray[Index]); // �ر� event
				socketArray[Index] = INVALID_SOCKET; 
				EventTotal -= 1;
			}

		}
	}

}

void WSAAsyncSelectMode()
{
	/*
	����win32������Ϣ�ķ�����ģ��
	�ŵ㣺�ڿͻ�������Ȼ�����ƣ�ʹ��windows���ڷַ�������Ϣ
	ȱ�㣺��������ʹ�ã���Ҫ����һ��windows����
	�����Բ��á�ʲô���������أ�Ŀǰ�������ǣ����ݡ�
	�����ģ�ͽ����ڿͻ���ʹ�ã����Ǻ����׺�nice�ġ�
	*/
	LPCTSTR testClassName = _T("TestWndClass");
	WNDCLASSEX wc;
	memset(&wc, 0, sizeof(wc));
	wc.cbSize = sizeof(wc);
	wc.lpfnWndProc = MainWndProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = testClassName;
	RegisterClassEx(&wc);
	HWND hwnd = CreateWindowEx(0, testClassName, _T("Test Window"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, wc.hInstance, NULL);
	ShowWindow(hwnd, SW_SHOW);

	int error = -1;
	SOCKET listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(5150);

	error = bind(listenSock, (PSOCKADDR)&serverAddr, sizeof(serverAddr));
	error = WSAAsyncSelect(listenSock, hwnd, WM_SOCKET, FD_ACCEPT | FD_CLOSE);
	error = WSAGetLastError();

	error = listen(listenSock, 5);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	UnregisterClass(testClassName, wc.hInstance);
}

void selectMode()
{
	/*
	��������ķ�����ģ��
	�ŵ�:������
	ȱ�㣺ÿ��FD_SET,FD_CLEAR,����client���࣬
	��ʹ�򵥵ı���������������ϵļ����˷ѡ�
	*/
	const int CLIENT_SIZE = 64;
	session sockArry[CLIENT_SIZE];
	for (int i = 0; i < CLIENT_SIZE; ++i)
	{
		sockArry[i].client = INVALID_SOCKET;
	}

	int error = -1;
	SOCKADDR_IN serverAddr;
	u_short nport = 5150;

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(nport);

	SOCKET listenSock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if (listenSock == INVALID_SOCKET)
		error = WSAGetLastError();

	error = bind(listenSock, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (SOCKET_ERROR == error)
	{
		error = WSAGetLastError();
	}

	error = listen(listenSock, CLIENT_SIZE);
	error = WSAGetLastError();
	char buf[1024];

	fd_set read_set;
	int ret = 0;
	do
	{
		FD_ZERO(&read_set);
		FD_SET(listenSock, &read_set);
		for (int i = 0; i < CLIENT_SIZE; ++i)
		{
			if (sockArry[i].client != INVALID_SOCKET)
			{
				FD_SET(sockArry[i].client, &read_set);
			}
		}

		if ((ret = select(0, &read_set, NULL, NULL, NULL)) == SOCKET_ERROR)
		{
			error = WSAGetLastError();
		}

		if (ret > 0)
		{
			if (FD_ISSET(listenSock, &read_set))
			{
				// insert into client array;
				int emptyIndex = -1;
				for (int i = 0; i < CLIENT_SIZE; ++i)
				{
					if (sockArry[i].client == INVALID_SOCKET)
					{
						emptyIndex = i;
						break;
					}
				}
				if (emptyIndex >= 0)
				{
					SOCKADDR_IN clientAddr;
					int addrLen = sizeof(SOCKADDR_IN);
					sockArry[emptyIndex].client = accept(listenSock, (SOCKADDR*)&clientAddr, &addrLen);
					sockArry[emptyIndex].addr.sin_addr.s_addr = clientAddr.sin_addr.s_addr;
					sockArry[emptyIndex].addr.sin_port = clientAddr.sin_port;
					if (sockArry[emptyIndex].client == SOCKET_ERROR)
						error = WSAGetLastError();
				}
			}

			for (int i = 0; i < CLIENT_SIZE; ++i)
			{
				if (sockArry[i].client == INVALID_SOCKET)
					continue;

				if (false == FD_ISSET(sockArry[i].client, &read_set))
					continue;

				int len = recv(sockArry[i].client, buf, 1024, 0);
				if (0 == len || SOCKET_ERROR == len)
				{
					closesocket(sockArry[i].client);
					sockArry[i].client = INVALID_SOCKET;
					continue;
				}
				printf("%s:%d say:%s\n", inet_ntoa (sockArry[i].addr.sin_addr),sockArry[i].addr.sin_port, buf);

				len = send(sockArry[i].client, buf, len, 0);
				if (0 == len || SOCKET_ERROR == len)
				{
					closesocket(sockArry[i].client);
					sockArry[i].client = INVALID_SOCKET;
					continue;
				}
			}

		
		}

	} while (strcmp((const char*)buf,"fin")!=0);

}

void block()
{
	/*
	������demoʱʹ�á�

	*/
	int error = -1;
	// address
	SOCKADDR_IN internetAddr;
	u_short nPort = 5150;
	internetAddr.sin_family = AF_INET;
	internetAddr.sin_addr.s_addr = INADDR_ANY;
	internetAddr.sin_port = htons(nPort);

	// listenSock
	SOCKET listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSock == INVALID_SOCKET)
		error = WSAGetLastError();

	// bind
	error = bind(listenSock, (SOCKADDR*)&internetAddr, sizeof(internetAddr));
	if (error == SOCKET_ERROR)
		error = WSAGetLastError();

	// listen
	error = listen(listenSock, 5);
	error = WSAGetLastError();

	// accept
	SOCKADDR_IN clientAddress;
	int addressLen = sizeof(sockaddr);
	SOCKET newConnection = INVALID_SOCKET;
	int clientNum = 0;

	printf("wait for connect......");
	newConnection = accept(listenSock, (SOCKADDR*)&clientAddress, &addressLen);
	if (newConnection == INVALID_SOCKET)
		error = WSAGetLastError();
	else
		printf("clinet %d %s connected", clientNum++, inet_ntoa(clientAddress.sin_addr));

	do
	{
		// recv

		HANDLE loopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		char *buf = new char[512];
		int len = 0;
		do
		{
			len = recv(newConnection, buf, 512, 0);
			printf("recv:%s\n", buf);
		}while (WAIT_OBJECT_0 != WaitForSingleObject(loopEvent, 0));

		if (len == 0)break;

		// send
		len = send(newConnection, buf, len, 0);
		error = WSAGetLastError();

		//if (strncmp(buf, "exit", 512)==0)
		//	break;
	} while (newConnection != -1);


	//close
	closesocket(newConnection);
	closesocket(listenSock);
}

const int DATABUFSIZE = 1024;
char buf[DATABUFSIZE];
LRESULT CALLBACK MainWndProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	SOCKET Accept;
	WSABUF DataBuf;
	DataBuf.len = DATABUFSIZE;
	DataBuf.buf = buf;
	int len;
	DWORD recvLen = 0, flag=0;
	DWORD  sendFlag = 0;
	int port;
	char* addr;
	sockaddr_in sockAddr;
	int addrLen;
	switch (wMsg)
	{
	case WM_PAINT:
		break;
	case WM_SOCKET:
	{
		if (WSAGETSELECTERROR(lParam))
		{
			closesocket((SOCKET)wParam);
			break;
		}

		switch (WSAGETSELECTEVENT(lParam))
		{
		case FD_ACCEPT:
			Accept = accept(wParam, NULL, NULL);
			WSAAsyncSelect(Accept, hDlg, WM_SOCKET,FD_READ | FD_WRITE | FD_CLOSE);
			break;
		case FD_READ:
			// get ip
			memset(&sockAddr, 0, sizeof(sockAddr));
			addrLen = sizeof(sockAddr);
			::getpeername((SOCKET)wParam, (sockaddr*)&sockAddr, &addrLen);

			port = ntohs(sockAddr.sin_port);
			addr = inet_ntoa(sockAddr.sin_addr);  // strIP  

			// get data
			len = WSARecv((SOCKET)wParam, &DataBuf, 1, &recvLen,&flag,NULL,NULL);
			if (-1 == len)
			{
				len = WSAGetLastError();
				closesocket((SOCKET)wParam);
			}
			else
			{
				printf("recv from %s:%d:%s\n",addr,port,buf);
			}
			len = WSASend((SOCKET)wParam, &DataBuf, 1, &recvLen, sendFlag, NULL, NULL);
			if ( -1 == len)
			{
				len = WSAGetLastError();
				closesocket((SOCKET)wParam);
			}
			break;
		case FD_WRITE:
			break;
		case FD_CLOSE:
			closesocket((SOCKET)wParam);
			break;
		case FD_ADDRESS_LIST_CHANGE:
			break;
		}
	}break;
	default:
		return DefWindowProc(hDlg, wMsg, wParam, lParam);
	}
	return 0;
}