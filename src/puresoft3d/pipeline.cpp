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

int PuresoftPipeline::m_numberOfThreads = 4; // = getCpuCures();

PuresoftPipeline::PuresoftPipeline(uintptr_t canvasWindow, int deviceWidth, int deviceHeight, PuresoftRenderer* rndr /* = NULL */)
	: m_deviceWidth(deviceWidth)
	, m_deviceHeight(deviceHeight)
	, m_canvasWindow(canvasWindow)
	, m_vp(NULL)
	, m_ip(NULL)
	, m_fp(NULL)
	, m_defaultDepth(deviceWidth, ((int)(deviceWidth / 4.0f + 0.5f) * 4) * sizeof(float), deviceHeight, sizeof(float))
	, m_userDataPool(NULL)
	, m_renderer(NULL)
	, m_behavior(BEHAVIOR_UPDATE_DEPTH | BEHAVIOR_TEST_DEPTH | BEHAVIOR_FACE_CULLING)
{
	assert(m_numberOfThreads <= MAX_FRAGTHREADS);
	memset(m_uniforms, 0, sizeof(m_uniforms));
	memset(m_fbos, 0, sizeof(m_fbos));
	memset(&m_userDataBuffers, 0, sizeof(m_userDataBuffers));

	m_rasterResult = m_rasterizer.initialize(deviceWidth, deviceHeight);

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

	setDepth(); // use default depth

	m_fragTaskQueues = new FragmentThreadTaskQueue[m_numberOfThreads];

	for(int i = 0; i < m_numberOfThreads - 1; i++)
	{
		m_threads[i] = _beginthreadex(NULL, 0, fragmentThread, this, CREATE_SUSPENDED, NULL);
		ResumeThread((HANDLE)m_threads[i]);
//		SetThreadPriority((HANDLE)m_threads[i], THREAD_PRIORITY_ABOVE_NORMAL);
	}
	m_threads[m_numberOfThreads - 1] = (uintptr_t)GetCurrentThread();
}

PuresoftPipeline::~PuresoftPipeline(void)
{
	for(int i = 0; i < m_numberOfThreads - 1; i++)
	{
		FragmentThreadTaskQueue* taskQueue = m_fragTaskQueues + i;
		taskQueue->beginPush()->taskType = QUIT;
		taskQueue->endPush();
	}
	WaitForMultipleObjects(m_numberOfThreads - 1, (const HANDLE*)m_threads, TRUE, INFINITE);

	for(int i = 0; i < m_numberOfThreads - 1; i++)
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
		if(m_uniforms[i].data)
		{
			_aligned_free(m_uniforms[i].data);
		}
	}

	for(size_t i = 0; i < m_vaoPool.size(); i++)
	{
		destroyVAO(i);
	}
	

	setUserDataBytes(0);

	m_renderer->shutdown();
	m_renderer->release();
	delete m_display;
}

int PuresoftPipeline::createVAO(void)
{
	size_t vacant = 0;
	for(; vacant < m_vaoPool.size(); vacant++)
	{
		if(!m_vaoPool[vacant])
			break;
	}

	if(m_vaoPool.size() == vacant)
	{
		m_vaoPool.push_back(NULL);
	}

	m_vaoPool[vacant] = new PuresoftVAO;

	return (int)vacant;

}

PuresoftVBO* PuresoftPipeline::attachVBO(int vao, int idx, PuresoftVBO* vbo)
{
	if(vao < 0 || vao >= (int)m_vaoPool.size())
	{
		throw std::out_of_range("PuresoftPipeline::attachVBO vao");
	}

	if(idx < 0 || idx >= (int)MAX_VBOS)
	{
		throw std::out_of_range("PuresoftPipeline::attachVBO idx");
	}

	return m_vaoPool[vao]->attachVBO(idx, vbo);
}

