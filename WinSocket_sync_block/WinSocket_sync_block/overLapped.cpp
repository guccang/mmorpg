#include "stdafx.h"
#include "overLapped.h"
#include <WinSock2.h>
#include "socketTools.h"

#define DATA_BUF_SIZE 1024
char wsabuf[DATA_BUF_SIZE];

/*
重叠I/O
WSASend
WSASendTo
WSARecv
WSARecvFrom
WSAIoctr
WSARecvMsg
AcceptEx
ConnectEx
TransmitFile
TransmitPacckets
DisconnectEx
WsANSPloctl

使用这些方法，传入overlapped参数，使用调用异步和数据拷贝异步方式。
而winsock I/O模型 之一的 WSAEventSelect / WSASelect / select 这3个
非阻塞模型，只是在回调上异步，而数据从内核空间到用户空间的copy仍然
是同步的。而使用overlapped（重叠IO）在收到event的时候数据已经从
内核空间copy到用户空间了。

overlapped(重叠IO)模型的使用，仅仅依赖于调用方法是否传递了overlapped结构。
而不是会说，使用重叠i/o所有的socket方法都需要使用重叠i/o模型。

比如可以使用accpt方法接受用户连接。使用WSARecv投递一个overlapped，那么
accept是阻塞方法，而WSARecv则是非阻塞+数据copy异步。因此~~~~~~看你自己
怎么抉择了。不过我想应当都是用一种模型。而不是混用。网络上的好多blog
混合使用这些，导致很多概念无法理解的，混淆的。因此，任然是推荐看看系统
的专业书籍，来学习。这样才不会成为一个半吊子。

当投递一个请求完成后，重叠IO支持，事件通知方式，和完成历程（回调）通知方式。

相关概念：alertable state
https://msdn.microsoft.com/en-us/library/aa363772(VS.85).aspx
使用重叠I/O只有当线程进入了alertable state的时候,当重叠i/o完成时，内核才会调用
完成例程。

*/
void overLappedEvent()
{
	/*
	重叠完成后，事件通知方式。

	测试重叠io用法。
	然而。。。
	使用WSARecv方法理解重叠IO用法。接受一个客户端的请求，接受客户端发送的消息，返回接受到的数据给客户端。
	重复收发过程，知道客户端主动断开连接。
	
	1：阻塞accpet接受一个客户端连接acceptSocket
	2：在此连接上投递一个recv事件请求
	3：等待recv事件
	4：获取事件完成数据
	5：展示数据
	6：重新投递recv事件请求
	7：重复3-6
	
	当客户端断开后，服务器也同实施关闭。
	*/
	WSABUF dataBuf;
	DWORD EventTotal = 0,
		RecvByte = 0,
		Flagsx = 0,
		ByteTransferred = 0;

	WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
	WSAOVERLAPPED acceptOverLapped;
	SOCKET listenSocket, acceptSocket;

	listenSocket = (SOCKET)getSocket();
	if (listenSocket == INVALID_SOCKET)
		return;

	if (false == doBind((unsigned int)listenSocket, 5150))
		return;

	if (false == doListen((unsigned int)listenSocket,WSA_MAXIMUM_WAIT_EVENTS))
		return;

	// 阻塞accept
	acceptSocket = accept(listenSocket, NULL, NULL);
	if (INVALID_SOCKET == acceptSocket)
	{
		printf("accept failed %d", WSAGetLastError());
	}

	printf("client connected!!!");
	EventArray[EventTotal] = WSACreateEvent();
	ZeroMemory(&acceptOverLapped, sizeof(WSAOVERLAPPED));
	acceptOverLapped.hEvent = EventArray[EventTotal];

	dataBuf.len = DATA_BUF_SIZE;
	dataBuf.buf = wsabuf;

	EventTotal++;

	// 重叠IO投递 recv方法。
	int error = 0;
	if (WSARecv(acceptSocket, &dataBuf, 1, &RecvByte, &Flagsx, &acceptOverLapped, NULL) == SOCKET_ERROR)
	{
		error = WSAGetLastError();
		if (error != WSA_IO_PENDING)
		{
			printf("WSARecv failed %d", error);
		}
	}

	while (true)
	{
		DWORD Index;

		// 等待投递事件触发
		Index = WSAWaitForMultipleEvents(EventTotal, EventArray, FALSE, WSA_INFINITE, FALSE);

		// 重置事件
		WSAResetEvent(EventArray[Index - WSA_WAIT_EVENT_0]);

		// 获得事件结果
		WSAGetOverlappedResult(acceptSocket, &acceptOverLapped, &ByteTransferred, FALSE, &Flagsx);

		if (ByteTransferred == 0) // client关闭了连接
		{
			printf("client close socket %d\n",acceptSocket);
			closesocket(acceptSocket);
			WSACloseEvent(EventArray[Index - WSA_WAIT_EVENT_0]);
			printf("press any key to exit");
			getchar();
			return;
		}

		// recv没有错误的话,显示获取到的数据
		printf("recv : %s",dataBuf.buf);
		error = send(acceptSocket,dataBuf.buf,ByteTransferred,0);

		// 准备重新投递工作
		Flagsx = 0;
		ZeroMemory(&acceptOverLapped, sizeof(WSAOVERLAPPED));

		acceptOverLapped.hEvent = EventArray[Index - WSA_WAIT_EVENT_0];
		dataBuf.len = DATA_BUF_SIZE;
		dataBuf.buf = wsabuf;

		// 在acceptSocket上投递新的recv事件
		if (SOCKET_ERROR == WSARecv(acceptSocket, &dataBuf, 1, &RecvByte, &Flagsx, &acceptOverLapped, NULL))
		{
			error = WSAGetLastError();
			if (error != WSA_IO_PENDING)
			{
				printf("wsarecv failed %d", error);
			}
		}
	}
}

