#pragma once
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include "config.h"

class PuresoftProcessor
{
public:
	virtual ~PuresoftProcessor();
	virtual size_t userDataBytes(void) const;
};

typedef struct
{
	const void* data[MAX_VBOS];
} VertexProcessorInput;

typedef __declspec(align(16)) struct
{
	float position[4];
	void* user;
} VertexProcessorOutput;

class PuresoftVertexProcessor : public PuresoftProcessor
{
public:
	virtual ~PuresoftVertexProcessor() {}
	virtual void preprocess(const void** uniforms) = 0;
	virtual void process(const VertexProcessorInput* input, VertexProcessorOutput* output) const = 0;
};

class PuresoftInterpolationProcessor : public PuresoftProcessor
{
public:
	virtual ~PuresoftInterpolationProcessor() {}
	virtual void preprocess(const void** uniforms) = 0;
	virtual void interpolateByContributes(void* interpolatedUserData, const void** vertexUserData, const float* correctedContributes) const = 0;
	virtual void calcStep(void* interpolatedUserDataStep, const void* interpolatedUserDataStart, const void* interpolatedUserDataEnd, int stepCount) const = 0;
	virtual void interpolateBySteps(void* interpolatedUserData, void* interpolatedUserDataStart, const void* interpolatedUserDataStep, float correctionFactor2) const = 0;
};

typedef struct
{
	int position[2];
	void* user;
} FragmentProcessorInput;

class FragmentProcessorOutput
{
public: virtual void write(int index, const void* data, size_t bytes) = 0;
};

class PuresoftFragmentProcessor : public PuresoftProcessor
{
public:
	virtual ~PuresoftFragmentProcessor() {}
	virtual void preprocess(const void** uniforms, const void** textures) = 0;
	virtual void process(const FragmentProcessorInput* input, FragmentProcessorOutput* output) const = 0;
};