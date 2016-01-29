#pragma once
#include <windows.h> // for CriticalSection

template<class T, size_t LEN>
class RingQueueMT
{
public:
	RingQueueMT(void)
	{
		InitializeCriticalSection(&m_cs);
		m_in = m_out = m_len = 0;
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
		while(true)
		{
			EnterCriticalSection(&m_cs);
			if(LEN == m_len)
			{
				// unlock queue for others and spin around
				LeaveCriticalSection(&m_cs);

				if(m_abort)
				{
					return false;
				}
			}
			else
			{
				// go on with queue locked
				break;
			}
		}

		memcpy(m_queue + m_in, &item, sizeof(T));
		if(LEN == ++m_in)
		{
			m_in = 0;
		}
		m_len++;
		LeaveCriticalSection(&m_cs);

		return true;
	}

	bool pop(T& item)
	{
		while(true)
		{
			EnterCriticalSection(&m_cs);
			if(0 == m_len)
			{
				// unlock queue for others and spin around
				LeaveCriticalSection(&m_cs);

				if(m_abort)
				{
					return false;
				}
			}
			else
			{
				// go on with queue locked
				break;
			}
		}

		memcpy(&item, m_queue + m_out, sizeof(T));
		if(LEN == ++m_out)
		{
			m_out = 0;
		}
		m_len--;
		LeaveCriticalSection(&m_cs);

		return true;
	}

	T* beginPush(void)
	{
		while(true)
		{
			EnterCriticalSection(&m_cs);
			if(LEN == m_len)
			{
				// unlock queue for others and spin around
				LeaveCriticalSection(&m_cs);

				if(m_abort)
				{
					return NULL;
				}
			}
			else
			{
				// go on with queue locked
				break;
			}
		}

		return m_queue + m_in;
	}

	void endPush(void)
	{
		if(LEN == ++m_in)
		{
			m_in = 0;
		}
		m_len++;
		LeaveCriticalSection(&m_cs);
	}

	T* beginPop(void)
	{
		while(true)
		{
			EnterCriticalSection(&m_cs);
			if(0 == m_len)
			{
				// unlock queue for others and spin around
				LeaveCriticalSection(&m_cs);

				if(m_abort)
				{
					return NULL;
				}
			}
			else
			{
				// go on with queue locked
				break;
			}
		}

		return m_queue + m_out;
	}

	void endPop(void)
	{
		if(LEN == ++m_out)
		{
			m_out = 0;
		}
		m_len--;
		LeaveCriticalSection(&m_cs);
	}

	void abort(void)
	{
		m_abort = true;
	}

	void reset(void)
	{
		m_abort = false;
	}

	size_t size(void) const
	{
		return m_len;
	}

	bool pollEmpty(void) const
	{
		while(true)
		{
			if(0 == m_len)
			{
				return true;
			}

			if(m_abort)
			{
				break;
			}
		}

		return false;
	}

private:
	CRITICAL_SECTION m_cs;
	T m_queue[LEN];
	size_t m_out;
	size_t m_in;
	volatile size_t m_len;
	volatile bool m_abort;
};