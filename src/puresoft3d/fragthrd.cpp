#include <assert.h>
#include <stdexcept>
#include "pipeline.h"

using namespace std;

class FBOBridge : public FragmentProcessorOutput
{
	int m_threadIndex;
	int m_behavior;
	bool m_discarded;
	PuresoftFBO** m_fbos;
public:
	FBOBridge(int threadIndex, int behavior, PuresoftFBO** fbos)
		: m_threadIndex(threadIndex)
		, m_behavior(behavior)
		, m_discarded(false)
		, m_fbos(fbos)
	{}

	void discard(void)
	{
		m_discarded = true;
	}

	void read(int index, void* data, size_t bytes)
	{
		assert(0 <= index && index < MAX_FBOS);
		assert(m_fbos[index]);
		m_fbos[index]->read(m_threadIndex, data, bytes);
	}

	void read1(int index, void* data)
	{
		assert(0 <= index && index < MAX_FBOS);
		assert(m_fbos[index]);
		m_fbos[index]->read1(m_threadIndex, data);
	}

	void read4(int index, void* data)
	{
		assert(0 <= index && index < MAX_FBOS);
		assert(m_fbos[index]);
		m_fbos[index]->read4(m_threadIndex, data);
	}

	void read16(int index, void* data)
	{
		assert(0 <= index && index < MAX_FBOS);
		assert(m_fbos[index]);
		m_fbos[index]->read16(m_threadIndex, data);
	}

	void write(int index, const void* data, size_t bytes)
	{
		assert(0 <= index && index < MAX_FBOS);
		assert(m_fbos[index]);
		assert(0 == (m_behavior & BEHAVIOR_ALPHABLEND));
		m_fbos[index]->write(m_threadIndex, data, bytes);
	}

	void write1(int index, const void* data)
	{
		assert(0 <= index && index < MAX_FBOS);
		assert(m_fbos[index]);
		assert(0 == (m_behavior & BEHAVIOR_ALPHABLEND));
		m_fbos[index]->write1(m_threadIndex, data);
	}

	void write4(int index, const void* data)
	{
		assert(0 <= index && index < MAX_FBOS);
		assert(m_fbos[index]);
		if(m_behavior & BEHAVIOR_ALPHABLEND)
		{
			m_fbos[index]->blend4(m_threadIndex, (const unsigned char*)data);
		}
		else
		{
			m_fbos[index]->write4(m_threadIndex, data);
		}
	}

	void write16(int index, const void* data)
	{
		assert(0 <= index && index < MAX_FBOS);
		assert(m_fbos[index]);
		if(m_behavior & BEHAVIOR_ALPHABLEND)
		{
			m_fbos[index]->blend16(m_threadIndex, (const float*)data);
		}
		else
		{
			m_fbos[index]->write16(m_threadIndex, data);
		}
	}

	bool discarded(void)
	{
		if(m_discarded)
		{
			m_discarded = false;
			return true;
		}
		else
		{
			return false;
		}
	}
};

unsigned __stdcall PuresoftPipeline::fragmentThread(void *param)
{
	// thread start off parameters
	PuresoftPipeline* pThis = (PuresoftPipeline*)param;
	int threadIndex = 0;
	unsigned int myThreadId = GetThreadId(GetCurrentThread());
	for(; threadIndex < m_numberOfThreads; threadIndex++)
	{
		if(GetThreadId((HANDLE)pThis->m_threads[threadIndex]) == myThreadId)
			break;
	}

	// input data structure for Fragment Processor
	FragmentProcessorInput fragInput;

	FragmentThreadTaskQueue* taskQueue = pThis->m_fragTaskQueues + threadIndex;

	PuresoftInterpolater::INTERPOLATIONSTEPPING stepping;

	while(true)
	{
		FRAGTHREADTASK* task = taskQueue->beginPop();

		if(QUIT == task->taskType)
		{
			taskQueue->endPop();
			break;
		}
		else if(ENDOFDRAW == task->taskType)
		{
			taskQueue->endPop();
#ifdef PROFILING
			taskQueue->m_ignorePopSpinning = true;
#endif
			continue;
		}
		else if(POST == task->taskType)
		{
			PuresoftPostProcessor* postProc = task->postProc;
			postProc->process(threadIndex, pThis->m_numberOfThreads, pThis->m_fbos[0], pThis->m_depth);
			taskQueue->endPop();
			continue;
		}
		//else: DRAW
#ifdef PROFILING
		taskQueue->m_ignorePopSpinning = false;
#endif

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

		if(pThis->m_behavior & BEHAVIOR_UPDATE_DEPTH)
		{
			pThis->m_depth->setCurRow(threadIndex, y);
		}

		// set starting column to all attached fbos
		PuresoftFBO** fbos = pThis->m_fbos;
		for(size_t i = 0; i < MAX_FBOS; i++)
		{
			PuresoftFBO* fbo = *fbos;
			if(fbo)
			{
				fbo->setCurCol(threadIndex, x1);
			}
		}

		if(pThis->m_behavior & BEHAVIOR_UPDATE_DEPTH)
		{
			pThis->m_depth->setCurCol(threadIndex, x1);
		}

		FBOBridge fragOutput(threadIndex, pThis->m_behavior, pThis->m_fbos);

		// process rasterization result of a scanline, column by column
		for(int x = x1; x <= x2; x++)
		{
			fragInput.position[0] = x;

			// get interpolated values as well as the other perspective correction factor
			float newDepth;
			pThis->m_interpolater.interpolateNextStep(fragInput.user, &newDepth, &stepping);

			// get current depth from the depth buffer and do depth test
			float currentDepth;
			if(pThis->m_behavior & BEHAVIOR_TEST_DEPTH)
			{
				pThis->m_depth->read4(threadIndex, &currentDepth);
			}
			else
			{
				currentDepth = 1.0f;
			}

			if(-1.0f < newDepth && (newDepth - currentDepth < -0.0001f))
			                        //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ to avoid shared-edge double-drawing
			{
				// call Fragment Processor to update FBOs
				pThis->m_fp->process(&fragInput, &fragOutput);

				// update depth buffer
				if(!fragOutput.discarded() && (pThis->m_behavior & BEHAVIOR_UPDATE_DEPTH))
				{
					pThis->m_depth->write4(threadIndex, &newDepth);
				}
			}

			// move fbo data pointers
			PuresoftFBO** fbos = pThis->m_fbos;
			for(int i = 0; i < (int)MAX_FBOS; i++, fbos++)
			{
				PuresoftFBO* fbo = *fbos;
				if(fbo)
				{
					fbo->nextCol(threadIndex);
				}
			}

			pThis->m_depth->nextCol(threadIndex);
		}
	}

	return 0;
}

