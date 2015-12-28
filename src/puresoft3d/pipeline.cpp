#include <windows.h>
#include <atlbase.h>
#include <stdlib.h>
#include <process.h>
#include <memory.h>
#include <stdexcept>
#include "mcemaths.h"
#include "pipeline.h"
#include "proc.h"
#include "interp.h"

using namespace std;

PuresoftPipeline::PuresoftPipeline(int width, int height)
	: m_width(width)
	, m_height(height)
	, m_rasterizer(width, height)
	, m_depth(width, ((int)(width / 4.0f + 0.5f) * 4) * sizeof(float), height, sizeof(float))
{
	memset(m_textures, 0, sizeof(m_textures));
	memset(m_uniforms, 0, sizeof(m_uniforms));
	memset(m_fbos, 0, sizeof(m_fbos));
	m_processor = NULL;

	m_threadSharedData.rasterResult = m_rasterizer.getResultPtr();

	for(size_t i = 0; i < MAX_FRAGTHREADS; i++)
	{
		m_threadHungary[i] = (uintptr_t)CreateEventW(NULL, FALSE, FALSE, NULL);
		m_threadSetoff[i] = (uintptr_t)CreateEventW(NULL, FALSE, FALSE, NULL);
	}

	for(size_t i = 0; i < MAX_FRAGTHREADS; i++)
	{
		FRAGTHREADPARAM* param = (FRAGTHREADPARAM*)malloc(sizeof(FRAGTHREADPARAM));
		param->index = i;
		param->hostInstance = this;
		m_threads[i] = _beginthreadex(NULL, 0, fragmentThread, param, 0, NULL);
		SetThreadPriority((HANDLE)m_threads[i], THREAD_PRIORITY_ABOVE_NORMAL);
	}
}

PuresoftPipeline::~PuresoftPipeline(void)
{
	m_threadSharedData.m_threadsQuit = true;
	for(size_t i = 0; i < MAX_FRAGTHREADS; i++)
	{
		SetEvent((HANDLE)m_threadSetoff[i]);
	}
	WaitForMultipleObjects(MAX_FRAGTHREADS, (const HANDLE*)m_threads, TRUE, INFINITE);

	for(size_t i = 0; i < MAX_FRAGTHREADS; i++)
	{
		CloseHandle((HANDLE)m_threads[i]);
		CloseHandle((HANDLE)m_threadHungary[i]);
		CloseHandle((HANDLE)m_threadSetoff[i]);
	}

	for(int i = 0; i < MAX_UNIFORMS; i++)
	{
		if(m_uniforms[i])
		{
			_aligned_free(m_uniforms[i]);
		}
	}
}

void PuresoftPipeline::setTexture(int idx, void* sampler)
{
	if(idx < 0 || idx >= MAX_TEXTURES)
	{
		throw std::out_of_range("PuresoftPipeline::setTexture");
	}

	m_textures[idx] = sampler;
}

void PuresoftPipeline::setViewport()
{
}

void PuresoftPipeline::setProcessor(PuresoftProcessor* proc)
{
	m_processor = proc;
	m_interpolater.setProcessor(proc);
}

void PuresoftPipeline::setFBO(int idx, PuresoftFBO* fbo)
{
	if(idx < 0 || idx >= MAX_FBOS)
	{
		throw std::out_of_range("PuresoftPipeline::setFBO");
	}

	m_fbos[idx] = fbo;
}

void PuresoftPipeline::setUniform(int idx, const void* data, size_t len)
{
	if(idx < 0 || idx >= MAX_UNIFORMS)
	{
		throw std::out_of_range("PuresoftPipeline::setUniform");
	}
	
	if(m_uniforms[idx])
	{
		_aligned_free(m_uniforms[idx]);
	}

	if(NULL == (m_uniforms[idx] = _aligned_malloc(len, 16)))
	{
		throw bad_alloc("PuresoftPipeline::setUniform");
	}

	memcpy(m_uniforms[idx], data, len);
}

