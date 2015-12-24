#pragma once
#include "config.h"
#include "vao.h"
#include "fbo.h"
#include "proc.h"
#include "interp.h"
#include "rasterizer.h"

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
	};

	static const size_t MAX_FRAGTHREADS = 4;
	uintptr_t m_threadSetoff[MAX_FRAGTHREADS];
	uintptr_t m_threadHungary[MAX_FRAGTHREADS];
	uintptr_t m_threads[MAX_FRAGTHREADS];
	FRAGTHREADSHARED m_threadSharedData;
	
	static unsigned __stdcall fragmentThread(void *param);
};