unsigned __stdcall PuresoftPipeline::fragmentThread_CallerThread(void *param)
{
	// thread start off parameters
	PuresoftPipeline* pThis = (PuresoftPipeline*)param;
	int threadIndex = m_numberOfThreads - 1;

	// input data structure for Fragment Processor
	FragmentProcessorInput fragInput;

	FragmentThreadTaskQueue* myQueue = pThis->m_fragTaskQueues + threadIndex;

	PuresoftInterpolater::INTERPOLATIONSTEPPING stepping;

	while(true)
	{
		FRAGTHREADTASK* task = myQueue->beginPop();

		if(QUIT == task->taskType)
		{
			myQueue->endPop();
			break;
		}

		bool dispatched = false;

		for(int i = 0; i < threadIndex; i++)
		{
			FragmentThreadTaskQueue* othersQueue = pThis->m_fragTaskQueues + i;

			if(0 == othersQueue->size())
			{
				FRAGTHREADTASK* newTask = othersQueue->beginPush();
				newTask->taskType = task->taskType;
				newTask->x1 = task->x1;
				newTask->x2 = task->x2;
				newTask->y = task->y;
				newTask->projZStart = task->projZStart;
				newTask->projZStep = task->projZStep;
				newTask->correctionFactor2Start = task->correctionFactor2Start;
				newTask->correctionFactor2Step = task->correctionFactor2Step;
				memcpy(newTask->userDataStart, task->userDataStart, pThis->m_ip->userDataBytes());
				memcpy(newTask->userDataStep, task->userDataStep, pThis->m_ip->userDataBytes());
				othersQueue->endPush();
				myQueue->endPop();
				dispatched = true;
				break;
			}
		}

		if(dispatched)
		{
			continue;
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

		myQueue->endPop();

		// set current row to all attached fbos
		for(size_t i = 0; i < MAX_FBOS; i++)
		{
			if(pThis->m_fbos[i])
			{
				pThis->m_fbos[i]->setCurRow(threadIndex, y);
			}
		}

		if(pThis->m_behavior & BEHAVIOR_UPDATE_DEPTH)
		{
			pThis->m_depth->setCurRow(threadIndex, y);
		}

		// set starting column to all attached fbos
		for(size_t i = 0; i < MAX_FBOS; i++)
		{
			if(pThis->m_fbos[i])
			{
				pThis->m_fbos[i]->setCurCol(threadIndex, x1);
			}
		}

		if(pThis->m_behavior & BEHAVIOR_UPDATE_DEPTH)
		{
			pThis->m_depth->setCurCol(threadIndex, x1);
		}

		FBOBridge fragOutput(threadIndex, pThis->m_behavior, pThis->m_fbos);

		// process rasterization result of a scanline, column by column
		for(int x = x1; x <= x2; x++)
		{
			fragInput.position[0] = x;

			// get interpolated values as well as the other perspective correction factor
			float newDepth;
			pThis->m_interpolater.interpolateNextStep(fragInput.user, &newDepth, &stepping);

			// get current depth from the depth buffer and do depth test
			float currentDepth;
			if(pThis->m_behavior & BEHAVIOR_TEST_DEPTH)
			{
				pThis->m_depth->read4(threadIndex, &currentDepth);
			}
			else
			{
				currentDepth = 1.0f;
			}

			if(-1.0f < newDepth && (newDepth - currentDepth < -0.0001f))
			                       //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ to avoid shared-edge double-drawing
			{
				// call Fragment Processor to update FBOs
				pThis->m_fp->process(&fragInput, &fragOutput);

				// update depth buffer
				if(!fragOutput.discarded() && (pThis->m_behavior & BEHAVIOR_UPDATE_DEPTH))
				{
					pThis->m_depth->write4(threadIndex, &newDepth);
				}
			}

			// move fbo data pointers
			for(size_t i = 0; i < MAX_FBOS; i++)
			{
				if(pThis->m_fbos[i])
				{
					pThis->m_fbos[i]->nextCol(threadIndex);
				}
			}
			pThis->m_depth->nextCol(threadIndex);
		}
	}

	return 0;
}