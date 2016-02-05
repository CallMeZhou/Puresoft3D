#include <memory.h>
#include "pipeline.h"

void PuresoftPipeline::setUserDataBytes(size_t bytes)
{
	if(0 == bytes)
	{
		if(m_userDataPool)
		{
			_aligned_free(m_userDataPool);
			m_userDataPool = NULL;
		}
	}
	else if(m_userDataBuffers.unitBytes < bytes)
	{
		setUserDataBytes(0);

		m_userDataBuffers.unitBytes = ((bytes * 8 + 512) / 512) * 64;

		size_t bufferCount = 
			3 +										// 3 vertices
			2 * m_numberOfThreads +					// 2 interpTemps
			1 * m_numberOfThreads +					// 1 fragment processor input for each threads
			MAX_FRAGTASKS * 2 * m_numberOfThreads;	// 1 task queue (queue length is MAX_FRAGTASKS) for each threads and
													// 2 buffers for each task

		size_t totalBytes = bufferCount * m_userDataBuffers.unitBytes;

		m_userDataPool = _aligned_malloc(totalBytes, 16);
		uintptr_t pool = (uintptr_t)m_userDataPool;

		m_userDataBuffers.verts = (void*)pool;
		pool += m_userDataBuffers.unitBytes * 3;

		for(int i = 0; i < m_numberOfThreads; i++)
		{
			m_userDataBuffers.interpTemps[i] = (void*)pool;
			pool += m_userDataBuffers.unitBytes * 2;

			m_userDataBuffers.fragInputs[i] = (void*)pool;
			pool += m_userDataBuffers.unitBytes;

			m_userDataBuffers.taskQueues[i] = (void*)pool;
			pool += MAX_FRAGTASKS * 2 * m_userDataBuffers.unitBytes;
		}
	}
}