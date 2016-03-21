#pragma once
#include <windows.h>

//#define YIELD_CPU SwitchToThread()
//#define YIELD_CPU Sleep(0)
//#define YIELD_CPU YieldProcessor()
#define YIELD_CPU

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

	T* beginPush(void)
	{
		while(true)
		{
			if(LEN == m_len)
			{
				// spin around
				if(m_abort)
				{
					return NULL;
				}

				YIELD_CPU;
			}
			else
			{
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
		InterlockedIncrement(&m_len);
	}

	T* beginPop(void)
	{
		while(true)
		{
			if(0 == m_len)
			{
				// spin around
				if(m_abort)
				{
					return NULL;
				}

				YIELD_CPU;
			}
			else
			{
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
		InterlockedDecrement(&m_len);
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

			YIELD_CPU;
		}

		return false;
	}

	bool pollEmpty_busy(void) const
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