void PuresoftPipeline::drawVAO(PuresoftVAO* vao)
{
	if(!m_processor)
	{
		return;
	}

	const PuresoftRasterizer::RESULT* rasterResult = m_rasterizer.getResultPtr();

	// get pointers to all vbos, reset all data pointers
	PuresoftVBO** vbos = vao->getVBOs();
	vao->rewindAll();

	// input data structure for Vertex Processor
	PuresoftVertexProcessor::VertexProcessorInput vertInput;
	// output data structure for Vertex Processor
	PuresoftVertexProcessor::VertexProcessorOutput vertOutput[3];
	// this is for interpolation perspective correction, this is the divider
	__declspec(align(16)) float correctionFactor1[4] = {0};
	// projected Z coordinates (divided by W), collect them for interpolation, interpolate for depth
	__declspec(align(16)) float projZs[4] = {0};

	m_threadSharedData.correctionFactor1 = correctionFactor1;
	m_threadSharedData.projZs = projZs;

	// vbo processing start
	while(true)
	{
		// process 3 elements
		// input:     vbo
		// processor: Vertex Processor
		// output:    vertOutput
		// next step: rasterization
		for(int i = 0; i < 3; i++)
		{
			// collect element data from each available vbo
			for(size_t j = 0; j < PuresoftVAO::MAX_VBOS; j++)
			{
				if(vbos[j])
				{
					if(NULL == (vertInput.data[j] = vbos[j]->next(0)))
					{
						return;
					}
				}
			}

			// call Vertex Processor
			m_processor->getVertProc()->process(&vertInput, &vertOutput[i], (const void**)m_uniforms);

			// check Vertex Processor's output at Interpolation Processor
			// we don't touch Vertex Processor output except the 'position'
			// we use 'position' to do rasterization
			m_interpolater.setProcessorInputExt(i, vertOutput[i].ext);

			// process the 'position'

			// homogenizing division (reciprocal W is negative reciprocal Z)
			float reciprocalW = 1.0f / vertOutput[i].position[3];
			mcemaths_mul_3_4(vertOutput[i].position, reciprocalW);

			// collect projected Zs, we'll interpolate it for depth later
			projZs[i] = vertOutput[i].position[2];

			// save -(1/Z) as one of the two factors for interpolation perspective correction
			// BTW, we do interpolation perspective correction in this way:
			// V_interp = z * (contibute_1 * V_1 / z + ... contibute_n * V_n / z)
			// we'll call '/z' correction factor 1 and 'z*' correction factor 2
			correctionFactor1[i] = reciprocalW;
		}

		// cull back face in naive way
 		__declspec(align(16)) float vec0[4], vec1[4], cross[4], test[4] = {0, 0, 1.0f, 0};
 		mcemaths_sub_3_4(vec0, vertOutput[1].position, vertOutput[0].position);
 		mcemaths_sub_3_4(vec1, vertOutput[2].position, vertOutput[1].position);
 		mcemaths_cross_3(cross, vec0, vec1);
 		mcemaths_norm_3_4(cross);
 		if(mcemaths_dot_3_4(test, cross) < 0)
 		{
 			continue;
 		}

		// do rasterization
		// output: const PuresoftRasterizer::RESULT* rasterResult
		if(!m_rasterizer.pushTriangle(vertOutput[0].position, vertOutput[1].position, vertOutput[2].position))
		{
			continue;
		}

		// process rasterization result, scanline by scanline
		for(size_t i = 0; i < MAX_FRAGTHREADS; i++)
		{
			SetEvent((HANDLE)m_threadSetoff[i]);
		}
		WaitForMultipleObjects(MAX_FRAGTHREADS, (const HANDLE*)m_threadHungary, TRUE, INFINITE);
	}
}

void PuresoftPipeline::clearDepth(float furthest /* = 1.0f */)
{
	__declspec(align(16)) const float temp[4] = {furthest, furthest, furthest, furthest};
	m_depth.clear16(temp);
}

