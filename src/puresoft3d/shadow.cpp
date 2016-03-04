#include <memory.h>
#include "mcemaths.h"
#include "defproc.h"
#include "samplr2d.h"
#include "defs.h"

VertexProcesserDEF05::VertexProcesserDEF05(void)
{
}

VertexProcesserDEF05::~VertexProcesserDEF05(void)
{
}

void VertexProcesserDEF05::preprocess(const PURESOFTUNIFORM* uniforms)
{
	m_PV = (const float*)uniforms[3].data;
	m_M = (const float*)uniforms[4].data;
}

void VertexProcesserDEF05::process(const VertexProcessorInput* input, VertexProcessorOutput* output) const
{
	const float* position = (const float*)input->data[0];
	ALIGN16 float pvm[16];
	mcemaths_transform_m4m4(pvm, m_PV, m_M);
	mcemaths_transform_m4v4(output->position, pvm, position);
}

InterpolationProcessorDEF05::InterpolationProcessorDEF05(void)
{
}

InterpolationProcessorDEF05::~InterpolationProcessorDEF05(void)
{
}

size_t InterpolationProcessorDEF05::userDataBytes(void) const
{
	return sizeof(PROCDATA_DEF05);
}

void InterpolationProcessorDEF05::preprocess(const PURESOFTUNIFORM* uniforms)
{
}

void InterpolationProcessorDEF05::interpolateByContributes(void* interpolatedUserData, const void** vertexUserData, const float* correctedContributes) const
{
}

void InterpolationProcessorDEF05::calcStep(void* interpolatedUserDataStep, const void* interpolatedUserDataStart, const void* interpolatedUserDataEnd, int stepCount) const
{
}

void InterpolationProcessorDEF05::interpolateBySteps(void* interpolatedUserData, void* interpolatedUserDataStart, const void* interpolatedUserDataStep, float correctionFactor2) const
{
}

FragmentProcessorDEF05::FragmentProcessorDEF05(void)
{
}

FragmentProcessorDEF05::~FragmentProcessorDEF05(void)
{
}

void FragmentProcessorDEF05::preprocess(const PURESOFTUNIFORM* uniforms, const void** textures)
{
}

void FragmentProcessorDEF05::process(const FragmentProcessorInput* input, FragmentProcessorOutput* output) const
{
}