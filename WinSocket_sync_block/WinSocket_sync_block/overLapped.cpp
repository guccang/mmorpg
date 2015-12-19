#include "stdafx.h"
#include "overLapped.h"
#include <WinSock2.h>
#include "socketTools.h"

#define DATA_BUF_SIZE 1024
char wsabuf[DATA_BUF_SIZE];

/*
�ص�I/O
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

ʹ����Щ����������overlapped������ʹ�õ����첽�����ݿ����첽��ʽ��
��winsock I/Oģ�� ֮һ�� WSAEventSelect / WSASelect / select ��3��
������ģ�ͣ�ֻ���ڻص����첽�������ݴ��ں˿ռ䵽�û��ռ��copy��Ȼ
��ͬ���ġ���ʹ��overlapped���ص�IO�����յ�event��ʱ�������Ѿ���
�ں˿ռ�copy���û��ռ��ˡ�

overlapped(�ص�IO)ģ�͵�ʹ�ã����������ڵ��÷����Ƿ񴫵���overlapped�ṹ��
�����ǻ�˵��ʹ���ص�i/o���е�socket��������Ҫʹ���ص�i/oģ�͡�

�������ʹ��accpt���������û����ӡ�ʹ��WSARecvͶ��һ��overlapped����ô
accept��������������WSARecv���Ƿ�����+����copy�첽�����~~~~~~�����Լ�
��ô�����ˡ���������Ӧ��������һ��ģ�͡������ǻ��á������ϵĺö�blog
���ʹ����Щ�����ºܶ�����޷����ģ������ġ���ˣ���Ȼ���Ƽ�����ϵͳ
��רҵ�鼮����ѧϰ�������Ų����Ϊһ������ӡ�

��Ͷ��һ��������ɺ��ص�IO֧�֣��¼�֪ͨ��ʽ����������̣��ص���֪ͨ��ʽ��

��ظ��alertable state
https://msdn.microsoft.com/en-us/library/aa363772(VS.85).aspx
ʹ���ص�I/Oֻ�е��߳̽�����alertable state��ʱ��,���ص�i/o���ʱ���ں˲Ż����
������̡�

*/
void overLappedEvent()
{
	/*
	�ص���ɺ��¼�֪ͨ��ʽ��

	�����ص�io�÷���
	Ȼ��������
	ʹ��WSARecv��������ص�IO�÷�������һ���ͻ��˵����󣬽��ܿͻ��˷��͵���Ϣ�����ؽ��ܵ������ݸ��ͻ��ˡ�
	�ظ��շ����̣�֪���ͻ��������Ͽ����ӡ�
	
	1������accpet����һ���ͻ�������acceptSocket
	2���ڴ�������Ͷ��һ��recv�¼�����
	3���ȴ�recv�¼�
	4����ȡ�¼��������
	5��չʾ����
	6������Ͷ��recv�¼�����
	7���ظ�3-6
	
	���ͻ��˶Ͽ��󣬷�����Ҳͬʵʩ�رա�
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

	// ����accept
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

	// �ص�IOͶ�� recv������
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

		// �ȴ�Ͷ���¼�����
		Index = WSAWaitForMultipleEvents(EventTotal, EventArray, FALSE, WSA_INFINITE, FALSE);

		// �����¼�
		WSAResetEvent(EventArray[Index - WSA_WAIT_EVENT_0]);

		// ����¼����
		WSAGetOverlappedResult(acceptSocket, &acceptOverLapped, &ByteTransferred, FALSE, &Flagsx);

		if (ByteTransferred == 0) // client�ر�������
		{
			printf("client close socket %d\n",acceptSocket);
			closesocket(acceptSocket);
			WSACloseEvent(EventArray[Index - WSA_WAIT_EVENT_0]);
			printf("press any key to exit");
			getchar();
			return;
		}

		// recvû�д���Ļ�,��ʾ��ȡ��������
		printf("recv : %s",dataBuf.buf);
		error = send(acceptSocket,dataBuf.buf,ByteTransferred,0);

		// ׼������Ͷ�ݹ���
		Flagsx = 0;
		ZeroMemory(&acceptOverLapped, sizeof(WSAOVERLAPPED));

		acceptOverLapped.hEvent = EventArray[Index - WSA_WAIT_EVENT_0];
		dataBuf.len = DATA_BUF_SIZE;
		dataBuf.buf = wsabuf;

		// ��acceptSocket��Ͷ���µ�recv�¼�
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
	����ص�������������֪ͨ��ʽ
	
	1��Ͷ��recv�ص�����
	2���߳̽���altertable state 
	3���ص�io��ɺ�,�̼߳������л�����ݣ���ʾ���� 
	4������Ͷ��recv�ص�����
	5: 2-4

	�ص��в���
	void CALLBACK workerRountine(DWORD dwError,DWORD cbTransfered,LPWSAOVERLAPPED lpOverLapped,DWORD dwFlags)
	lpOverLappedָ��overlapped�ڴ�����
	���ʹ��overlappedData �ṹ�壬�ĵ�һ�����ݽṹΪWSAOVERLAPPED�����Խ��Զ��������ݴ���ص���

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

	// ��ʼ׼��Ͷ���ص�����
	flags = 0;
	LPOVERLAPPEDDATA overLappedData = new OVERLAPEDDATA;
	ZeroMemory(&(overLappedData->overlapped), sizeof(WSAOVERLAPPED));
	overLappedData->dataBuf.len = DATA_BUF_SIZE;
	overLappedData->dataBuf.buf = new char[DATA_BUF_SIZE];
	overLappedData->client = acceptSocket;
	int error;
	// ʹ��׼���õ�����ݣ�Ͷ��recv����
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

	// iO��ɡ���������~~funck
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