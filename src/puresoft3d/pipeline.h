#pragma once
#include <vector>
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
#include "samplr2d.h"

const PURESOFTBGRA PURESOFTBGRA_BLACK = {0};

const int BEHAVIOR_UPDATE_DEPTH = 0x00000001;
const int BEHAVIOR_TEST_DEPTH   = 0x00000002;
const int BEHAVIOR_FACE_CULLING = 0x00000004;
const int BEHAVIOR_ALPHABLEND   = 0x00000008;

class PuresoftProcessor;
class PuresoftInterpolationProcessor;
__declspec(align(64)) class PuresoftPipeline : public mcemaths::align_base_64
{
public:
	PuresoftPipeline(uintptr_t canvasWindow, int deviceWidth, int deviceHeight, PuresoftRenderer* rndr = NULL);
	~PuresoftPipeline(void);

	// texture api
	int  createTexture(const PURESOFTIMGBUFF32* image, int extraLayers = 0, PuresoftFBO::WRAPMODE mode = PuresoftFBO::CLAMP);
	void destroyTexture(int idx);
	void getTexture(int idx, PURESOFTIMGBUFF32* image, PuresoftFBO::LAYER layer = PuresoftFBO::LAYER_DEFAULT);

	// processor api
	int  addProcessor(PuresoftProcessor* proc);
	void destroyProcessor(int idx);
	int  createProgramme(int vid, int iid, int fid);
	void destroyProgramme(int idx);
	void useProgramme(int idx);

	// vao api
	int createVAO(void);
	PuresoftVBO* attachVBO(int vao, int idx, PuresoftVBO* vbo);
	PuresoftVBO* detachVBO(int vao, int idx);
	PuresoftVBO* getVBO(int vao, int idx);
	void destroyVAO(int vao);

	// rendering api
	void setRenderer(PuresoftRenderer* rndr);
	void setViewport(int width, int height, uintptr_t canvasWindow = 0);
	void setDepth(int idx = -1);
	void setFBO(int idx, PuresoftFBO* fbo);
	void setUniform(int idx, const void* data, size_t len);
	void drawVAO(int vao, bool callerThrdForFragProc = false);
	void postProcess(PuresoftPostProcessor* processor);
	void swapBuffers(void);
	void enable(int behavior);
	void disable(int behavior);
	void clearDepth(float furthest = 1.0f);
	void clearColour(PURESOFTBGRA bkgnd = PURESOFTBGRA_BLACK);

	// debug api
	void saveTexture(int idx, const wchar_t* path, bool dataIsFloat);
	void saveTextureAsRaw(int idx, const wchar_t* path);
	void getTaskQCounters(unsigned int* taskPushCalled, unsigned int* taskPushSpinned, unsigned int* taskPopCalled, unsigned int* taskPopSpinned);

// fundamental data
private:
	int m_deviceWidth;
	int m_deviceHeight;
	uintptr_t m_canvasWindow;
	volatile int m_behavior;
	PuresoftInterpolater m_interpolater;
	PuresoftRasterizer m_rasterizer;
	const PuresoftRasterizer::RESULT* m_rasterResult;
	PuresoftFBO* m_fbos[MAX_FBOS];
	PuresoftFBO* m_depth;
	PuresoftFBO* m_display;
	PuresoftRenderer* m_renderer;
	PuresoftFBO m_defaultDepth;
	PURESOFTUNIFORM m_uniforms[MAX_UNIFORMS];

// processors
	typedef std::vector<PuresoftProcessor*> PROCCOLL;
	PROCCOLL m_processors;
	typedef struct {int vp; int ip; int fp;} PURESOFTPROGRAMME;
	typedef std::vector<PURESOFTPROGRAMME> PROGCOLL;
	PROGCOLL m_programmes;
	PuresoftVertexProcessor* m_vp;
	PuresoftInterpolationProcessor* m_ip;
	PuresoftFragmentProcessor* m_fp;

// textures
	typedef std::vector<PuresoftFBO*> FBOCOLL;
	FBOCOLL m_texPool;

// vao
	typedef std::vector<PuresoftVAO*> VAOCOLL;
	VAOCOLL m_vaoPool;

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

	enum FRAGTHREADTASKTYPE {NOOP, ENDOFDRAW, QUIT, DRAW, POST};
	typedef struct 
	{
		FRAGTHREADTASKTYPE taskType; // task type
		int x1;
		int x2;
		int y;
		float projZStart;
		float projZStep;
		float correctionFactor2Start;
		float correctionFactor2Step;
		void* userDataStart;
		void* userDataStep;
		PuresoftPostProcessor* postProc;
	} FRAGTHREADTASK;

	typedef RingQueueMT<FRAGTHREADTASK, MAX_FRAGTASKS> FragmentThreadTaskQueue;
	FragmentThreadTaskQueue* m_fragTaskQueues;
	
	static unsigned __stdcall fragmentThread(void *param);
	static unsigned __stdcall fragmentThread_CallerThread(void *param);
};

