#include "pipeline.h"

void PuresoftPipeline::postProcess(PuresoftPostProcessor* processor)
{
	for(int i = 0; i < m_numberOfThreads - 1; i++)
	{
 		FRAGTHREADTASK* task = m_fragTaskQueues[i].beginPush();
 		task->taskType = POST;
 		task->postProc = processor;
 		m_fragTaskQueues[i].endPush();
	}

	processor->process(m_numberOfThreads - 1, m_numberOfThreads, m_fbos[0], m_depth);

	for(int i = 0; i < m_numberOfThreads - 1; i++)
	{
		m_fragTaskQueues[i].pollEmpty();
	}
}
