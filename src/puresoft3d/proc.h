#pragma once
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include "config.h"
#include "vao.h"

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

class PuresoftVertexProcessor : public align_base_64
{
public:
	typedef struct
	{
		const void* data[PuresoftVAO::MAX_VBOS];
	} VertexProcessorInput;

	typedef struct
	{
		__declspec(align(16)) float position[4];
		const void* ext; // usually to a structure instance embedded in child class of PuresoftVertexProcessor
	} VertexProcessorOutput;

public:
	virtual void release(void) = 0;
	virtual void process(const VertexProcessorInput* input, VertexProcessorOutput* output, const void** uniforms) = 0;
};

class PuresoftInterpolationProcessor : public align_base_64
{
public:
	virtual void release(void) = 0;
	virtual void setInputExt(int idx, const void* ext) = 0;
	virtual void processLeftEnd(const float* correctedContributes) = 0;
	virtual void processRightEnd(const float* correctedContributes) = 0;
	virtual void processDelta(float reciprocalScanlineLength) = 0;
	virtual void* processOutput(float correctionFactor2) = 0;
};

class PuresoftFragmentProcessor : public align_base_64
{
public:
	typedef struct
	{
		int position[2];
		const void* ext; // usually to a structure instance embedded in child class of PuresoftInterpolationProcessor
	} FragmentProcessorInput;

	typedef struct
	{
		const void* data[MAX_FBOS];
		size_t dataSizes[MAX_FBOS];
	} FragmentProcessorOutput;

public:
	virtual void release(void) = 0;
	virtual void process(const FragmentProcessorInput* input, FragmentProcessorOutput* output, const void** uniforms, const void** textures) = 0;
};

typedef void* (_stdcall *createProcessorInstance)(void);

class PuresoftProcessor
{
public:
	static const size_t MAX_FRAG_PIPES = 8;

public:
	PuresoftProcessor(createProcessorInstance vpc, createProcessorInstance ipc, createProcessorInstance fpc);
	~PuresoftProcessor();
	PuresoftVertexProcessor* getVertProc(void);
	PuresoftInterpolationProcessor* getInterpProc(int idx);
	PuresoftFragmentProcessor* getFragProc(int idx);

private:
	PuresoftVertexProcessor* m_vertProc;
	PuresoftInterpolationProcessor* m_interpProc[MAX_FRAG_PIPES];
	PuresoftFragmentProcessor* m_fragProc[MAX_FRAG_PIPES];
};