void CALLBACK workerRountine(DWORD dwError, DWORD cbTransfered, LPWSAOVERLAPPED lpOverLapped, DWORD dwFlags);
typedef struct overlappedData
{
	WSAOVERLAPPED overlapped;
	SOCKET client;
	WSABUF dataBuf;
	DWORD recvBytes;
	DWORD sendBytes;
}OVERLAPEDDATA, *LPOVERLAPPEDDATA;


void clear(LPOVERLAPPEDDATA data)
{
	closesocket(data->client);
	delete []data->dataBuf.buf;
	delete data;
}
void overLappedCallBack()
{
	/*
	完成重叠请求后，完成历程通知方式
	
	1：投递recv重叠请求
	2：线程进入altertable state 
	3：重叠io完成后,线程继续运行获得数据，显示数据 
	4：重新投递recv重叠请求
	5: 2-4

	回调中参数
	void CALLBACK workerRountine(DWORD dwError,DWORD cbTransfered,LPWSAOVERLAPPED lpOverLapped,DWORD dwFlags)
	lpOverLapped指向overlapped内存区域。
	因此使用overlappedData 结构体，的第一个数据结构为WSAOVERLAPPED，可以将自动定义数据传入回调中

	*/

	DWORD  flags = 0;
	SOCKET listenSocket,acceptSocket;

	listenSocket = getSocket();
	if (false == doBind(listenSocket, SERVER_PORT))
		return;

	if (false == doListen(listenSocket))
		return;

	acceptSocket = accept(listenSocket, NULL, NULL);
	if (INVALID_SOCKET == acceptSocket)
	{
		printf("accept error %d",WSAGetLastError());
		return;
	}

	// 开始准备投递重叠数据
	flags = 0;
	LPOVERLAPPEDDATA overLappedData = new OVERLAPEDDATA;
	ZeroMemory(&(overLappedData->overlapped), sizeof(WSAOVERLAPPED));
	overLappedData->dataBuf.len = DATA_BUF_SIZE;
	overLappedData->dataBuf.buf = new char[DATA_BUF_SIZE];
	overLappedData->client = acceptSocket;
	int error;
	// 使用准备好点的数据，投递recv请求
	if (SOCKET_ERROR == WSARecv(acceptSocket, &(overLappedData->dataBuf), 1, &(overLappedData->recvBytes), &flags, &(overLappedData->overlapped), workerRountine))
	{
		error = WSAGetLastError();
		if (error != WSA_IO_PENDING)
		{
			printf("WSARecv failed %d",error);
			clear(overLappedData);
			return;
		}
	}

	// alterable state
	WSAEVENT event = WSACreateEvent();
	int Index = 0;
	while (true)
	{
		// enter alteralbe state
		Index = WSAWaitForMultipleEvents(1, &event, FALSE, WSA_INFINITE, TRUE);
		if (Index == WAIT_IO_COMPLETION)
		{
			continue;
		}
		else
		{
			printf("alterable state error %d",WSAGetLastError());
			return;
		}
	}
}

void CALLBACK workerRountine(DWORD dwError,DWORD cbTransfered,LPWSAOVERLAPPED lpOverLapped,DWORD dwFlags)
{
	LPOVERLAPPEDDATA overlappedData = (LPOVERLAPPEDDATA)lpOverLapped;
	dwFlags = dwFlags;

	if (dwError != 0 || cbTransfered == 0)
	{
		printf("cb workeRountine error %d", WSAGetLastError());
		return;
	}

	// iO完成。。数据呢~~funck
	printf("recv:%s\n", overlappedData->dataBuf.buf);
	send(overlappedData->client, overlappedData->dataBuf.buf, cbTransfered, 0);

	DWORD flags = 0;
	ZeroMemory(overlappedData->dataBuf.buf, DATA_BUF_SIZE);

	if (WSARecv(overlappedData->client, &(overlappedData->dataBuf), 1, &(overlappedData->recvBytes), &flags, &(overlappedData->overlapped), workerRountine) == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		if (error != WSA_IO_PENDING)
		{
			printf("WSARecv failed %d",error);
			clear(overlappedData);
			return;
		}
	}
}