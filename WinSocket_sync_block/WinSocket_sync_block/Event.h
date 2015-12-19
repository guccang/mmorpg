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
	};

	enum EVENT_ID
	{
		EVENTID_TCPClientEvent,
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
		void OnAccept()
		{
			// do something
			::OnAccept(_client,*_stream);
		}
	};
}