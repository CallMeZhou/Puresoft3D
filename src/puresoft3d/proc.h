#pragma once
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include "config.h"
#include "vao.h"
#include "mcemaths.hpp"

class align_base_64
{
public:
	void* operator new(size_t bytes) {return _aligned_malloc(bytes, 64);}
	void operator delete(void* mem) {_aligned_free(mem);}
	void* operator new(size_t bytes, void* place) {assert(0 == (intptr_t)place % 64);return place;}
	void operator delete(void* mem, void* place) {}
	void* operator new[](size_t bytes) {return _aligned_malloc(bytes, 64);}
	void operator delete[](void* mem) {_aligned_free(mem);}
	void* operator new[](size_t bytes, void* place) {assert(0 == (intptr_t)place % 64);return place;}
	void operator delete[](void* mem, void* place) {}
};

typedef struct
{
	const void* data[PuresoftVAO::MAX_VBOS];
} VertexProcessorInput;

typedef __declspec(align(16)) struct
{
	float position[4];
	void* user;
} VertexProcessorOutput;

typedef struct
{
	int position[2];
	void* user;
} FragmentProcessorInput;

typedef struct
{
	void* data[MAX_FBOS];
	size_t dataSizes[MAX_FBOS];
} FragmentProcessorOutput;

class PuresoftVertexProcessor : public align_base_64
{
public:
	virtual void release(void) = 0;
	virtual void process(const VertexProcessorInput* input, VertexProcessorOutput* output, const void** uniforms) = 0;
};

class PuresoftInterpolationProcessor : public align_base_64
{
public:
	virtual void release(void) = 0;
	virtual void setUserData(const void* const data[3]) = 0;
	virtual void processLeftEnd(const float* correctedContributes) = 0;
	virtual void processRightEnd(const float* correctedContributes) = 0;
	virtual void processDelta(float reciprocalScanlineLength) = 0;
	virtual void processOutput(float correctionFactor2, void* interpData) = 0;
};

class PuresoftFragmentProcessor : public align_base_64
{
public:
	virtual void release(void) = 0;
	virtual void process(const FragmentProcessorInput* input, FragmentProcessorOutput* output, const void** uniforms, const void** textures) = 0;
};

class PuresoftProcessorUserDataManager
{
public:
	virtual void release(void) = 0;
	virtual void alloc(size_t count) = 0;
	virtual void* get(size_t idx) = 0;
};

typedef void* (_stdcall *createProcessorInstance)(void);

class PuresoftProcessor
{
public:
	static const size_t MAX_FRAG_PIPES = 8;

public:
	PuresoftProcessor(createProcessorInstance vpc, createProcessorInstance ipc, createProcessorInstance fpc, createProcessorInstance udmc);
	~PuresoftProcessor();
	PuresoftProcessorUserDataManager* getUDM(void);
	PuresoftVertexProcessor* getVertProc(void);
	PuresoftInterpolationProcessor* getInterpProc(int idx);
	PuresoftFragmentProcessor* getFragProc(int idx);

private:
	PuresoftProcessorUserDataManager* m_udm;
	PuresoftVertexProcessor* m_vertProc;
	PuresoftInterpolationProcessor* m_interpProc[MAX_FRAG_PIPES];
	PuresoftFragmentProcessor* m_fragProc[MAX_FRAG_PIPES];
};