#pragma once
#include "config.h"

typedef struct 
{
	size_t unitBytes;
	// verts for 3 vertices;
	void* verts;
	// interpTemps[0] .. interpTemps[max_threads-1] for interpolation temporaries of threads
	void* interpTemps[MAX_FRAGTHREADS];
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

