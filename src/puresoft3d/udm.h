#pragma once
#include "config.h"

typedef struct 
{
	// verts for 3 vertices;
	void* verts;
	// tempInterp for INTERPOLATIONSTARTSTEP::interpolatedUserDataStart and interpolatedUserDataStep
	void* tempInterp;
	// fragInputs[0] ... fragInputs[max_threads-1] for fragment inputs of threads
	void* fragInputs[MAX_FRAGTHREADS];
	// taskQueues[0] .. taskQueues[max_threads-1] for task queues of threads
	void* taskQueues[MAX_FRAGTHREADS];
} USERDATABUFFERS;

class PuresoftUserDataManager
{
public:
	PuresoftUserDataManager(size_t cores);
	~PuresoftUserDataManager(void);

	USERDATABUFFERS* setUserDataBytes(size_t bytes);
	USERDATABUFFERS* getBuffers(void);

private:
	size_t m_cores;
	size_t m_userDataBytes;
	void* m_pool;
	USERDATABUFFERS m_buffers;
};

