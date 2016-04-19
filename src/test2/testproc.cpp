#include <memory.h>
#include <math.h>
#include "defs.h"
#include "mcemaths.h"
#include "samplr2d.h"
#include "samplrproj.h"
#include "testproc.h"

static const float fieldOfLight = 6.283185f * (25.0f / 360.0f);

//////////////////////////////////////////////////////////////////////////
// VP_PositionOnly
void VP_PositionOnly::preprocess(const PURESOFTUNIFORM* uniforms)
{
	m_PVM = (const float*)uniforms[5].data;
}

void VP_PositionOnly::process(const VertexProcessorInput* input, VertexProcessorOutput* output) const
{
	const float* position = (const float*)input->data[0];
	// project world-position into view space
	mcemaths_transform_m4v4(output->position, m_PVM, position);
}

//////////////////////////////////////////////////////////////////////////
// IP_Null
size_t IP_Null::userDataBytes(void) const
{
	return 16;
}

void IP_Null::preprocess(const PURESOFTUNIFORM* uniforms)
{}

void IP_Null::interpolateByContributes(void* interpolatedUserData, const void** vertexUserData, const float* correctedContributes) const
{}

void IP_Null::calcStep(void* interpolatedUserDataStep, const void* interpolatedUserDataStart, const void* interpolatedUserDataEnd, int stepCount) const
{}

void IP_Null::correctInterpolation(void* interpolatedUserData, const void* interpolatedUserDataStart, float correctionFactor2) const
{}

void IP_Null::stepForward(void* interpolatedUserDataStart, const void* interpolatedUserDataStep, int stepCount) const
{}

//////////////////////////////////////////////////////////////////////////
// FP_SingleColourNoLighting
void FP_SingleColourNoLighting::preprocess(const PURESOFTUNIFORM* uniforms, const void** textures)
{
	m_diffuse = (const float*)uniforms[31].data;
}

void FP_SingleColourNoLighting::process(const FragmentProcessorInput* input, FragmentProcessorOutput* output) const
{
	const PROCDATA_SINGLECOLOUR* inData = (const PROCDATA_SINGLECOLOUR*)input->user;

	ALIGN16 float outputColour[4];
	mcemaths_quatcpy(outputColour, m_diffuse);
	mcemaths_mul_3_4(outputColour, 255.0f);

	PURESOFTBGRA bytesColour;
	bytesColour.elems.r = (unsigned char)outputColour[2];
	bytesColour.elems.g = (unsigned char)outputColour[1];
	bytesColour.elems.b = (unsigned char)outputColour[0];
	bytesColour.elems.a = 0;

	output->write4(0, &bytesColour);
}

//////////////////////////////////////////////////////////////////////////
// VP_SingleColour
static void copyUserDataSingleColour(PROCDATA_SINGLECOLOUR* dest, const PROCDATA_SINGLECOLOUR* src)
{
	__asm
	{
		mov		eax,		src
		movaps	xmm0,		[eax]
		movaps	xmm1,		[eax + 16]
		movaps	xmm2,		[eax + 32]
		mov		eax,		dest
		movaps	[eax],		xmm0
		movaps	[eax + 16],	xmm1
		movaps	[eax + 32],	xmm2
	}
}

void VP_SingleColour::preprocess(const PURESOFTUNIFORM* uniforms)
{
	m_M = (const float*)uniforms[0].data;
	m_MR = (const float*)uniforms[1].data;
	m_PVM = (const float*)uniforms[5].data;
	m_shadowPV = (const float*)uniforms[6].data;
}

