#include <atlbase.h>
#include <memory.h>
#include "mcemaths.h"
#include "testip.h"

void* _stdcall MyTestInterpolationProcessor::createInstance(void)
{
	return new MyTestInterpolationProcessor;
}

MyTestInterpolationProcessor::MyTestInterpolationProcessor(void)
{
}

MyTestInterpolationProcessor::~MyTestInterpolationProcessor(void)
{
}

void MyTestInterpolationProcessor::release()
{
	delete this;
}

void MyTestInterpolationProcessor::interpolateByContributes(void* interpolatedUserData, const void** vertexUserData, const float* correctedContributes)
{
	MYTESTPROCDATA temp[3];
	memcpy(temp,     vertexUserData[0], sizeof(MYTESTPROCDATA));
	memcpy(temp + 1, vertexUserData[1], sizeof(MYTESTPROCDATA));
	memcpy(temp + 2, vertexUserData[2], sizeof(MYTESTPROCDATA));

	mcemaths_mul_3_4(temp[0].normal, correctedContributes[0]);
	mcemaths_mul_3_4(temp[0].worldPos, correctedContributes[0]);
	mcemaths_mul_3_4(temp[0].texcoord, correctedContributes[0]);

	mcemaths_mul_3_4(temp[1].normal, correctedContributes[1]);
	mcemaths_mul_3_4(temp[1].worldPos, correctedContributes[1]);
	mcemaths_mul_3_4(temp[1].texcoord, correctedContributes[1]);

	mcemaths_mul_3_4(temp[2].normal, correctedContributes[2]);
	mcemaths_mul_3_4(temp[2].worldPos, correctedContributes[2]);
	mcemaths_mul_3_4(temp[2].texcoord, correctedContributes[2]);

	MYTESTPROCDATA* output = (MYTESTPROCDATA*)interpolatedUserData;

	mcemaths_quatcpy(output->normal, temp[0].normal);
	mcemaths_quatcpy(output->worldPos, temp[0].worldPos);
	mcemaths_quatcpy(output->texcoord, temp[0].texcoord);

	mcemaths_add_3_4_ip(output->normal, temp[1].normal);
	mcemaths_add_3_4_ip(output->worldPos, temp[1].worldPos);
	mcemaths_add_3_4_ip(output->texcoord, temp[1].texcoord);

	mcemaths_add_3_4_ip(output->normal, temp[2].normal);
	mcemaths_add_3_4_ip(output->worldPos, temp[2].worldPos);
	mcemaths_add_3_4_ip(output->texcoord, temp[2].texcoord);
}

void MyTestInterpolationProcessor::calcStep(void* interpolatedUserDataStep, const void* interpolatedUserDataStart, const void* interpolatedUserDataEnd, int stepCount)
{
	MYTESTPROCDATA* start = (MYTESTPROCDATA*)interpolatedUserDataStart;
	MYTESTPROCDATA* end = (MYTESTPROCDATA*)interpolatedUserDataEnd;
	MYTESTPROCDATA* step = (MYTESTPROCDATA*)interpolatedUserDataStep;

	//ATLTRACE("start.tex=(%f, %f), end.tex=(%f, %f), stepCnt=%ld, ", start->texcoord[0], start->texcoord[1], end->texcoord[0], end->texcoord[1], stepCount);

	float reciprocalStepCount = 0 == stepCount ? 1.0f : 1.0f / (float)stepCount;
	mcemaths_sub_3_4(step->normal, end->normal, start->normal);
	mcemaths_sub_3_4(step->worldPos, end->worldPos, start->worldPos);
	mcemaths_sub_3_4(step->texcoord, end->texcoord, start->texcoord);
	mcemaths_mul_3_4(step->normal, reciprocalStepCount);
	mcemaths_mul_3_4(step->worldPos, reciprocalStepCount);
	mcemaths_mul_3_4(step->texcoord, reciprocalStepCount);

	//ATLTRACE("step.tex=(%f, %f)\n", step->texcoord[0], step->texcoord[1]);
}

void MyTestInterpolationProcessor::interpolateBySteps(void* interpolatedUserData, void* interpolatedUserDataStart, const void* interpolatedUserDataStep, float correctionFactor2)
{
	MYTESTPROCDATA* output = (MYTESTPROCDATA*)interpolatedUserData;
	MYTESTPROCDATA* start = (MYTESTPROCDATA*)interpolatedUserDataStart;
	memcpy(output, start, sizeof(MYTESTPROCDATA));
	mcemaths_mul_3_4(output->normal, correctionFactor2);
	mcemaths_mul_3_4(output->worldPos, correctionFactor2);
	mcemaths_mul_3_4(output->texcoord, correctionFactor2);

	const MYTESTPROCDATA* step = (MYTESTPROCDATA*)interpolatedUserDataStep;

	//ATLTRACE("(%f, %f; %f, %f; ", start->texcoord[0], start->texcoord[1], step->texcoord[0], step->texcoord[1]);

	mcemaths_add_3_4_ip(start->normal, step->normal);
	mcemaths_add_3_4_ip(start->worldPos, step->worldPos);
	mcemaths_add_3_4_ip(start->texcoord, step->texcoord);

	//ATLTRACE("%f, %f), ", start->texcoord[0], start->texcoord[1]);
}