#include <assert.h>
#include <stdexcept>
#include "pipeline.h"

using namespace std;

class FBOBridge : public FragmentProcessorOutput
{
	int m_threadIndex;
	PuresoftFBO** m_fbos;
public:
	FBOBridge(int threadIndex, PuresoftFBO** fbos)
		: m_threadIndex(threadIndex)
		, m_fbos(fbos)
	{}

	void write(int index, const void* data, size_t bytes)
	{
		if(index < 0 || index >= MAX_FBOS)
		{
			throw std::out_of_range("PuresoftPipeline::FragmentProcessorOutput::write");
		}

		assert(m_fbos[index]);

		m_fbos[index]->write(m_threadIndex, data, bytes);
	}
};

unsigned __stdcall PuresoftPipeline::fragmentThread(void *param)
{
	// thread start off parameters
	PuresoftPipeline* pThis = (PuresoftPipeline*)param;
	int threadIndex = 0;
	for(; threadIndex < m_numberOfThreads; threadIndex++)
	{
		if(GetThreadId((HANDLE)pThis->m_threads[threadIndex]) == GetThreadId(GetCurrentThread()))
			break;
	}

	// input data structure for Fragment Processor
	FragmentProcessorInput fragInput;
	FBOBridge fragOutput(threadIndex, pThis->m_fbos);

	FragmentThreadTaskQueue* taskQueue = pThis->m_fragTaskQueues + threadIndex;

	PuresoftInterpolater::INTERPOLATIONSTEPPING stepping;

	while(true)
	{
		FRAGTHREADTASK* task = taskQueue->beginPop();

		if(task->eot)
		{
			taskQueue->endPop();
			break;
		}

		int x1 = task->x1, x2 = task->x2, y = task->y;
		fragInput.user = pThis->m_userDataBuffers.fragInputs[threadIndex];
		fragInput.position[1] = y;
		stepping.proc = pThis->m_ip;
		stepping.interpolatedUserDataStart = pThis->m_userDataBuffers.interpTemps[threadIndex];
		stepping.interpolatedUserDataStep = (void*)((size_t)stepping.interpolatedUserDataStart + pThis->m_userDataBuffers.unitBytes);
		memcpy(stepping.interpolatedUserDataStart, task->userDataStart, pThis->m_userDataBuffers.unitBytes);
		memcpy(stepping.interpolatedUserDataStep, task->userDataStep, pThis->m_userDataBuffers.unitBytes);
		stepping.correctionFactor2Start = task->correctionFactor2Start;
		stepping.correctionFactor2Step = task->correctionFactor2Step;
		stepping.projectedZStart = task->projZStart;
		stepping.projectedZStep = task->projZStep;

		taskQueue->endPop();

		// set current row to all attached fbos
		for(size_t i = 0; i < MAX_FBOS; i++)
		{
			if(pThis->m_fbos[i])
			{
				pThis->m_fbos[i]->setCurRow(threadIndex, y);
			}
		}

		pThis->m_depth.setCurRow(threadIndex, y);

		// set starting column to all attached fbos
		for(size_t i = 0; i < MAX_FBOS; i++)
		{
			if(pThis->m_fbos[i])
			{
				pThis->m_fbos[i]->setCurCol(threadIndex, x1);
			}
		}

		pThis->m_depth.setCurCol(threadIndex, x1);

		// process rasterization result of a scanline, column by column
		for(int x = x1; x <= x2; x++)
		{
			fragInput.position[0] = x;

			// get interpolated values as well as the other perspective correction factor
			float newDepth;
			pThis->m_interpolater.interpolateNextStep(fragInput.user, &newDepth, &stepping);

			// get current depth from the depth buffer and do depth test
			float currentDepth;
			pThis->m_depth.read4(threadIndex, &currentDepth);

			if(newDepth < currentDepth) // depth test passed
			{
				// update depth buffer
				pThis->m_depth.write4(threadIndex, &newDepth);

				// go ahead with Fragment Processor (fbos are updated meanwhile)
				pThis->m_fp->process(&fragInput, &fragOutput);
			}

			// move fbo data pointers
			for(size_t i = 0; i < MAX_FBOS; i++)
			{
				if(pThis->m_fbos[i])
				{
					pThis->m_fbos[i]->nextCol(threadIndex);
				}
			}
			pThis->m_depth.nextCol(threadIndex);
		}
	}

	return 0;
}