#pragma once
#include "TcpReceiver.h"

namespace GUGGAME
{
	class eventBase
	{
	public:
		eventBase* _next;
		int _id;
		eventBase(int id)
		{
			_id = id;
		}
		virtual void OnAccept(void* params)
		{
			// do something
			params = params;
		}
	};

	enum EVENT_ID
	{
		EVENTID_TCPClientEvent,
		EVENTID_IOCPEvent,
	};

	class TCPClientEvent : public eventBase
	{
	public :
		LEUD::StreamFix* _stream;
		SOCKET _client;
		TCPClientEvent(SOCKET client,LEUD::StreamFix* stream)
			:eventBase(EVENTID_TCPClientEvent)
		{
			_stream = stream;
			_client = client;
		}
		virtual ~TCPClientEvent()
		{
			if (_stream)
			{
				delete _stream->m_Buffer;
				delete _stream;
			}
		}
		void OnAccept(void* params)
		{
			params = params;
			// do something
			::OnAccept(_client,*_stream);
		}
	};

	class IOCPEvent : public eventBase
	{
	public :
		IOCPEvent(SOCKET socket, int errorCode)
			:eventBase(EVENTID_IOCPEvent)
		{
			_socket = socket;
			_errorCode = errorCode;
		}
		SOCKET _socket;
		int _errorCode;
		void OnAccept(void* params)
		{
			params = params;
			if (_errorCode == WSAENOTSOCK)
			{
				ViewList::remove(_socket,_errorCode);
			}
			else if (_errorCode == WSAECONNABORTED)
			{
				ViewList::remove(_socket,_errorCode);
			}
		}
	};
}