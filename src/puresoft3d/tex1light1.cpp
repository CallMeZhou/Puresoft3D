#include <memory.h>
#include "mcemaths.h"
#include "defproc.h"
#include "samplr2d.h"
#include "defs.h"

VertexProcesserDEF01::VertexProcesserDEF01(void)
{
}

VertexProcesserDEF01::~VertexProcesserDEF01(void)
{
}

void VertexProcesserDEF01::preprocess(const void** uniforms)
{
	m_PV = (const float*)uniforms[0];
	m_M = (const float*)uniforms[1];
	m_Mrot = (const float*)uniforms[2];
}

void VertexProcesserDEF01::process(const VertexProcessorInput* input, VertexProcessorOutput* output) const
{
	PROCDATA_DEF01* userOutput = (PROCDATA_DEF01*)output->user;

	const float* position = (const float*)input->data[0];
	const float* normals = (const float*)input->data[1];
	const float* texcoord = (const float*)input->data[2];

	mcemaths_transform_m4v4(userOutput->worldPos, m_M, position);

	mcemaths_quatcpy(output->position, userOutput->worldPos);
	userOutput->worldPos[3] = 0;

	mcemaths_transform_m4v4_ip(output->position, m_PV);
	//output->position[3] = 1.0f;

	mcemaths_transform_m4v4(userOutput->normal, m_Mrot, normals);

	userOutput->texcoord[0] = texcoord[0];
	userOutput->texcoord[1] = texcoord[1];
}

InterpolationProcessorDEF01::InterpolationProcessorDEF01(void)
{
}

InterpolationProcessorDEF01::~InterpolationProcessorDEF01(void)
{
}

size_t InterpolationProcessorDEF01::userDataBytes(void) const
{
	return sizeof(PROCDATA_DEF01);
}

void InterpolationProcessorDEF01::preprocess(const void** uniforms)
{
}