void VP_SingleColour::process(const VertexProcessorInput* input, VertexProcessorOutput* output) const
{
	PROCDATA_SINGLECOLOUR* userOutput = (PROCDATA_SINGLECOLOUR*)output->user;

	const float* position = (const float*)input->data[0];
	const float* normal = (const float*)input->data[3];
	const float* texcoord = (const float*)input->data[4];

	// get vertex position in world space
	mcemaths_transform_m4v4(userOutput->worldPos, m_M, position);

	// project world-position into shadow map's UV space, inp:userOutput->worldPos, outp:userOutput->shadowcoord
	mcemaths_quatcpy(userOutput->shadowcoord, userOutput->worldPos);
	mcemaths_transform_m4v4_ip(userOutput->shadowcoord, m_shadowPV);

	// remove world-position's w (later we'll use it as an xyz coordinate)
	userOutput->worldPos[3] = 0;

	// project model-position into view space
	mcemaths_transform_m4v4(output->position, m_PVM, position);

	// transform normal
	mcemaths_transform_m4v4(userOutput->normal, m_MR, normal);
}

//////////////////////////////////////////////////////////////////////////
// IP_SingleColour
size_t IP_SingleColour::userDataBytes(void) const
{
	return sizeof(PROCDATA_SINGLECOLOUR);
}

void IP_SingleColour::preprocess(const PURESOFTUNIFORM* uniforms)
{
}

void IP_SingleColour::interpolateByContributes(void* interpolatedUserData, const void** vertexUserData, const float* correctedContributes) const
{
	__declspec(align(16)) PROCDATA_SINGLECOLOUR temp[3];
	copyUserDataSingleColour(temp,     (PROCDATA_SINGLECOLOUR*)vertexUserData[0]);
	copyUserDataSingleColour(temp + 1, (PROCDATA_SINGLECOLOUR*)vertexUserData[1]);
	copyUserDataSingleColour(temp + 2, (PROCDATA_SINGLECOLOUR*)vertexUserData[2]);

	mcemaths_mul_3_4(temp[0].normal, correctedContributes[0]);
	mcemaths_mul_3_4(temp[0].worldPos, correctedContributes[0]);
	mcemaths_mul_3_4(temp[0].shadowcoord, correctedContributes[0]);

	mcemaths_mul_3_4(temp[1].normal, correctedContributes[1]);
	mcemaths_mul_3_4(temp[1].worldPos, correctedContributes[1]);
	mcemaths_mul_3_4(temp[1].shadowcoord, correctedContributes[1]);

	mcemaths_mul_3_4(temp[2].normal, correctedContributes[2]);
	mcemaths_mul_3_4(temp[2].worldPos, correctedContributes[2]);
	mcemaths_mul_3_4(temp[2].shadowcoord, correctedContributes[2]);

	PROCDATA_SINGLECOLOUR* output = (PROCDATA_SINGLECOLOUR*)interpolatedUserData;

	mcemaths_quatcpy(output->normal, temp[0].normal);
	mcemaths_quatcpy(output->worldPos, temp[0].worldPos);
	mcemaths_quatcpy(output->shadowcoord, temp[0].shadowcoord);

	mcemaths_add_3_4_ip(output->normal, temp[1].normal);
	mcemaths_add_3_4_ip(output->worldPos, temp[1].worldPos);
	mcemaths_add_3_4_ip(output->shadowcoord, temp[1].shadowcoord);

	mcemaths_add_3_4_ip(output->normal, temp[2].normal);
	mcemaths_add_3_4_ip(output->worldPos, temp[2].worldPos);
	mcemaths_add_3_4_ip(output->shadowcoord, temp[2].shadowcoord);
}

void IP_SingleColour::calcStep(void* interpolatedUserDataStep, const void* interpolatedUserDataStart, const void* interpolatedUserDataEnd, int stepCount) const
{
	PROCDATA_SINGLECOLOUR* start = (PROCDATA_SINGLECOLOUR*)interpolatedUserDataStart;
	PROCDATA_SINGLECOLOUR* end = (PROCDATA_SINGLECOLOUR*)interpolatedUserDataEnd;
	PROCDATA_SINGLECOLOUR* step = (PROCDATA_SINGLECOLOUR*)interpolatedUserDataStep;

	float reciprocalStepCount = 0 == stepCount ? 1.0f : 1.0f / (float)stepCount;
	mcemaths_sub_3_4(step->normal, end->normal, start->normal);
	mcemaths_sub_3_4(step->worldPos, end->worldPos, start->worldPos);
	mcemaths_sub_3_4(step->shadowcoord, end->shadowcoord, start->shadowcoord);

	mcemaths_mul_3_4(step->normal, reciprocalStepCount);
	mcemaths_mul_3_4(step->worldPos, reciprocalStepCount);
	mcemaths_mul_3_4(step->shadowcoord, reciprocalStepCount);
}

