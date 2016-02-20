#pragma once
#include "proc.h"

#pragma pack(16)
__declspec(align(16)) struct PROCDATA_DEF01
{
	float normal[4];
	float worldPos[4];
	float texcoord[4];
};
#pragma pack()

class VertexProcesserDEF01 : public PuresoftVertexProcessor
{
public:
	static void* _stdcall createInstance(void);

public:
	VertexProcesserDEF01(void);
	~VertexProcesserDEF01(void);

	void release(void);
	void process(const VertexProcessorInput* input, VertexProcessorOutput* output, const void** uniforms);
};

class InterpolationProcessorDEF01 : public PuresoftInterpolationProcessor
{
public:
	static void* _stdcall createInstance(void);

public:
	InterpolationProcessorDEF01(void);
	~InterpolationProcessorDEF01(void);

	void release(void);
	void interpolateByContributes(void* interpolatedUserData, const void** vertexUserData, const float* correctedContributes);
	void calcStep(void* interpolatedUserDataStep, const void* interpolatedUserDataStart, const void* interpolatedUserDataEnd, int stepCount);
	void interpolateBySteps(void* interpolatedUserData, void* interpolatedUserDataStart, const void* interpolatedUserDataStep, float correctionFactor2);
};

class FragmentProcessorDEF01 : public PuresoftFragmentProcessor
{
public:
	static void* _stdcall createInstance(void);

public:
	FragmentProcessorDEF01(void);
	~FragmentProcessorDEF01(void);

	void release(void);
	void process(const FragmentProcessorInput* input, FragmentProcessorOutput* output, const void** uniforms, const void** textures);
};