void InterpolationProcessorDEF01::interpolateByContributes(void* interpolatedUserData, const void** vertexUserData, const float* correctedContributes) const
{
	PROCDATA_DEF01 temp[3];
	memcpy(temp,     vertexUserData[0], sizeof(PROCDATA_DEF01));
	memcpy(temp + 1, vertexUserData[1], sizeof(PROCDATA_DEF01));
	memcpy(temp + 2, vertexUserData[2], sizeof(PROCDATA_DEF01));

	mcemaths_mul_3_4(temp[0].normal, correctedContributes[0]);
	mcemaths_mul_3_4(temp[0].worldPos, correctedContributes[0]);
	mcemaths_mul_3_4(temp[0].texcoord, correctedContributes[0]);

	mcemaths_mul_3_4(temp[1].normal, correctedContributes[1]);
	mcemaths_mul_3_4(temp[1].worldPos, correctedContributes[1]);
	mcemaths_mul_3_4(temp[1].texcoord, correctedContributes[1]);

	mcemaths_mul_3_4(temp[2].normal, correctedContributes[2]);
	mcemaths_mul_3_4(temp[2].worldPos, correctedContributes[2]);
	mcemaths_mul_3_4(temp[2].texcoord, correctedContributes[2]);

	PROCDATA_DEF01* output = (PROCDATA_DEF01*)interpolatedUserData;

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

void InterpolationProcessorDEF01::calcStep(void* interpolatedUserDataStep, const void* interpolatedUserDataStart, const void* interpolatedUserDataEnd, int stepCount) const
{
	PROCDATA_DEF01* start = (PROCDATA_DEF01*)interpolatedUserDataStart;
	PROCDATA_DEF01* end = (PROCDATA_DEF01*)interpolatedUserDataEnd;
	PROCDATA_DEF01* step = (PROCDATA_DEF01*)interpolatedUserDataStep;

	float reciprocalStepCount = 0 == stepCount ? 1.0f : 1.0f / (float)stepCount;
	mcemaths_sub_3_4(step->normal, end->normal, start->normal);
	mcemaths_sub_3_4(step->worldPos, end->worldPos, start->worldPos);
	mcemaths_sub_3_4(step->texcoord, end->texcoord, start->texcoord);
	mcemaths_mul_3_4(step->normal, reciprocalStepCount);
	mcemaths_mul_3_4(step->worldPos, reciprocalStepCount);
	mcemaths_mul_3_4(step->texcoord, reciprocalStepCount);
}

void InterpolationProcessorDEF01::interpolateBySteps(void* interpolatedUserData, void* interpolatedUserDataStart, const void* interpolatedUserDataStep, float correctionFactor2) const
{
	PROCDATA_DEF01* output = (PROCDATA_DEF01*)interpolatedUserData;
	PROCDATA_DEF01* start = (PROCDATA_DEF01*)interpolatedUserDataStart;
	memcpy(output, start, sizeof(PROCDATA_DEF01));
	mcemaths_mul_3_4(output->normal, correctionFactor2);
	mcemaths_mul_3_4(output->worldPos, correctionFactor2);
	mcemaths_mul_3_4(output->texcoord, correctionFactor2);

	const PROCDATA_DEF01* step = (PROCDATA_DEF01*)interpolatedUserDataStep;

	mcemaths_add_3_4_ip(start->normal, step->normal);
	mcemaths_add_3_4_ip(start->worldPos, step->worldPos);
	mcemaths_add_3_4_ip(start->texcoord, step->texcoord);
}

FragmentProcessorDEF01::FragmentProcessorDEF01(void)
{
}

FragmentProcessorDEF01::~FragmentProcessorDEF01(void)
{
}

void FragmentProcessorDEF01::preprocess(const void** uniforms, const void** textures)
{
	m_lightPos = (const float*)uniforms[4];
	m_cameraPos = (const float*)uniforms[5];
	m_diffuseTex = (const PuresoftFBO*)textures[*(const int*)uniforms[6]];
}

void FragmentProcessorDEF01::process(const FragmentProcessorInput* input, FragmentProcessorOutput* output) const
{
	ALIGN16 float outputColour[4];
	PURESOFTBGRA bytesColour;

	const PROCDATA_DEF01* inData = (const PROCDATA_DEF01*)input->user;
	PuresoftSampler2D::get4(m_diffuseTex, inData->texcoord[0], inData->texcoord[1], &bytesColour);
	outputColour[0] = bytesColour.elems.b;
	outputColour[1] = bytesColour.elems.g;
	outputColour[2] = bytesColour.elems.r;
	outputColour[3] = bytesColour.elems.a;

	ALIGN16 float L[4];
	mcemaths_sub_3_4(L, m_lightPos, inData->worldPos);
	float distance = mcemaths_len_3_4(L);
	mcemaths_div_3_4(L, distance);

	ALIGN16 float E[4];
	mcemaths_sub_3_4(E, m_cameraPos, inData->worldPos);
	mcemaths_norm_3_4(E);
	ALIGN16 float H[4];
	mcemaths_add_3_4(H, E, L);
	mcemaths_norm_3_4(H);

	float lambert = mcemaths_dot_3_4(L, inData->normal);

	float specular = mcemaths_dot_3_4(H, inData->normal);
	specular = specular < 0 ? 0 : specular;
	//specular = pow(specular, 50.0f);
	specular = opt_pow(specular, 50);


	mcemaths_add_1to4(outputColour, 255.0f * specular);
	mcemaths_mul_3_4(outputColour, lambert);
	mcemaths_clamp_3_4(outputColour, 0, 255.0f);
	bytesColour.elems.r = (unsigned char)outputColour[2];
	bytesColour.elems.g = (unsigned char)outputColour[1];
	bytesColour.elems.b = (unsigned char)outputColour[0];
	bytesColour.elems.a = 0;

	output->write(0, &bytesColour, sizeof(bytesColour));
}