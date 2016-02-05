#pragma once
#include "proc.h"
#include "testpd.h"

class MyTestInterpolationProcessor :
	public PuresoftInterpolationProcessor
{
public:
	static void* _stdcall createInstance(void);

public:
	MyTestInterpolationProcessor(void);
	~MyTestInterpolationProcessor(void);

	void release(void);
	void interpolateByContributes(void* interpolatedUserData, const void** vertexUserData, const float* correctedContributes);
	void calcStep(void* interpolatedUserDataStep, const void* interpolatedUserDataStart, const void* interpolatedUserDataEnd, int stepCount);
	void interpolateBySteps(void* interpolatedUserData, void* interpolatedUserDataStart, const void* interpolatedUserDataStep, float correctionFactor2);
};

