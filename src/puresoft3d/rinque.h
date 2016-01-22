#pragma once
#include <windows.h> // for CriticalSection

template<class T, size_t LEN>
class RingQueueMT
{
public:
	RingQueueMT(void)
	{
		InitializeCriticalSection(&m_cs);
		m_head = 0;
		m_tail = 0;
		m_abort = false;
	}

	~RingQueueMT()
	{
		DeleteCriticalSection(&m_cs);
	}

	template<typename prep>
	void prepare(prep pr)
	{
		for(size_t i = 0; i < LEN; i++)
		{
			pr(i, m_queue + i);
		}
	}

	bool push(const T& item = NULL)
	{
		// push in queue tail
		EnterCriticalSection(&m_cs);
		memcpy(m_queue + m_tail, &item, sizeof(T));
		int nextTail = m_tail + 1;
		if(LEN == nextTail)
		{
			nextTail = 0;
		}
		LeaveCriticalSection(&m_cs);

		// spin waiting for queue to be not full
		while(true)
		{
			EnterCriticalSection(&m_cs);
			if(nextTail == m_head) // spin around
			{
				LeaveCriticalSection(&m_cs);

				if(m_abort)
				{
					return false;
				}
			}
			else // stop spinning
			{
				break;
			}
		}

		// complete pushing queue tail
		m_tail = nextTail;
		LeaveCriticalSection(&m_cs);

		return true;
	}

	T* beginPush(int* beginPushTracer)
	{
		// push in queue tail
		EnterCriticalSection(&m_cs);
		int nextTail = m_tail + 1;
		if(LEN == nextTail)
		{
			nextTail = 0;
		}

		*beginPushTracer = nextTail;
		return m_queue + m_tail;
	}

	bool endPush(int beginPushTracer)
	{
		LeaveCriticalSection(&m_cs);

		// spin waiting for queue to be not full
		while(true)
		{
			EnterCriticalSection(&m_cs);
			if(beginPushTracer == m_head) // spinning
			{
				LeaveCriticalSection(&m_cs);

				if(m_abort)
				{
					return false;
				}
			}
			else // stop spinning
			{
				break;
			}
		}

		// complete pushing queue tail
		m_tail = beginPushTracer;
		LeaveCriticalSection(&m_cs);

		return true;
	}

	bool pop(T& item)
	{
		// spin waiting for queue to be not empty
		while(true)
		{
			EnterCriticalSection(&m_cs);
			if(m_head == m_tail) // spin around
			{
				LeaveCriticalSection(&m_cs);

				if(m_abort)
				{
					return false;
				}
			}
			else // stop spinning
			{
				break;
			}
		}

		// take away queue head
		memcpy(&item, m_queue + m_head, sizeof(T));
		m_head++;

		if(LEN == m_head)
		{
			m_head = 0;
		}

		LeaveCriticalSection(&m_cs);
		return true;
	}

	void abort(void)
	{
		m_abort = true;
	}

	void reset(void)
	{
		m_abort = false;
	}

	bool empty(void)
	{
		EnterCriticalSection(&m_cs);
		bool isEmpty = (m_head == m_tail);
		LeaveCriticalSection(&m_cs);
		return isEmpty;
	}
	bool pollEmpty(void)
	{
		while(true)
		{
			if(empty())
			{
				return true;
			}

			if(m_abort)
			{
				return false;
			}
		}

		// unreachable
		return true;
	}

private:
	CRITICAL_SECTION m_cs;
	T m_queue[LEN];
	volatile int m_head;
	volatile int m_tail;
	volatile bool m_abort;
};