void IP_SingleColour::correctInterpolation(void* interpolatedUserData, const void* interpolatedUserDataStart, float correctionFactor2) const
{
	PROCDATA_SINGLECOLOUR* output = (PROCDATA_SINGLECOLOUR*)interpolatedUserData;
	const PROCDATA_SINGLECOLOUR* start = (const PROCDATA_SINGLECOLOUR*)interpolatedUserDataStart;
	copyUserDataSingleColour(output, start);
	mcemaths_mul_3_4(output->normal, correctionFactor2);
	mcemaths_mul_3_4(output->worldPos, correctionFactor2);
	mcemaths_mul_3_4(output->shadowcoord, correctionFactor2);
}

void IP_SingleColour::stepForward(void* interpolatedUserDataStart, const void* interpolatedUserDataStep, int stepCount) const
{
	PROCDATA_SINGLECOLOUR* start = (PROCDATA_SINGLECOLOUR*)interpolatedUserDataStart;
	const PROCDATA_SINGLECOLOUR* step = (PROCDATA_SINGLECOLOUR*)interpolatedUserDataStep;

	if(1 == stepCount)
	{
		mcemaths_add_3_4_ip(start->normal, step->normal);
		mcemaths_add_3_4_ip(start->worldPos, step->worldPos);
		mcemaths_add_3_4_ip(start->shadowcoord, step->shadowcoord);
	}
	else
	{
		mcemaths_step_3_4_ip(start->normal, step->normal, (float)stepCount);
		mcemaths_step_3_4_ip(start->worldPos, step->worldPos, (float)stepCount);
		mcemaths_step_3_4_ip(start->shadowcoord, step->shadowcoord, (float)stepCount);
	}
}

//////////////////////////////////////////////////////////////////////////
// FP_SingleColour
void FP_SingleColour::preprocess(const PURESOFTUNIFORM* uniforms, const void** textures)
{
	m_lightPos = (const float*)uniforms[20].data;
	m_lightDir = (const float*)uniforms[21].data;
	m_cameraPos = (const float*)uniforms[22].data;
	m_ambient = (const float*)uniforms[30].data;
	m_diffuse = (const float*)uniforms[31].data;
	m_specularColour = (const float*)uniforms[32].data;
	m_specularExponent = (const float*)uniforms[33].data;
	m_shadowTex = (const PuresoftFBO*)textures[*(const int*)uniforms[23].data];
}

