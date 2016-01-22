#pragma once
#include "config.h"
#include "vao.h"
#include "fbo.h"
#include "proc.h"
#include "interp.h"
#include "rasterizer.h"
#include "rinque.h"
#include "udm.h"

class PuresoftProcessor;
class PuresoftInterpolationProcessor;
class PuresoftPipeline
{
public:
	PuresoftPipeline(int width, int height);
	~PuresoftPipeline(void);

	// resource
	void setTexture(int idx, void* sampler);

	// draw
	void setViewport();
	void setProcessor(PuresoftProcessor* proc);
	void setFBO(int idx, PuresoftFBO* fbo);
	void setUniform(int idx, const void* data, size_t len);
	void drawVAO(PuresoftVAO* vao);

	// clear series
	void clearDepth(float furthest = 1.0f);

private:
	int m_width;
	int m_height;
	void* m_textures[MAX_TEXTURES];
	void* m_uniforms[MAX_UNIFORMS];
	PuresoftProcessor* m_processor;
	PuresoftInterpolater m_interpolater;
	PuresoftRasterizer m_rasterizer;
	PuresoftFBO* m_fbos[MAX_FBOS];
	PuresoftFBO m_depth;

private:
	struct FRAGTHREADPARAM
	{
		int index;
		PuresoftPipeline* hostInstance;
	};

	struct FRAGTHREADSHARED
	{
		volatile bool m_threadsQuit;
		const PuresoftRasterizer::RESULT* rasterResult;
		float* correctionFactor1;
		float* projZs;
		void* userData[3];
	};

	static size_t m_numberOfThreads;
	PuresoftUserDataManager m_udm;

	uintptr_t m_threads[MAX_FRAGTHREADS];
	FRAGTHREADSHARED m_threadSharedData;

	struct FRAGTHREADTASK
	{
		int x1;
		int x2;
		int y;
		float projZStart;
		float projZStep;
		float correctionFactor2Start;
		float correctionFactor2Step;
		void* userDataStart;
		void* userDataStep;
		bool eot; // end of tasks
	};

	typedef RingQueueMT<FRAGTHREADTASK, MAX_FRAGTASKS> FragmentThreadTaskQueue;
	FragmentThreadTaskQueue* m_fragTaskQueues;
	
	static unsigned __stdcall fragmentThread(void *param);
};