unsigned __stdcall PuresoftPipeline::fragmentThread(void *param)
{
	// thread start off parameters
	int threadIndex = ((FRAGTHREADPARAM*)param)->index;
	PuresoftPipeline* pThis = ((FRAGTHREADPARAM*)param)->hostInstance;

	// input data structure for Fragment Processor
	PuresoftFragmentProcessor::FragmentProcessorInput fragInput;
	// output data structure for Vertex Processor
	PuresoftFragmentProcessor::FragmentProcessorOutput fragOuput;

	FRAGTHREADSHARED& shared = pThis->m_threadSharedData;

	while(true)
	{
		WaitForSingleObject((HANDLE)pThis->m_threadSetoff[threadIndex], INFINITE);

		if(shared.m_threadsQuit)
		{
			break;
		}

		int y = shared.rasterResult->firstRow + threadIndex;

		// set current row to all attached fbos
		for(size_t i = 0; i < MAX_FBOS; i++)
		{
			if(pThis->m_fbos[i])
			{
				pThis->m_fbos[i]->setCurRow(threadIndex, y);
			}
		}

		pThis->m_depth.setCurRow(threadIndex, y);

		// for each scanline
		for(; y <= shared.rasterResult->lastRow; y += MAX_FRAGTHREADS)
		{
			int x = shared.rasterResult->m_rows[y].left;

			// set starting column to all attached fbos
			for(size_t i = 0; i < MAX_FBOS; i++)
			{
				if(pThis->m_fbos[i])
				{
					pThis->m_fbos[i]->setCurCol(threadIndex, x);
				}
			}

			pThis->m_depth.setCurCol(threadIndex, x);

			// calculate interpolated values for the first and last pixel of scanline
			// scanlineBegin calculates delta interpolated values as well
			PuresoftInterpolater::SCANLINE_BEGIN_PARAMS scanlineParams;
			scanlineParams.vertices = (const int*)shared.rasterResult->vertices;
			scanlineParams.reciprocalWs = shared.correctionFactor1;
			scanlineParams.projectedZs = shared.projZs;
			scanlineParams.row = y;
			scanlineParams.leftColumn = shared.rasterResult->m_rows[y].left;
			scanlineParams.rightColumn = shared.rasterResult->m_rows[y].right;
			scanlineParams.leftVerts = shared.rasterResult->m_rows[y].leftVerts;
			scanlineParams.rightVerts = shared.rasterResult->m_rows[y].rightVerts;
			pThis->m_interpolater.scanlineBegin(threadIndex, &scanlineParams);

			// process rasterization result of a scanline, column by column

			for(; x <= shared.rasterResult->m_rows[y].right; x++)
			{
				// get interpolated values as well as the other perspective correction factor
				float newDepth;
				pThis->m_interpolater.scanlineNext(threadIndex, &newDepth, &fragInput.ext);
				fragInput.position[0] = x;
				fragInput.position[1] = y;

				// get current depth from the depth buffer and do depth test
				float currentDepth;
				pThis->m_depth.read4(threadIndex, &currentDepth);

				if(newDepth < currentDepth) // depth test passed
				{
					// update depth buffer
					pThis->m_depth.write4(threadIndex, &newDepth);
					pThis->m_depth.nextCol(threadIndex);

					// go ahead with Fragment Processor
					pThis->m_processor->getFragProc(threadIndex)->process(&fragInput, &fragOuput, (const void**)pThis->m_uniforms, (const void**)pThis->m_textures);

					// update each attached fbo with Fragment Processor output
					for(size_t i = 0; i < MAX_FBOS; i++)
					{
						if(pThis->m_fbos[i])
						{
							pThis->m_fbos[i]->write(threadIndex, fragOuput.data[i], fragOuput.dataSizes[i]);
							pThis->m_fbos[i]->nextCol(threadIndex);
						}
					}
				}
				else // depth test passed, skip everything of the current column
				{
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

			// for each attached fbo: go to next row
			for(size_t i = 0; i < MAX_FBOS; i++)
			{
				if(pThis->m_fbos[i])
				{
					for(int j = 0; j < MAX_FRAGTHREADS; j++)
					{
						pThis->m_fbos[i]->nextRow(threadIndex);
					}
				}
			}

			for(int j = 0; j < MAX_FRAGTHREADS; j++)
			{
				pThis->m_depth.nextRow(threadIndex);
			}
		}

		SetEvent((HANDLE)pThis->m_threadHungary[threadIndex]);
	}

	free(param);
	return 0;
}