void FP_SingleColour::process(const FragmentProcessorInput* input, FragmentProcessorOutput* output) const
{
	const PROCDATA_SINGLECOLOUR* inData = (const PROCDATA_SINGLECOLOUR*)input->user;

	float shadowFactor = PuresoftSamplerProjection::get(m_shadowTex, inData->shadowcoord);

	ALIGN16 float L[4];
	mcemaths_sub_3_4(L, m_lightPos, inData->worldPos);
	mcemaths_norm_3_4(L);

	ALIGN16 float E[4];
	mcemaths_sub_3_4(E, m_cameraPos, inData->worldPos);
	mcemaths_norm_3_4(E);

	ALIGN16 float H[4];
	mcemaths_add_3_4(H, E, L);
	mcemaths_norm_3_4(H);


	enum {LAMBERT, SPECULAR};
	ALIGN16 float factors[4];

	factors[LAMBERT] = mcemaths_dot_3_4(L, inData->normal);

	float yawOfLight = (float)acos(mcemaths_dot_3_4(L, m_lightDir));
	factors[LAMBERT] *= yawOfLight < fieldOfLight ? 1.0f : (float)opt_pow(cos(yawOfLight - fieldOfLight), 150);

	factors[SPECULAR] = opt_pow(mcemaths_dot_3_4(H, inData->normal), (unsigned int)*m_specularExponent);

	mcemaths_clamp_3_4(factors, 0, 1.0f);

	ALIGN16 float outputColour[4];
	mcemaths_quatcpy(outputColour, m_diffuse);
	mcemaths_mul_3_4(outputColour, factors[LAMBERT]);

	ALIGN16 float specularColour[4];
	mcemaths_quatcpy(specularColour, m_diffuse);
	mcemaths_mulvec_3_4(specularColour, m_specularColour);
	mcemaths_mul_3_4(specularColour, factors[SPECULAR]);

	ALIGN16 float ambientColour[4];
	mcemaths_quatcpy(ambientColour, m_diffuse);
	mcemaths_mulvec_3_4(ambientColour, m_ambient);

	mcemaths_add_3_4_ip(outputColour, specularColour);
	mcemaths_mul_3_4(outputColour, shadowFactor);
	mcemaths_add_3_4_ip(outputColour, ambientColour);
	mcemaths_clamp_3_4(outputColour, 0, 1.0f);

	mcemaths_mul_3_4(outputColour, 255.0f);

	PURESOFTBGRA bytesColour;
	bytesColour.elems.r = (unsigned char)outputColour[2];
	bytesColour.elems.g = (unsigned char)outputColour[1];
	bytesColour.elems.b = (unsigned char)outputColour[0];
	bytesColour.elems.a = 0;

	output->write4(0, &bytesColour);
}

//////////////////////////////////////////////////////////////////////////
// VP_DiffuseOnly
static void copyUserDataDiffuseOnly(PROCDATA_DIFFUSEONLY* dest, const PROCDATA_DIFFUSEONLY* src)
{
	__asm
	{
		mov		eax,		src
		movaps	xmm0,		[eax]
		movaps	xmm1,		[eax + 16]
		movaps	xmm2,		[eax + 32]
		movaps	xmm3,		[eax + 48]
		mov		eax,		dest
		movaps	[eax],		xmm0
		movaps	[eax + 16],	xmm1
		movaps	[eax + 32],	xmm2
		movaps	[eax + 48],	xmm3
	}
}

void VP_DiffuseOnly::preprocess(const PURESOFTUNIFORM* uniforms)
{
	m_M = (const float*)uniforms[0].data;
	m_MR = (const float*)uniforms[1].data;
	m_PV = (const float*)uniforms[4].data;
	m_PVM = (const float*)uniforms[5].data;
	m_shadowPV = (const float*)uniforms[6].data;
}

void VP_DiffuseOnly::process(const VertexProcessorInput* input, VertexProcessorOutput* output) const
{
	PROCDATA_DIFFUSEONLY* userOutput = (PROCDATA_DIFFUSEONLY*)output->user;

	const float* position = (const float*)input->data[0];
	const float* normal = (const float*)input->data[3];
	const float* texcoord = (const float*)input->data[4];

	// get vertex position in world space
	mcemaths_transform_m4v4(userOutput->worldPos, m_M, position);

	// project world-position into shadow map's UV space, inp:userOutput->worldPos, outp:userOutput->shadowcoord
	mcemaths_quatcpy(userOutput->shadowcoord, userOutput->worldPos);
	mcemaths_transform_m4v4_ip(userOutput->shadowcoord, m_shadowPV);

	// project world-position into view space
	mcemaths_transform_m4v4(output->position, m_PVM, position);

	// remove world-position's w (later we'll use it as an xyz coordinate)
	userOutput->worldPos[3] = 0;

	// transform normals
	mcemaths_transform_m4v4(userOutput->normal, m_MR, normal);

	// pass on texture coord
	userOutput->texcoord[0] = texcoord[0];
	userOutput->texcoord[1] = texcoord[1];
}

