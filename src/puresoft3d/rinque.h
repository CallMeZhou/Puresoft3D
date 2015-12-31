#pragma once
#include <windows.h> // for CriticalSection

template<typename T>
class RingQueueMTDefCopier
{
public:
	static void copy(T& d, const T& s);
};

template<typename T, size_t LEN, class COPIER = RingQueueMTDefCopier<T> >
class RingQueueMT
{
public:
	RingQueueMT(void);
	~RingQueueMT();
	bool push(const T& item);
	bool pop(T& item);
	void abort(void);
	void reset(void);

private:
	CRITICAL_SECTION m_cs;
	T m_queue[LEN];
	int m_head;
	int m_tail;
	volatile bool m_abort;
};

template<typename T>
void RingQueueMTDefCopier<T>::copy(T& d, const T& s)
{
	memcpy(&d, &s, sizeof(T));
}

template<typename T, size_t LEN, class COPIER>
RingQueueMT<T, LEN, COPIER>::RingQueueMT(void)
{
	InitializeCriticalSection(&m_cs);
	m_head = 0;
	m_tail = 0;
	m_abort = false;
}

template<typename T, size_t LEN, class COPIER>
RingQueueMT<T, LEN, COPIER>::~RingQueueMT()
{
	DeleteCriticalSection(&m_cs);
}

template<typename T, size_t LEN, class COPIER>
bool RingQueueMT<T, LEN, COPIER>::push(const T& item)
{
	// push in queue tail
	EnterCriticalSection(&m_cs);
	COPIER::copy(m_queue[m_tail], item);
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

template<typename T, size_t LEN, class COPIER>
bool RingQueueMT<T, LEN, COPIER>::pop(T& item)
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
	COPIER::copy(item, m_queue[m_head]);
	m_head++;

	if(LEN == m_head)
	{
		m_head = 0;
	}

	LeaveCriticalSection(&m_cs);
	return true;
}

template<typename T, size_t LEN, class COPIER>
void RingQueueMT<T, LEN, COPIER>::abort(void)
{
	m_abort = true;
}

template<typename T, size_t LEN, class COPIER>
void RingQueueMT<T, LEN, COPIER>::reset(void)
{
	m_abort = false;
}