PuresoftVBO* PuresoftPipeline::detachVBO(int vao, int idx)
{
	if(vao < 0 || vao >= (int)m_vaoPool.size())
	{
		throw std::out_of_range("PuresoftPipeline::attachVBO vao");
	}

	if(idx < 0 || idx >= (int)MAX_VBOS)
	{
		throw std::out_of_range("PuresoftPipeline::attachVBO idx");
	}

	return m_vaoPool[vao]->detachVBO(idx);
}

PuresoftVBO* PuresoftPipeline::getVBO(int vao, int idx)
{
	if(vao < 0 || vao >= (int)m_vaoPool.size())
	{
		throw std::out_of_range("PuresoftPipeline::attachVBO vao");
	}

	if(idx < 0 || idx >= (int)MAX_VBOS)
	{
		throw std::out_of_range("PuresoftPipeline::attachVBO idx");
	}

	return m_vaoPool[vao]->getVBO(idx);
}

void PuresoftPipeline::destroyVAO(int vao)
{
	if(vao < 0 || vao >= (int)m_vaoPool.size())
	{
		throw std::out_of_range("PuresoftPipeline::attachVBO vao");
	}

	PuresoftVAO* vaoObj = m_vaoPool[vao];
	if(!vaoObj)
		return;

	PuresoftVBO** vbos = vaoObj->getVBOs();
	for(size_t i = 0; i < MAX_VBOS; i++)
	{
		if(vbos[i])
		{
			delete vbos[i];
		}
	}

	delete vaoObj;
	m_vaoPool[vao] = NULL;
}

void PuresoftPipeline::setViewport(int width, int height, uintptr_t canvasWindow /* = 0 */)
{
	m_rasterResult = m_rasterizer.initialize(width, height);

	if(canvasWindow)
	{
		m_canvasWindow = canvasWindow;
		m_renderer->setCanvas(canvasWindow);
	}
}

void PuresoftPipeline::setDepth(int idx /* = -1 */)
{
	if(-1 == idx)
	{
		m_depth = &m_defaultDepth;
	}
	else
	{
		if(idx < 0 || idx >= (int)m_texPool.size())
		{
			throw std::out_of_range("PuresoftPipeline::setDepth");
		}

		// only support single float depth buffer, the scanline of which must be multiplication of 4
		if(4 != m_texPool[idx]->getElemLen() || (0 != m_texPool[idx]->getScanline() % 4))
		{
			throw std::invalid_argument("PuresoftPipeline::setDepth");
		}

		m_depth = m_texPool[idx];
	}
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
		m_renderer->startup(m_canvasWindow, m_deviceWidth, m_deviceHeight);
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
	
	if(!data)
	{
		if(m_uniforms[idx].data)
		{
			_aligned_free(m_uniforms[idx].data);
			m_uniforms[idx].data = NULL;
			m_uniforms[idx].capacity = 0;
		}
	}
	else
	{
		if(m_uniforms[idx].capacity < len)
		{
			if(m_uniforms[idx].data)
			{
				_aligned_free(m_uniforms[idx].data);
			}

			if(NULL == (m_uniforms[idx].data = _aligned_malloc(len, 16)))
			{
				throw bad_alloc("PuresoftPipeline::setUniform");
			}

			m_uniforms[idx].capacity = len;
		}

		memcpy(m_uniforms[idx].data, data, len);
	}
}

void PuresoftPipeline::swapBuffers(void)
{
	void* backBuffer = m_renderer->swapBuffers();

	if(m_fbos[0] == m_display)
	{
		m_fbos[0]->setBuffer(backBuffer);
	}
}

void PuresoftPipeline::enable(int behavior)
{
	m_behavior |= behavior;
}

void PuresoftPipeline::disable(int behavior)
{
	m_behavior &= ~behavior;
}

void PuresoftPipeline::clearDepth(float furthest /* = 1.0f */)
{
	__declspec(align(16)) const float temp[4] = {furthest, furthest, furthest, furthest};
	m_depth->clear16(temp);
}

void PuresoftPipeline::clearColour(PURESOFTBGRA bkgnd /* = PURESOFTBGRA_BLACK */)
{
	m_display->clear4(&bkgnd);
}