//////////////////////////////////////////////////////////////////////////
// IP_DiffuseOnly
size_t IP_DiffuseOnly::userDataBytes(void) const
{
	return sizeof(PROCDATA_DIFFUSEONLY);
}

void IP_DiffuseOnly::preprocess(const PURESOFTUNIFORM* uniforms)
{
}

void IP_DiffuseOnly::interpolateByContributes(void* interpolatedUserData, const void** vertexUserData, const float* correctedContributes) const
{
	__declspec(align(16)) PROCDATA_DIFFUSEONLY temp[3];
	copyUserDataDiffuseOnly(temp,     (PROCDATA_DIFFUSEONLY*)vertexUserData[0]);
	copyUserDataDiffuseOnly(temp + 1, (PROCDATA_DIFFUSEONLY*)vertexUserData[1]);
	copyUserDataDiffuseOnly(temp + 2, (PROCDATA_DIFFUSEONLY*)vertexUserData[2]);

	mcemaths_mul_3_4(temp[0].normal, correctedContributes[0]);
	mcemaths_mul_3_4(temp[0].worldPos, correctedContributes[0]);
	mcemaths_mul_3_4(temp[0].texcoord, correctedContributes[0]);
	mcemaths_mul_3_4(temp[0].shadowcoord, correctedContributes[0]);

	mcemaths_mul_3_4(temp[1].normal, correctedContributes[1]);
	mcemaths_mul_3_4(temp[1].worldPos, correctedContributes[1]);
	mcemaths_mul_3_4(temp[1].texcoord, correctedContributes[1]);
	mcemaths_mul_3_4(temp[1].shadowcoord, correctedContributes[1]);

	mcemaths_mul_3_4(temp[2].normal, correctedContributes[2]);
	mcemaths_mul_3_4(temp[2].worldPos, correctedContributes[2]);
	mcemaths_mul_3_4(temp[2].texcoord, correctedContributes[2]);
	mcemaths_mul_3_4(temp[2].shadowcoord, correctedContributes[2]);

	PROCDATA_DIFFUSEONLY* output = (PROCDATA_DIFFUSEONLY*)interpolatedUserData;

	mcemaths_quatcpy(output->normal, temp[0].normal);
	mcemaths_quatcpy(output->worldPos, temp[0].worldPos);
	mcemaths_quatcpy(output->texcoord, temp[0].texcoord);
	mcemaths_quatcpy(output->shadowcoord, temp[0].shadowcoord);

	mcemaths_add_3_4_ip(output->normal, temp[1].normal);
	mcemaths_add_3_4_ip(output->worldPos, temp[1].worldPos);
	mcemaths_add_3_4_ip(output->texcoord, temp[1].texcoord);
	mcemaths_add_3_4_ip(output->shadowcoord, temp[1].shadowcoord);

	mcemaths_add_3_4_ip(output->normal, temp[2].normal);
	mcemaths_add_3_4_ip(output->worldPos, temp[2].worldPos);
	mcemaths_add_3_4_ip(output->texcoord, temp[2].texcoord);
	mcemaths_add_3_4_ip(output->shadowcoord, temp[2].shadowcoord);
}

void IP_DiffuseOnly::calcStep(void* interpolatedUserDataStep, const void* interpolatedUserDataStart, const void* interpolatedUserDataEnd, int stepCount) const
{
	PROCDATA_DIFFUSEONLY* start = (PROCDATA_DIFFUSEONLY*)interpolatedUserDataStart;
	PROCDATA_DIFFUSEONLY* end = (PROCDATA_DIFFUSEONLY*)interpolatedUserDataEnd;
	PROCDATA_DIFFUSEONLY* step = (PROCDATA_DIFFUSEONLY*)interpolatedUserDataStep;

	float reciprocalStepCount = 0 == stepCount ? 1.0f : 1.0f / (float)stepCount;
	mcemaths_sub_3_4(step->normal, end->normal, start->normal);
	mcemaths_sub_3_4(step->worldPos, end->worldPos, start->worldPos);
	mcemaths_sub_3_4(step->texcoord, end->texcoord, start->texcoord);
	mcemaths_sub_3_4(step->shadowcoord, end->shadowcoord, start->shadowcoord);

	mcemaths_mul_3_4(step->normal, reciprocalStepCount);
	mcemaths_mul_3_4(step->worldPos, reciprocalStepCount);
	mcemaths_mul_3_4(step->texcoord, reciprocalStepCount);
	mcemaths_mul_3_4(step->shadowcoord, reciprocalStepCount);
}

