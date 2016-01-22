#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include "udm.h"

PuresoftUserDataManager::PuresoftUserDataManager(size_t cores)
{
	assert(cores <= MAX_FRAGTHREADS);
	m_cores = cores;
	m_pool = NULL;
	m_userDataBytes = 0;
	memset(&m_buffers, 0, sizeof(m_buffers));
}

PuresoftUserDataManager::~PuresoftUserDataManager(void)
{
	setUserDataBytes(0);
}

USERDATABUFFERS* PuresoftUserDataManager::setUserDataBytes(size_t bytes)
{
	if(0 == bytes)
	{
		if(m_pool)
		{
			_aligned_free(m_pool);
			m_pool = NULL;
		}

		memset(&m_buffers, 0, sizeof(m_buffers));
		m_userDataBytes = 0;
	}
	else if(m_userDataBytes < bytes)
	{
		setUserDataBytes(0);

		m_userDataBytes = ((bytes * 8 + 127) / 128) * 16;

		size_t bufferCount = 
			3 +							// 3 vertices
			2 +							// 2 tempInterps
			1 * m_cores +				// 1 fragment processor input for each threads
			MAX_FRAGTASKS * 2 * m_cores;// 1 task queue (queue length is MAX_FRAGTASKS) for each threads
										// 2 buffers for each task

		size_t totalBytes = bufferCount * m_userDataBytes;

		m_pool = _aligned_malloc(totalBytes, 16);
		uintptr_t pool = (uintptr_t)m_pool;

		m_buffers.verts = (void*)pool;
		pool += m_userDataBytes * 3;
		m_buffers.tempInterp = (void*)pool;
		pool += m_userDataBytes * 2;

		for(size_t i = 0; i < m_cores; i++)
		{
			m_buffers.fragInputs[i] = (void*)pool;
			pool += m_userDataBytes;

			m_buffers.taskQueues[i] = (void*)pool;
			pool += MAX_FRAGTASKS * 2 * m_userDataBytes;
		}
	}

	return &m_buffers;
}

USERDATABUFFERS* PuresoftUserDataManager::getBuffers(void)
{
	return &m_buffers;
}