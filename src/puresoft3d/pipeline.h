#pragma once
#include "config.h"
#include "defs.h"
#include "mcemaths.hpp"
#include "vao.h"
#include "fbo.h"
#include "proc.h"
#include "interp.h"
#include "rasterizer.h"
#include "rinque.h"
#include "rndr.h"

const PURESOFTBGRA PURESOFTBGRA_BLACK = {0};

class PuresoftProcessor;
class PuresoftInterpolationProcessor;
__declspec(align(16)) class PuresoftPipeline : public mcemaths::align_base_16
{
public:
	PuresoftPipeline(uintptr_t canvasWindow, int width, int height, PuresoftRenderer* rndr = NULL);
	~PuresoftPipeline(void);

	// resource
	void setTexture(int idx, const wchar_t* filepath);
	void setRenderer(PuresoftRenderer* rndr);
	void setViewport(uintptr_t canvasWindow);
	void setProcessor(int idx, PuresoftProcessor* proc);
	void setFBO(int idx, PuresoftFBO* fbo);
	void setUniform(int idx, const void* data, size_t len);

	// draw
	void useProcessor();
	void drawVAO(PuresoftVAO* vao);
	void swapBuffers(void);

	// clear series
	void clearDepth(float furthest = 1.0f);
	void clearColour(PURESOFTBGRA bkgnd = PURESOFTBGRA_BLACK);

// fundamental data
private:
	int m_width;
	int m_height;
	uintptr_t m_canvasWindow;
	int m_currentProcessor;
	void* m_uniforms[MAX_UNIFORMS];
	PURESOFTIMGBUFF32 m_textures[MAX_TEXTURES];
	PuresoftProcessor* m_processors[MAX_PROCS];
	PuresoftInterpolater m_interpolater;
	PuresoftRasterizer m_rasterizer;
	PuresoftFBO* m_fbos[MAX_FBOS];
	PuresoftFBO m_depth;
	PuresoftFBO* m_display;
	PuresoftRenderer* m_renderer;

// user data buffer management
private:
	void* m_userDataPool;

	struct 
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
	} m_userDataBuffers;

	void setUserDataBytes(size_t bytes);

// vertex processing thread
// (it's a same-thread function-call instead of a thread for the time being, as I'm too poor to
// have a core-pool. would you lend me a couple of Xeon E5s? ^_^ )
private:
	VertexProcessorOutput m_vertOutput[3];
	// static unsigned __stdcall vertexThread(void *param);
	bool processVertices(PuresoftVBO** vbos, VertexProcessorOutput* output, float* reciprocalWs, float* projectedZs);
	bool isBackFace(float* vert0, float* vert1, float* vert2);

// fragment processing thread
private:
	static int m_numberOfThreads;
	uintptr_t m_threads[MAX_FRAGTHREADS];

	typedef struct 
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
	} FRAGTHREADTASK;

	typedef RingQueueMT<FRAGTHREADTASK, MAX_FRAGTASKS> FragmentThreadTaskQueue;
	FragmentThreadTaskQueue* m_fragTaskQueues;
	
	static unsigned __stdcall fragmentThread(void *param);
};

