#pragma once
namespace GUGGAME
{
	template <typename T>
	class queue
	{
		T* _head;
		T* _tail;
		size_t size;
	public :
		T* front(){ return _head; }
		queue()
		{
			_head = _tail = NULL;
			size = 0;
		}

		void pop_front()
		{
			if (_head)
			{
				if (_head == _tail)
				{
					_head = _tail = NULL;
				}
				else
				{
					_head = (T*)_head->_next;
				}
				size--;
			}
		}
		void push_back(T* node)
		{
			if (_tail)
			{
				node->_next = NULL;
				_tail->_next = node;
				_tail = node;
			}
			else
			{
				_head = _tail = node;
				node->_next = NULL;
			}
			size++;
		}
		void push_front(T* node)
		{
			if (_head)
			{
				node->_next = _head;
				_head = node;
			}
			else
			{
				node->_next = NULL;
				_head = _tail = node;
			}
			size++;
		}
		bool empty()
		{
			return size == 0;
		}
	};


	template<typename T>
	class gugQueue
	{
	private:
		typedef queue<T> QueueContainer;
		QueueContainer _container;
		QueueContainer _container2;
		QueueContainer* _cur;
		QueueContainer* _proc;
		CriticalSection _cs;
	public:
		gugQueue()
		{
			CriticalSection::Lock lock(_cs);
			_cur  = &_container;
			_proc = &_container2;
		}
		void PushEvent(T* e)
		{
			CriticalSection::Lock lock(_cs);
			_cur->push_back(e);
		}
		void OnAccept()
		{
			{
				CriticalSection::Lock lock(_cs);
				if (_proc == &_container2)
				{
					_cur = &_container2;
					_proc = &_container;
				}
				else
				{
					_cur = &_container;
					_proc = &_container2;
				}
			}

			T* e = NULL;
			while (e = _proc->front())
			{
				_proc->pop_front();
				e->OnAccept();
				delete e;
				e = NULL;
			}
		}
	};
}