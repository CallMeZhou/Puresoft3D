#pragma once
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include "config.h"

typedef struct
{
	const void* data[MAX_VBOS];
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

class FragmentProcessorOutput
{
public: virtual void write(int index, const void* data, size_t bytes) = 0;
};

class PuresoftVertexProcessor
{
public:
	virtual void release(void) = 0;
	virtual void process(const VertexProcessorInput* input, VertexProcessorOutput* output, const void** uniforms) = 0;
};

class PuresoftInterpolationProcessor
{
public:
	virtual void release(void) = 0;
	virtual void interpolateByContributes(void* interpolatedUserData, const void** vertexUserData, const float* correctedContributes) = 0;
	virtual void calcStep(void* interpolatedUserDataStep, const void* interpolatedUserDataStart, const void* interpolatedUserDataEnd, int stepCount) = 0;
	virtual void interpolateBySteps(void* interpolatedUserData, void* interpolatedUserDataStart, const void* interpolatedUserDataStep, float correctionFactor2) = 0;
};

class PuresoftFragmentProcessor
{
public:
	virtual void release(void) = 0;
	virtual void process(const FragmentProcessorInput* input, FragmentProcessorOutput* output, const void** uniforms, const void** textures) = 0;
};

typedef void* (_stdcall *createProcessorInstance)(void);

class PuresoftProcessor
{
public:
	PuresoftProcessor(createProcessorInstance vpc, createProcessorInstance ipc, createProcessorInstance fpc, size_t userDataBytes);
	~PuresoftProcessor();
	size_t getUserDataBytes(void) const;
	PuresoftVertexProcessor* getVertProc(void);
	PuresoftInterpolationProcessor* getInterpProc(void);
	PuresoftFragmentProcessor* getFragProc(void);

private:
	size_t m_userDataBytes;
	PuresoftVertexProcessor* m_vertProc;
	PuresoftInterpolationProcessor* m_interpProc;
	PuresoftFragmentProcessor* m_fragProc;
};