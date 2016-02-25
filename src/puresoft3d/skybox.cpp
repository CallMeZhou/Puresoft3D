#include <memory.h>
#include "mcemaths.h"
#include "defproc.h"
#include "samplrcube.h"
#include "defs.h"

const ALIGN16 float observer[4] = {0, 0, 1.0f, 0};

VertexProcesserDEF04::VertexProcesserDEF04(void)
{
}

VertexProcesserDEF04::~VertexProcesserDEF04(void)
{
}

void VertexProcesserDEF04::preprocess(const void** uniforms)
{
	m_P = (const float*)uniforms[0];
	m_V = (const float*)uniforms[1];
}

void VertexProcesserDEF04::process(const VertexProcessorInput* input, VertexProcessorOutput* output) const
{
	PROCDATA_DEF04* userOutput = (PROCDATA_DEF04*)output->user;

	const float* position = (const float*)input->data[0];

	ALIGN16 float vectorFromObserverToNearPlane[4];
	mcemaths_sub_3_4(vectorFromObserverToNearPlane, position, observer);
	mcemaths_norm_3_4(vectorFromObserverToNearPlane);

	ALIGN16 float inversedView[16];
	mcemaths_mat4cpy(inversedView, m_V);
	mcemaths_mat4transpose(inversedView);
	mcemaths_zero_vec_ary(inversedView + 12, 1);

	mcemaths_transform_m4v4(userOutput->direction, inversedView, vectorFromObserverToNearPlane);

	vectorFromObserverToNearPlane[3] = 1.0f;
	mcemaths_transform_m4v4(output->position, m_P, vectorFromObserverToNearPlane);
}

InterpolationProcessorDEF04::InterpolationProcessorDEF04(void)
{
}

InterpolationProcessorDEF04::~InterpolationProcessorDEF04(void)
{
}

size_t InterpolationProcessorDEF04::userDataBytes(void) const
{
	return sizeof(PROCDATA_DEF04);
}

void InterpolationProcessorDEF04::preprocess(const void** uniforms)
{
}

void InterpolationProcessorDEF04::interpolateByContributes(void* interpolatedUserData, const void** vertexUserData, const float* correctedContributes) const
{
	PROCDATA_DEF04 temp[3];
	memcpy(temp,     vertexUserData[0], sizeof(PROCDATA_DEF04));
	memcpy(temp + 1, vertexUserData[1], sizeof(PROCDATA_DEF04));
	memcpy(temp + 2, vertexUserData[2], sizeof(PROCDATA_DEF04));

	mcemaths_mul_3_4(temp[0].direction, correctedContributes[0]);
	mcemaths_mul_3_4(temp[1].direction, correctedContributes[1]);
	mcemaths_mul_3_4(temp[2].direction, correctedContributes[2]);

	PROCDATA_DEF04* output = (PROCDATA_DEF04*)interpolatedUserData;

	mcemaths_quatcpy(output->direction, temp[0].direction);
	mcemaths_add_3_4_ip(output->direction, temp[1].direction);
	mcemaths_add_3_4_ip(output->direction, temp[2].direction);
}

void InterpolationProcessorDEF04::calcStep(void* interpolatedUserDataStep, const void* interpolatedUserDataStart, const void* interpolatedUserDataEnd, int stepCount) const
{
	PROCDATA_DEF04* start = (PROCDATA_DEF04*)interpolatedUserDataStart;
	PROCDATA_DEF04* end = (PROCDATA_DEF04*)interpolatedUserDataEnd;
	PROCDATA_DEF04* step = (PROCDATA_DEF04*)interpolatedUserDataStep;

	float reciprocalStepCount = 0 == stepCount ? 1.0f : 1.0f / (float)stepCount;
	mcemaths_sub_3_4(step->direction, end->direction, start->direction);
	mcemaths_mul_3_4(step->direction, reciprocalStepCount);
}

void InterpolationProcessorDEF04::interpolateBySteps(void* interpolatedUserData, void* interpolatedUserDataStart, const void* interpolatedUserDataStep, float correctionFactor2) const
{
	PROCDATA_DEF04* output = (PROCDATA_DEF04*)interpolatedUserData;
	PROCDATA_DEF04* start = (PROCDATA_DEF04*)interpolatedUserDataStart;
	memcpy(output, start, sizeof(PROCDATA_DEF04));
	mcemaths_mul_3_4(output->direction, correctionFactor2);

	const PROCDATA_DEF04* step = (PROCDATA_DEF04*)interpolatedUserDataStep;

	mcemaths_add_3_4_ip(start->direction, step->direction);
}

FragmentProcessorDEF04::FragmentProcessorDEF04(void)
{
}

FragmentProcessorDEF04::~FragmentProcessorDEF04(void)
{
}

void FragmentProcessorDEF04::preprocess(const void** uniforms, const void** textures)
{
	m_skyboxTex = (const PuresoftFBO*)uniforms[2];
}

void FragmentProcessorDEF04::process(const FragmentProcessorInput* input, FragmentProcessorOutput* output) const
{
	PURESOFTBGRA bytesColour;

	const PROCDATA_DEF04* inData = (const PROCDATA_DEF04*)input->user;
	PuresoftSamplerCube::get4(m_skyboxTex, inData->direction, &bytesColour);
	output->write(0, &bytesColour, sizeof(bytesColour));
}