#include "pipeline.h"

void PuresoftPipeline::drawVAO(PuresoftVAO* vao)
{
	if(!m_processor)
	{
		return;
	}

	// reset vertex pointers of all vbos
	vao->rewindAll();

	const PuresoftRasterizer::RESULT* rasterResult = m_rasterizer.getResultPtr();
	__declspec(align(16)) float correctionFactor1[4] = {0};
	__declspec(align(16)) float projZs[4] = {0};
	PuresoftInterpolater::INTERPOLATIONSTARTSTEP interp;
	interp.proc = m_processor->getInterpProc();
	interp.vertexUserData[0] = m_vertOutput[0].user;
	interp.vertexUserData[1] = m_vertOutput[1].user;
	interp.vertexUserData[2] = m_vertOutput[2].user;
	interp.vertices = (const int*)rasterResult->vertices;
	interp.reciprocalWs = correctionFactor1;
	interp.projectedZs = projZs;

	while(true)
	{
		// vertex transformation
		if(!processVertices(vao->getVBOs(), m_vertOutput, correctionFactor1, projZs))
		{
			// done with all vertices
			break;
		}

		// cull back-face
		if(isBackFace(m_vertOutput[0].position, m_vertOutput[1].position, m_vertOutput[2].position))
		{
			continue;
		}

		// rasterization
		if(!m_rasterizer.pushTriangle(m_vertOutput[0].position, m_vertOutput[1].position, m_vertOutput[2].position))
		{
			// 0 sized triangle
			continue;
		}

		// interpolation + fragment filling
		for(interp.row = rasterResult->firstRow; interp.row <= rasterResult->lastRow; interp.row++)
		{
			// get one row of the rasterized triangle
			PuresoftRasterizer::RESULT_ROW& row = rasterResult->m_rows[interp.row];

			// skip zero length row
			if(row.left == row.right)
			{
				continue;
			}

			// partitioning (determine which fragment thread to process this row)
			int threadIndex = interp.row % m_numberOfThreads;
			FragmentThreadTaskQueue* taskQueue = m_fragTaskQueues + threadIndex;

			// interpolation (preparation of per-fragment interpolation)
			FRAGTHREADTASK* newTask = taskQueue->beginPush();
			interp.leftColumn = row.left;
			interp.rightColumn = row.right;
			interp.leftVerts = row.leftVerts;
			interp.rightVerts = row.rightVerts;
			interp.interpolatedUserDataStart = newTask->userDataStart;
			interp.interpolatedUserDataStep = newTask->userDataStep;
			m_interpolater.interpolateStartAndStep(&interp);

			// complete the task item
			newTask->eot = false;
			newTask->x1 = row.left;
			newTask->x2 = row.right;
			newTask->y = interp.row;
			newTask->projZStart = interp.projectedZStart;
			newTask->projZStep = interp.projectedZStep;
			newTask->correctionFactor2Start = interp.correctionFactor2Start;
			newTask->correctionFactor2Step = interp.correctionFactor2Step;

			// push to fragment thread
			taskQueue->endPush();
		}
	}

	// wait for all fragment threads to finish up tasks
	for(int i = 0; i < m_numberOfThreads; i++)
	{
		m_fragTaskQueues[i].pollEmpty();
	}
}