void IP_DiffuseOnly::correctInterpolation(void* interpolatedUserData, const void* interpolatedUserDataStart, float correctionFactor2) const
{
	PROCDATA_DIFFUSEONLY* output = (PROCDATA_DIFFUSEONLY*)interpolatedUserData;
	const PROCDATA_DIFFUSEONLY* start = (const PROCDATA_DIFFUSEONLY*)interpolatedUserDataStart;
	copyUserDataDiffuseOnly(output, start);
	mcemaths_mul_3_4(output->normal, correctionFactor2);
	mcemaths_mul_3_4(output->worldPos, correctionFactor2);
	mcemaths_mul_3_4(output->texcoord, correctionFactor2);
	mcemaths_mul_3_4(output->shadowcoord, correctionFactor2);
}

void IP_DiffuseOnly::stepForward(void* interpolatedUserDataStart, const void* interpolatedUserDataStep, int stepCount) const
{
	PROCDATA_DIFFUSEONLY* start = (PROCDATA_DIFFUSEONLY*)interpolatedUserDataStart;
	const PROCDATA_DIFFUSEONLY* step = (PROCDATA_DIFFUSEONLY*)interpolatedUserDataStep;

	if(1 == stepCount)
	{
		mcemaths_add_3_4_ip(start->normal, step->normal);
		mcemaths_add_3_4_ip(start->worldPos, step->worldPos);
		mcemaths_add_3_4_ip(start->texcoord, step->texcoord);
		mcemaths_add_3_4_ip(start->shadowcoord, step->shadowcoord);
	}
	else
	{
		mcemaths_step_3_4_ip(start->normal, step->normal, (float)stepCount);
		mcemaths_step_3_4_ip(start->worldPos, step->worldPos, (float)stepCount);
		mcemaths_step_3_4_ip(start->texcoord, step->texcoord, (float)stepCount);
		mcemaths_step_3_4_ip(start->shadowcoord, step->shadowcoord, (float)stepCount);
	}
}

//////////////////////////////////////////////////////////////////////////
// FP_DiffuseOnly
void FP_DiffuseOnly::preprocess(const PURESOFTUNIFORM* uniforms, const void** textures)
{
	m_lightPos = (const float*)uniforms[20].data;
	m_lightDir = (const float*)uniforms[21].data;
	m_cameraPos = (const float*)uniforms[22].data;
	m_shadowTex = (const PuresoftFBO*)textures[*(const int*)uniforms[23].data];
	m_ambient = (const float*)uniforms[30].data;
	m_specularExponent = (const float*)uniforms[33].data;
	m_diffuseTex = (const PuresoftFBO*)textures[*(const int*)uniforms[40].data];
}

