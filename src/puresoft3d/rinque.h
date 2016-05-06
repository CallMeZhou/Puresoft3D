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
		m_in = m_out = m_len = 0;
#ifdef PROFILING
		m_ignorePopSpinning = false;
#endif
	}

	~RingQueueMT()
	{
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
#ifdef PROFILING
		m_counters.beginPushCalled++;
#endif
		while(true)
		{
			if(LEN == m_len)
			{
#ifdef PROFILING
				m_counters.beginPushSpinned++;
#endif
				// friendly spin around
				YIELD_CPU;
			}
			else
			{
				// stop spinning
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
#ifdef PROFILING
		m_counters.beginPopCalled++;
#endif

		while(true)
		{
			if(0 == m_len)
			{
#ifdef PROFILING
				if(!m_ignorePopSpinning)
				{
					m_counters.beginPopSpinned++;
				}
#endif

				// friendly spin around
				YIELD_CPU;
			}
			else
			{
				// stop spinning
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
		}

		return false;
	}

private:
	T m_queue[LEN];
	size_t m_out;
	size_t m_in;
	volatile size_t m_len;

// debuggabilities
#ifdef PROFILING
public:
	struct COUNTERS
	{
		volatile unsigned int beginPopCalled;
		volatile unsigned int beginPopSpinned;
		volatile unsigned char filling[56];
		volatile unsigned int beginPushCalled;
		volatile unsigned int beginPushSpinned;
	} m_counters;
	bool m_ignorePopSpinning;
	void resetCounters(void)
	{
		InterlockedExchange(&m_counters.beginPopCalled, 0);
		InterlockedExchange(&m_counters.beginPopSpinned, 0);
		InterlockedExchange(&m_counters.beginPushCalled, 0);
		InterlockedExchange(&m_counters.beginPushSpinned, 0);
	}
#endif
};