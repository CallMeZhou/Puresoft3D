#pragma once
#include "vao.h"
#include "pipeline.h" // for MAX_FBOS

class PuresoftVertexProcessor
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
	virtual ~PuresoftVertexProcessor(void) {};
	virtual void process(const VertexProcessorInput* input, VertexProcessorOutput* output, const void** uniforms) = 0;
};

class PuresoftInterpolationProcessor
{
public:
	virtual ~PuresoftInterpolationProcessor(void) {};
	virtual void setInputExt(int idx, const void* ext) = 0;
	virtual void scanlineBegin(int left, int right, int y, const float* contributesForLeft, const float* contributesForRight, const float* correctionFactor1) = 0;
	virtual void scanlineNext(float* correctionFactor2, const void** outputExt) = 0;
};

class PuresoftFragmentProcessor
{
public:
	typedef struct
	{
		int position[2];
		const void* ext; // usually to a structure instance embedded in child class of PuresoftInterpolationProcessor
	} FragmentProcessorInput;

	typedef struct
	{
		const void* data[PuresoftPipeline::MAX_FBOS];
		size_t dataSizes[PuresoftPipeline::MAX_FBOS];
	} FragmentProcessorOutput;

public:
	virtual ~PuresoftFragmentProcessor(void) {};
	virtual void process(const FragmentProcessorInput* input, FragmentProcessorOutput* output, const void** uniforms, const void** textures) = 0;
};

class PuresoftProcessor
{
	friend class PuresoftPipeline;
public:
	PuresoftProcessor(PuresoftVertexProcessor* vp, PuresoftInterpolationProcessor* ip, PuresoftFragmentProcessor* fp)
		: m_vertProc(vp), m_interpProc(ip), m_fragProc(fp) {}
private:
	PuresoftVertexProcessor* m_vertProc;
	PuresoftInterpolationProcessor* m_interpProc;
	PuresoftFragmentProcessor* m_fragProc;
};