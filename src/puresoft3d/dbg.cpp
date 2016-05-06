#include "pipeline.h"

void PuresoftPipeline::saveTexture(int idx, const wchar_t* path, bool dataIsFloat)
{
	if(-1 == idx)
	{
		m_depth->saveAsBmpFile(path, dataIsFloat);
	}
	else if(-2 == idx)
	{
		m_display->saveAsBmpFile(path, dataIsFloat);
	}
	else
	{
		if(idx >= (int)m_texPool.size())
		{
			throw std::out_of_range("PuresoftPipeline::saveTexture");
		}

		m_texPool[idx]->saveAsBmpFile(path, dataIsFloat);
	}
}

void PuresoftPipeline::saveTextureAsRaw(int idx, const wchar_t* path)
{
	if(-1 == idx)
	{
		m_depth->saveAsRawFile(path);
	}
	else if(-2 == idx)
	{
		m_display->saveAsRawFile(path);
	}
	else
	{
		if(idx >= (int)m_texPool.size())
		{
			throw std::out_of_range("PuresoftPipeline::saveTextureAsRaw");
		}

		m_texPool[idx]->saveAsRawFile(path);
	}
}

void PuresoftPipeline::getTaskQCounters(unsigned int* taskPushCalled, unsigned int* taskPushSpinned, unsigned int* taskPopCalled, unsigned int* taskPopSpinned)
{
	*taskPushCalled = 0;
	*taskPushSpinned = 0;
	*taskPopCalled = 0;
	*taskPopSpinned = 0;
#if PROFILING
	for(int i = 0; i < m_numberOfThreads; i++)
	{
		FragmentThreadTaskQueue& q = m_fragTaskQueues[i];
		*taskPushCalled += q.m_counters.beginPushCalled;
		*taskPushSpinned += q.m_counters.beginPushSpinned;
		*taskPopCalled += q.m_counters.beginPopCalled;
		*taskPopSpinned += q.m_counters.beginPopSpinned;
	}
#endif
}