void FP_DiffuseOnly::process(const FragmentProcessorInput* input, FragmentProcessorOutput* output) const
{
	PURESOFTBGRA bytesColour;
	ALIGN16 float outputColour[4], ambiemtColour[4];

	const PROCDATA_DIFFUSEONLY* inData = (const PROCDATA_DIFFUSEONLY*)input->user;

	float shadowFactor = PuresoftSamplerProjection::get(m_shadowTex, inData->shadowcoord);

	PuresoftSampler2D::get4(m_diffuseTex, inData->texcoord[0], inData->texcoord[1], &bytesColour);
	outputColour[0] = bytesColour.elems.b;
	outputColour[1] = bytesColour.elems.g;
	outputColour[2] = bytesColour.elems.r;
	outputColour[3] = bytesColour.elems.a;

	mcemaths_quatcpy(ambiemtColour, outputColour);
	mcemaths_mulvec_3_4(ambiemtColour, m_ambient);

	ALIGN16 float L[4];
	mcemaths_sub_3_4(L, m_lightPos, inData->worldPos);
	mcemaths_norm_3_4(L);

	ALIGN16 float E[4];
	mcemaths_sub_3_4(E, m_cameraPos, inData->worldPos);
	mcemaths_norm_3_4(E);

	ALIGN16 float H[4];
	mcemaths_add_3_4(H, E, L);
	mcemaths_norm_3_4(H);

	enum {LAMBERT, SPECULAR};
	ALIGN16 float factors[4];

	factors[LAMBERT] = mcemaths_dot_3_4(L, inData->normal);

	float yawOfLight = (float)acos(mcemaths_dot_3_4(L, m_lightDir));
	factors[LAMBERT] *= yawOfLight < fieldOfLight ? 1.0f : (float)opt_pow(cos(yawOfLight - fieldOfLight), 150);

	factors[SPECULAR] = opt_pow(mcemaths_dot_3_4(H, inData->normal), (unsigned int)*m_specularExponent);

	mcemaths_clamp_3_4(factors, 0, 1.0f);

	mcemaths_mul_3_4(outputColour, (factors[LAMBERT] + factors[SPECULAR]) * shadowFactor);
	mcemaths_add_3_4_ip(outputColour, ambiemtColour);
	mcemaths_clamp_3_4(outputColour, 0, 255.0f);

	bytesColour.elems.r = (unsigned char)outputColour[2];
	bytesColour.elems.g = (unsigned char)outputColour[1];
	bytesColour.elems.b = (unsigned char)outputColour[0];
	bytesColour.elems.a = 0;

	output->write4(0, &bytesColour);
}

//////////////////////////////////////////////////////////////////////////
// VP_Shadow
void VP_Shadow::preprocess(const PURESOFTUNIFORM* uniforms)
{
	m_PVM = (const float*)uniforms[5].data;
}

void VP_Shadow::process(const VertexProcessorInput* input, VertexProcessorOutput* output) const
{
	ALIGN16 float shrink[4];
	mcemaths_quatcpy(shrink, (const float*)input->data[0]);
	mcemaths_mul_3(shrink, 0.9f);
	mcemaths_transform_m4v4(output->position, m_PVM, shrink);
}

//////////////////////////////////////////////////////////////////////////
// FP_Null
void FP_Null::preprocess(const PURESOFTUNIFORM* uniforms, const void** textures)
{}

void FP_Null::process(const FragmentProcessorInput* input, FragmentProcessorOutput* output) const
{}

//////////////////////////////////////////////////////////////////////////
// PP_Test
#include <intrin.h>
void PP_Test::process(int threadIndex, int threadCount, PuresoftFBO* frame, PuresoftFBO* depth)
{
	// amount of rows for this thread
	int bandSize = frame->getHeight() / threadCount;
	if(threadIndex == threadCount - 1)
	{
		bandSize += frame->getHeight() % threadCount;
	}

	// buffer entry for this thread
	uintptr_t frameBuffer = (uintptr_t)frame->getBuffer();
	frameBuffer += threadIndex * (bandSize * frame->getScanline());

	const unsigned char f[] = {50,50,50,50,50,50,50,50};
	__asm{
		lea eax,f
		movq mm2,[eax]
	}
	for(int y = 0; y < bandSize; y++)
	{
		PURESOFTBGRA* row = (PURESOFTBGRA*)frameBuffer;
		for(int x = 0; x < frame->getWidth(); x+=2)
		{
			__asm{
				mov eax,1
				movd mm1,eax
				mov edx,row
				movq mm0,[edx]
				paddb mm0,mm2
				movntq [edx],mm0
			}
			row+=2;
		}

		frameBuffer += frame->getScanline();
	}
	_mm_empty();
}