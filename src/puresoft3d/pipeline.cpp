#include <windows.h>
#include <atlbase.h>
#include <stdlib.h>
#include <process.h>
#include <assert.h>
#include <memory.h>
#include <stdexcept>
#include "mcemaths.h"
#include "pipeline.h"
#include "proc.h"
#include "interp.h"
#include "rndrgdi.h"
#include "rndrddraw.h"

using namespace std;

int PuresoftPipeline::m_numberOfThreads = 3; // = getCpuCures();

PuresoftPipeline::PuresoftPipeline(uintptr_t canvasWindow, int width, int height, PuresoftRenderer* rndr /* = NULL */)
	: m_width(width)
	, m_height(height)
	, m_canvasWindow(canvasWindow)
	, m_vp(NULL)
	, m_ip(NULL)
	, m_fp(NULL)
	, m_rasterizer(width, height)
	, m_depth(width, ((int)(width / 4.0f + 0.5f) * 4) * sizeof(float), height, sizeof(float))
	, m_userDataPool(NULL)
	, m_renderer(NULL)
{
	assert(m_numberOfThreads <= MAX_FRAGTHREADS);
	memset(m_uniforms, 0, sizeof(m_uniforms));
	memset(m_fbos, 0, sizeof(m_fbos));
	memset(&m_userDataBuffers, 0, sizeof(m_userDataBuffers));

	if(rndr)
	{
		setRenderer(rndr);
	}
	else
	{
		setRenderer(new PuresoftGdiRenderer);
	}

	PURESOFTIMGBUFF32 rndrDesc;
	m_renderer->getDesc(&rndrDesc);

	m_display = new PuresoftFBO(rndrDesc.width, rndrDesc.scanline, rndrDesc.height, rndrDesc.elemLen, true, m_renderer->swapBuffers());
	setFBO(0, NULL);

	m_fragTaskQueues = new FragmentThreadTaskQueue[m_numberOfThreads];

	for(int i = 0; i < m_numberOfThreads; i++)
	{
		m_threads[i] = _beginthreadex(NULL, 0, fragmentThread, this, CREATE_SUSPENDED, NULL);
		ResumeThread((HANDLE)m_threads[i]);
//		SetThreadPriority((HANDLE)m_threads[i], THREAD_PRIORITY_ABOVE_NORMAL);
	}
}

PuresoftPipeline::~PuresoftPipeline(void)
{
	FRAGTHREADTASK task;
	task.eot = true;
	for(int i = 0; i < m_numberOfThreads; i++)
	{
		m_fragTaskQueues[i].push(task);
	}
	WaitForMultipleObjects(m_numberOfThreads, (const HANDLE*)m_threads, TRUE, INFINITE);

	for(int i = 0; i < m_numberOfThreads; i++)
	{
		CloseHandle((HANDLE)m_threads[i]);
	}

	delete[] m_fragTaskQueues;

	for(PROCCOLL::iterator i = m_processors.begin(); i != m_processors.end(); i++)
	{
		if(*i)
		{
			delete *i;
		}
	}

	for(FBOCOLL::iterator i = m_texPool.begin(); i != m_texPool.end(); i++)
	{
		if(*i)
		{
			delete *i;
		}
	}

	for(size_t i = 0; i < MAX_UNIFORMS; i++)
	{
		if(m_uniforms[i])
		{
			_aligned_free(m_uniforms[i]);
		}
	}

	setUserDataBytes(0);

	m_renderer->shutdown();
	m_renderer->release();
	delete m_display;
}

void PuresoftPipeline::setViewport(uintptr_t canvasWindow)
{
	m_canvasWindow = canvasWindow;
	m_renderer->setCanvas(canvasWindow);
}

void PuresoftPipeline::setRenderer(PuresoftRenderer* rndr)
{
	if(m_renderer)
	{
		m_renderer->shutdown();
		m_renderer->release();
	}

	if(m_renderer = rndr)
	{
		m_renderer->startup(m_canvasWindow, m_width, m_height);
	}
}

void PuresoftPipeline::setFBO(int idx, PuresoftFBO* fbo)
{
	if(idx < 0 || idx >= MAX_FBOS)
	{
		throw std::out_of_range("PuresoftPipeline::setFBO");
	}

	if(0 == idx && NULL == fbo)
	{
		m_fbos[0] = m_display;
	}
	else
	{
		m_fbos[idx] = fbo;
	}
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

void PuresoftPipeline::swapBuffers(void)
{
	void* backBuffer = m_renderer->swapBuffers();

	if(m_fbos[0] == m_display)
	{
		m_fbos[0]->setBuffer(backBuffer);
	}
}

void PuresoftPipeline::clearDepth(float furthest /* = 1.0f */)
{
	__declspec(align(16)) const float temp[4] = {furthest, furthest, furthest, furthest};
	m_depth.clear16(temp);
}

void PuresoftPipeline::clearColour(PURESOFTBGRA bkgnd /* = PURESOFTBGRA_BLACK */)
{
	m_display->clear4(&bkgnd);
}