#include <memory.h>
#include "defs.h"
#include "mcemaths.h"
#include "samplr2d.h"
#include "samplrproj.h"
#include "testproc.h"

static void copyUserData(PROCDATA_PLANET* dest, const PROCDATA_PLANET* src)
{
	__asm
	{
		mov		eax,		src
		movaps	xmm0,		[eax]
		movaps	xmm1,		[eax + 16]
		movaps	xmm2,		[eax + 32]
		movaps	xmm3,		[eax + 48]
		movaps	xmm4,		[eax + 64]
		movaps	xmm5,		[eax + 80]
		mov		eax,		dest
		movaps	[eax],		xmm0
		movaps	[eax + 16],	xmm1
		movaps	[eax + 32],	xmm2
		movaps	[eax + 48],	xmm3
		movaps	[eax + 64],	xmm4
		movaps	[eax + 80],	xmm5
	}
}

void VP_Planet::preprocess(const PURESOFTUNIFORM* uniforms)
{
	m_PV = (const float*)uniforms[3].data;
	m_M = (const float*)uniforms[4].data;
	m_Mrot = (const float*)uniforms[5].data;
	m_shadowPV = (const float*)uniforms[16].data;
}

void VP_Planet::process(const VertexProcessorInput* input, VertexProcessorOutput* output) const
{
	PROCDATA_PLANET* userOutput = (PROCDATA_PLANET*)output->user;

	const float* position = (const float*)input->data[0];
	const float* tangent = (const float*)input->data[1];
	const float* binormal = (const float*)input->data[2];
	const float* normal = (const float*)input->data[3];
	const float* texcoord = (const float*)input->data[4];

	// get vertex position in world space
	mcemaths_transform_m4v4(userOutput->worldPos, m_M, position);

	// project world-position into shadow map's UV space, inp:userOutput->worldPos, outp:userOutput->shadowcoord
	mcemaths_quatcpy(userOutput->shadowcoord, userOutput->worldPos);
//	ALIGN16 float enlarge[4];
//	mcemaths_quatcpy(enlarge, normal);
//	mcemaths_mul_3_4(enlarge, 1.5f);
//	mcemaths_add_3_4_ip(userOutput->shadowcoord, enlarge); // slightly enlarge the model for anti-acne
	mcemaths_transform_m4v4_ip(userOutput->shadowcoord, m_shadowPV);

	// project world-position into view space
	mcemaths_quatcpy(output->position, userOutput->worldPos);
	mcemaths_transform_m4v4_ip(output->position, m_PV);

	// remove world-position's w (later we'll use it as an xyz coordinate)
	userOutput->worldPos[3] = 0;

	// transform normals
	mcemaths_transform_m4v4(userOutput->tangent, m_Mrot, tangent);
	mcemaths_transform_m4v4(userOutput->binormal, m_Mrot, binormal);
	mcemaths_transform_m4v4(userOutput->normal, m_Mrot, normal);

	// pass on texture coord
	userOutput->texcoord[0] = texcoord[0];
	userOutput->texcoord[1] = texcoord[1];
}

size_t IP_Planet::userDataBytes(void) const
{
	return sizeof(PROCDATA_PLANET);
}

void IP_Planet::preprocess(const PURESOFTUNIFORM* uniforms)
{
}

void IP_Planet::interpolateByContributes(void* interpolatedUserData, const void** vertexUserData, const float* correctedContributes) const
{
	__declspec(align(16)) PROCDATA_PLANET temp[3];
	copyUserData(temp,     (PROCDATA_PLANET*)vertexUserData[0]);
	copyUserData(temp + 1, (PROCDATA_PLANET*)vertexUserData[1]);
	copyUserData(temp + 2, (PROCDATA_PLANET*)vertexUserData[2]);

	mcemaths_mul_3_4(temp[0].tangent, correctedContributes[0]);
	mcemaths_mul_3_4(temp[0].binormal, correctedContributes[0]);
	mcemaths_mul_3_4(temp[0].normal, correctedContributes[0]);
	mcemaths_mul_3_4(temp[0].worldPos, correctedContributes[0]);
	mcemaths_mul_3_4(temp[0].texcoord, correctedContributes[0]);
	mcemaths_mul_3_4(temp[0].shadowcoord, correctedContributes[0]);

	mcemaths_mul_3_4(temp[1].tangent, correctedContributes[1]);
	mcemaths_mul_3_4(temp[1].binormal, correctedContributes[1]);
	mcemaths_mul_3_4(temp[1].normal, correctedContributes[1]);
	mcemaths_mul_3_4(temp[1].worldPos, correctedContributes[1]);
	mcemaths_mul_3_4(temp[1].texcoord, correctedContributes[1]);
	mcemaths_mul_3_4(temp[1].shadowcoord, correctedContributes[1]);

	mcemaths_mul_3_4(temp[2].tangent, correctedContributes[2]);
	mcemaths_mul_3_4(temp[2].binormal, correctedContributes[2]);
	mcemaths_mul_3_4(temp[2].normal, correctedContributes[2]);
	mcemaths_mul_3_4(temp[2].worldPos, correctedContributes[2]);
	mcemaths_mul_3_4(temp[2].texcoord, correctedContributes[2]);
	mcemaths_mul_3_4(temp[2].shadowcoord, correctedContributes[2]);

	PROCDATA_PLANET* output = (PROCDATA_PLANET*)interpolatedUserData;

	mcemaths_quatcpy(output->tangent, temp[0].tangent);
	mcemaths_quatcpy(output->binormal, temp[0].binormal);
	mcemaths_quatcpy(output->normal, temp[0].normal);
	mcemaths_quatcpy(output->worldPos, temp[0].worldPos);
	mcemaths_quatcpy(output->texcoord, temp[0].texcoord);
	mcemaths_quatcpy(output->shadowcoord, temp[0].shadowcoord);

	mcemaths_add_3_4_ip(output->tangent, temp[1].tangent);
	mcemaths_add_3_4_ip(output->binormal, temp[1].binormal);
	mcemaths_add_3_4_ip(output->normal, temp[1].normal);
	mcemaths_add_3_4_ip(output->worldPos, temp[1].worldPos);
	mcemaths_add_3_4_ip(output->texcoord, temp[1].texcoord);
	mcemaths_add_3_4_ip(output->shadowcoord, temp[1].shadowcoord);

	mcemaths_add_3_4_ip(output->tangent, temp[2].tangent);
	mcemaths_add_3_4_ip(output->binormal, temp[2].binormal);
	mcemaths_add_3_4_ip(output->normal, temp[2].normal);
	mcemaths_add_3_4_ip(output->worldPos, temp[2].worldPos);
	mcemaths_add_3_4_ip(output->texcoord, temp[2].texcoord);
	mcemaths_add_3_4_ip(output->shadowcoord, temp[2].shadowcoord);
}

void IP_Planet::calcStep(void* interpolatedUserDataStep, const void* interpolatedUserDataStart, const void* interpolatedUserDataEnd, int stepCount) const
{
	PROCDATA_PLANET* start = (PROCDATA_PLANET*)interpolatedUserDataStart;
	PROCDATA_PLANET* end = (PROCDATA_PLANET*)interpolatedUserDataEnd;
	PROCDATA_PLANET* step = (PROCDATA_PLANET*)interpolatedUserDataStep;

	float reciprocalStepCount = 0 == stepCount ? 1.0f : 1.0f / (float)stepCount;
	mcemaths_sub_3_4(step->tangent, end->tangent, start->tangent);
	mcemaths_sub_3_4(step->binormal, end->binormal, start->binormal);
	mcemaths_sub_3_4(step->normal, end->normal, start->normal);
	mcemaths_sub_3_4(step->worldPos, end->worldPos, start->worldPos);
	mcemaths_sub_3_4(step->texcoord, end->texcoord, start->texcoord);
	mcemaths_sub_3_4(step->shadowcoord, end->shadowcoord, start->shadowcoord);

	mcemaths_mul_3_4(step->tangent, reciprocalStepCount);
	mcemaths_mul_3_4(step->binormal, reciprocalStepCount);
	mcemaths_mul_3_4(step->normal, reciprocalStepCount);
	mcemaths_mul_3_4(step->worldPos, reciprocalStepCount);
	mcemaths_mul_3_4(step->texcoord, reciprocalStepCount);
	mcemaths_mul_3_4(step->shadowcoord, reciprocalStepCount);
}

void IP_Planet::correctInterpolation(void* interpolatedUserData, const void* interpolatedUserDataStart, float correctionFactor2) const
{
	PROCDATA_PLANET* output = (PROCDATA_PLANET*)interpolatedUserData;
	const PROCDATA_PLANET* start = (const PROCDATA_PLANET*)interpolatedUserDataStart;
	copyUserData(output, start);
	mcemaths_mul_3_4(output->tangent, correctionFactor2);
	mcemaths_mul_3_4(output->binormal, correctionFactor2);
	mcemaths_mul_3_4(output->normal, correctionFactor2);
	mcemaths_mul_3_4(output->worldPos, correctionFactor2);
	mcemaths_mul_3_4(output->texcoord, correctionFactor2);
	mcemaths_mul_3_4(output->shadowcoord, correctionFactor2);
}

void IP_Planet::stepForward(void* interpolatedUserDataStart, const void* interpolatedUserDataStep, int stepCount) const
{
	PROCDATA_PLANET* start = (PROCDATA_PLANET*)interpolatedUserDataStart;
	const PROCDATA_PLANET* step = (PROCDATA_PLANET*)interpolatedUserDataStep;

	if(1 == stepCount)
	{
		mcemaths_add_3_4_ip(start->tangent, step->normal);
		mcemaths_add_3_4_ip(start->binormal, step->normal);
		mcemaths_add_3_4_ip(start->normal, step->normal);
		mcemaths_add_3_4_ip(start->worldPos, step->worldPos);
		mcemaths_add_3_4_ip(start->texcoord, step->texcoord);
		mcemaths_add_3_4_ip(start->shadowcoord, step->shadowcoord);
	}
	else
	{
		mcemaths_step_3_4_ip(start->tangent, step->normal, (float)stepCount);
		mcemaths_step_3_4_ip(start->binormal, step->normal, (float)stepCount);
		mcemaths_step_3_4_ip(start->normal, step->normal, (float)stepCount);
		mcemaths_step_3_4_ip(start->worldPos, step->worldPos, (float)stepCount);
		mcemaths_step_3_4_ip(start->texcoord, step->texcoord, (float)stepCount);
		mcemaths_step_3_4_ip(start->shadowcoord, step->shadowcoord, (float)stepCount);
	}

}

void FP_Earth::preprocess(const PURESOFTUNIFORM* uniforms, const void** textures)
{
	m_lightPos = (const float*)uniforms[7].data;
	m_cameraPos = (const float*)uniforms[8].data;
	m_diffuseTex = (const PuresoftFBO*)textures[*(const int*)uniforms[9].data];
	m_bumpTex = (const PuresoftFBO*)textures[*(const int*)uniforms[10].data];
	m_specularTex = (const PuresoftFBO*)textures[*(const int*)uniforms[11].data];
	m_nightTex = (const PuresoftFBO*)textures[*(const int*)uniforms[12].data];
	m_shadowTex = (const PuresoftFBO*)textures[*(const int*)uniforms[15].data];
}

void FP_Earth::process(const FragmentProcessorInput* input, FragmentProcessorOutput* output) const
{
	ALIGN16 float outputColour[4];
	ALIGN16 float bumpNormal[4];
	ALIGN16 float nightColour[4];
	PURESOFTBGRA bytesColour;

	const PROCDATA_PLANET* inData = (const PROCDATA_PLANET*)input->user;

	PuresoftSampler2D::get4(m_diffuseTex, inData->texcoord[0], inData->texcoord[1], &bytesColour);
	outputColour[0] = bytesColour.elems.b;
	outputColour[1] = bytesColour.elems.g;
	outputColour[2] = bytesColour.elems.r;
	outputColour[3] = bytesColour.elems.a;

	PuresoftSampler2D::get4(m_nightTex, inData->texcoord[0], inData->texcoord[1], &bytesColour);
	nightColour[0] = bytesColour.elems.b;
	nightColour[1] = bytesColour.elems.g;
	nightColour[2] = bytesColour.elems.r;
	nightColour[3] = bytesColour.elems.a;

	PuresoftSampler2D::get4(m_bumpTex, inData->texcoord[0], inData->texcoord[1], &bytesColour);
	bumpNormal[0] = bytesColour.elems.r;
	bumpNormal[1] = bytesColour.elems.g;
	bumpNormal[2] = bytesColour.elems.b;
	bumpNormal[3] = 0;

	float shadowFactor = PuresoftSamplerProjection::get(m_shadowTex, inData->shadowcoord);

	PuresoftSampler2D::get4(m_specularTex, inData->texcoord[0], inData->texcoord[1], &bytesColour);
	float specularControl = bytesColour.elems.r / 255.0f;

	mcemaths_div_3_4(bumpNormal, 255.0f);
	mcemaths_mul_3_4(bumpNormal, 2.0f);
	mcemaths_sub_4by1(bumpNormal, 1.0f);

	ALIGN16 float tbn[16];
	mcemaths_make_tbn(tbn, inData->tangent, inData->binormal, inData->normal);

	mcemaths_transform_m4v4_ip(bumpNormal, tbn);
	mcemaths_norm_3_4(bumpNormal);

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

	float lambert = mcemaths_dot_3_4(L, bumpNormal);

	float specular = mcemaths_dot_3_4(H, bumpNormal);
	specular = specular < 0 ? 0 : specular;
	//specular = pow(specular, 50.0f);
	specular = opt_pow(specular, 50);
	specular *= specularControl;

	mcemaths_add_1to4(outputColour, 255.0f * specular);

	mcemaths_mul_3_4(outputColour, lambert);

	if(lambert < 0.2f)
	{
		mcemaths_add_3_4_ip(outputColour, nightColour);
	}

	mcemaths_mul_3_4(outputColour, shadowFactor);
	mcemaths_clamp_3_4(outputColour, 0, 255.0f);

	bytesColour.elems.r = (unsigned char)outputColour[2];
	bytesColour.elems.g = (unsigned char)outputColour[1];
	bytesColour.elems.b = (unsigned char)outputColour[0];
	bytesColour.elems.a = 0;

	output->write4(0, &bytesColour);
}

void FP_Satellite::preprocess(const PURESOFTUNIFORM* uniforms, const void** textures)
{
	m_lightPos = (const float*)uniforms[7].data;
	m_cameraPos = (const float*)uniforms[8].data;
	m_diffuseTex = (const PuresoftFBO*)textures[*(const int*)uniforms[9].data];
	m_bumpTex = (const PuresoftFBO*)textures[*(const int*)uniforms[10].data];
	m_shadowTex = (const PuresoftFBO*)textures[*(const int*)uniforms[15].data];
}

void FP_Satellite::process(const FragmentProcessorInput* input, FragmentProcessorOutput* output) const
{
	ALIGN16 float outputColour[4];
	ALIGN16 float bumpNormal[4];
	PURESOFTBGRA bytesColour;

	const PROCDATA_PLANET* inData = (const PROCDATA_PLANET*)input->user;

	PuresoftSampler2D::get4(m_diffuseTex, inData->texcoord[0], inData->texcoord[1], &bytesColour);
	outputColour[0] = bytesColour.elems.b;
	outputColour[1] = bytesColour.elems.g;
	outputColour[2] = bytesColour.elems.r;
	outputColour[3] = bytesColour.elems.a;

	PuresoftSampler2D::get4(m_bumpTex, inData->texcoord[0], inData->texcoord[1], &bytesColour);
	bumpNormal[0] = bytesColour.elems.r;
	bumpNormal[1] = bytesColour.elems.g;
	bumpNormal[2] = bytesColour.elems.b;
	bumpNormal[3] = 0;

	float shadowFactor = PuresoftSamplerProjection::get(m_shadowTex, inData->shadowcoord);

	mcemaths_div_3_4(bumpNormal, 255.0f);
	mcemaths_mul_3_4(bumpNormal, 2.0f);
	mcemaths_sub_4by1(bumpNormal, 1.0f);

	ALIGN16 float tbn[16];
	mcemaths_make_tbn(tbn, inData->tangent, inData->binormal, inData->normal);

	mcemaths_transform_m4v4_ip(bumpNormal, tbn);
	mcemaths_norm_3_4(bumpNormal);

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

	float lambert = mcemaths_dot_3_4(L, bumpNormal);

	float specular = mcemaths_dot_3_4(H, bumpNormal);
	specular = specular < 0 ? 0 : specular;
	//specular = pow(specular, 50.0f);
	specular = opt_pow(specular, 50);

	mcemaths_add_1to4(outputColour, 255.0f * specular);

	mcemaths_mul_3_4(outputColour, lambert);

	mcemaths_mul_3_4(outputColour, shadowFactor);
	mcemaths_clamp_3_4(outputColour, 0, 255.0f);

	bytesColour.elems.r = (unsigned char)outputColour[2];
	bytesColour.elems.g = (unsigned char)outputColour[1];
	bytesColour.elems.b = (unsigned char)outputColour[0];
	bytesColour.elems.a = 0;

	output->write4(0, &bytesColour);
}

//////////////////////////////////////////////////////////////////////////

static void copyUserData(PROCDATA_CLOUD* dest, const PROCDATA_CLOUD* src)
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

void VP_Cloud::preprocess(const PURESOFTUNIFORM* uniforms)
{
	m_PV = (const float*)uniforms[3].data;
	m_M = (const float*)uniforms[4].data;
	m_Mrot = (const float*)uniforms[5].data;
	m_shadowPV = (const float*)uniforms[16].data;
}

void VP_Cloud::process(const VertexProcessorInput* input, VertexProcessorOutput* output) const
{
	PROCDATA_CLOUD* userOutput = (PROCDATA_CLOUD*)output->user;

	const float* position = (const float*)input->data[0];
	const float* normal = (const float*)input->data[3];
	const float* texcoord = (const float*)input->data[4];

	// get vertex position in world space
	mcemaths_transform_m4v4(userOutput->worldPos, m_M, position);

	// project world-position into shadow map's UV space, inp:userOutput->worldPos, outp:userOutput->shadowcoord
	mcemaths_quatcpy(userOutput->shadowcoord, userOutput->worldPos);
	//	ALIGN16 float enlarge[4];
	//	mcemaths_quatcpy(enlarge, normal);
	//	mcemaths_mul_3_4(enlarge, 1.5f);
	//	mcemaths_add_3_4_ip(userOutput->shadowcoord, enlarge); // slightly enlarge the model for anti-acne
	mcemaths_transform_m4v4_ip(userOutput->shadowcoord, m_shadowPV);

	// project world-position into view space
	mcemaths_quatcpy(output->position, userOutput->worldPos);
	mcemaths_transform_m4v4_ip(output->position, m_PV);

	// remove world-position's w (later we'll use it as an xyz coordinate)
	userOutput->worldPos[3] = 0;

	// transform normal
	mcemaths_transform_m4v4(userOutput->normal, m_Mrot, normal);

	// pass on texture coord
	userOutput->texcoord[0] = texcoord[0];
	userOutput->texcoord[1] = texcoord[1];
}

size_t IP_Cloud::userDataBytes(void) const
{
	return sizeof(PROCDATA_CLOUD);
}

void IP_Cloud::preprocess(const PURESOFTUNIFORM* uniforms)
{
}

void IP_Cloud::interpolateByContributes(void* interpolatedUserData, const void** vertexUserData, const float* correctedContributes) const
{
	__declspec(align(16)) PROCDATA_CLOUD temp[3];
	copyUserData(temp,     (PROCDATA_CLOUD*)vertexUserData[0]);
	copyUserData(temp + 1, (PROCDATA_CLOUD*)vertexUserData[1]);
	copyUserData(temp + 2, (PROCDATA_CLOUD*)vertexUserData[2]);

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

	PROCDATA_CLOUD* output = (PROCDATA_CLOUD*)interpolatedUserData;

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

void IP_Cloud::calcStep(void* interpolatedUserDataStep, const void* interpolatedUserDataStart, const void* interpolatedUserDataEnd, int stepCount) const
{
	PROCDATA_CLOUD* start = (PROCDATA_CLOUD*)interpolatedUserDataStart;
	PROCDATA_CLOUD* end = (PROCDATA_CLOUD*)interpolatedUserDataEnd;
	PROCDATA_CLOUD* step = (PROCDATA_CLOUD*)interpolatedUserDataStep;

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

void IP_Cloud::correctInterpolation(void* interpolatedUserData, const void* interpolatedUserDataStart, float correctionFactor2) const
{
	PROCDATA_CLOUD* output = (PROCDATA_CLOUD*)interpolatedUserData;
	const PROCDATA_CLOUD* start = (const PROCDATA_CLOUD*)interpolatedUserDataStart;
	copyUserData(output, start);
	mcemaths_mul_3_4(output->normal, correctionFactor2);
	mcemaths_mul_3_4(output->worldPos, correctionFactor2);
	mcemaths_mul_3_4(output->texcoord, correctionFactor2);
	mcemaths_mul_3_4(output->shadowcoord, correctionFactor2);
}

void IP_Cloud::stepForward(void* interpolatedUserDataStart, const void* interpolatedUserDataStep, int stepCount) const
{
	PROCDATA_CLOUD* start = (PROCDATA_CLOUD*)interpolatedUserDataStart;
	const PROCDATA_CLOUD* step = (PROCDATA_CLOUD*)interpolatedUserDataStep;

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

void FP_Cloud::preprocess(const PURESOFTUNIFORM* uniforms, const void** textures)
{
	m_lightPos = (const float*)uniforms[7].data;
	m_cameraPos = (const float*)uniforms[8].data;
	m_diffuseTex = (const PuresoftFBO*)textures[*(const int*)uniforms[9].data];
	m_shadowTex = (const PuresoftFBO*)textures[*(const int*)uniforms[15].data];
}

void FP_Cloud::process(const FragmentProcessorInput* input, FragmentProcessorOutput* output) const
{
	ALIGN16 float cloudColour[4];
	PURESOFTBGRA textureColour;
	const PROCDATA_CLOUD* inData = (const PROCDATA_CLOUD*)input->user;

	PuresoftSampler2D::get4(m_diffuseTex, inData->texcoord[0], inData->texcoord[1], &textureColour);

	cloudColour[0] = 255.0f;
	cloudColour[1] = 255.0f;
	cloudColour[2] = 255.0f;
	cloudColour[3] = textureColour.elems.r;

	float shadowFactor = PuresoftSamplerProjection::get(m_shadowTex, inData->shadowcoord);

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

	float lambert = 2.0f * mcemaths_dot_3_4(L, inData->normal);

	mcemaths_mul_3(cloudColour, lambert * shadowFactor);

	__asm{
		movaps		xmm0,	[cloudColour]
		cvtps2dq	xmm0,	xmm0
		packusdw	xmm0,	xmm0
		packuswb	xmm0,	xmm0
		movss		[cloudColour], xmm0
	}

	output->write4(0, cloudColour);
}

//////////////////////////////////////////////////////////////////////////

static void copyUserData(PROCDATA_CLOUDSHADOW* dest, const PROCDATA_CLOUDSHADOW* src)
{
	__asm
	{
		mov		eax,		src
		movaps	xmm0,		[eax]
		mov		eax,		dest
		movaps	[eax],		xmm0
	}
}

void VP_CloudShadow::preprocess(const PURESOFTUNIFORM* uniforms)
{
	m_PV = (const float*)uniforms[3].data;
	m_M = (const float*)uniforms[4].data;
}

void VP_CloudShadow::process(const VertexProcessorInput* input, VertexProcessorOutput* output) const
{
	PROCDATA_CLOUDSHADOW* userOutput = (PROCDATA_CLOUDSHADOW*)output->user;
	const float* position = (const float*)input->data[0];
	const float* texcoord = (const float*)input->data[4];

	ALIGN16 float position_adjusted[4], pvm[16];
	mcemaths_quatcpy(position_adjusted, position);
	mcemaths_mul_3(position_adjusted, 0.95f);
	mcemaths_transform_m4m4(pvm, m_PV, m_M);
	mcemaths_transform_m4v4(output->position, pvm, position_adjusted);

	userOutput->texcoord[0] = texcoord[0];
	userOutput->texcoord[1] = texcoord[1];
}

size_t IP_CloudShadow::userDataBytes(void) const
{
	return sizeof(PROCDATA_CLOUDSHADOW);
}

void IP_CloudShadow::preprocess(const PURESOFTUNIFORM* uniforms)
{
}

void IP_CloudShadow::interpolateByContributes(void* interpolatedUserData, const void** vertexUserData, const float* correctedContributes) const
{
	__declspec(align(16)) PROCDATA_CLOUDSHADOW temp[3];
	copyUserData(temp,     (PROCDATA_CLOUDSHADOW*)vertexUserData[0]);
	copyUserData(temp + 1, (PROCDATA_CLOUDSHADOW*)vertexUserData[1]);
	copyUserData(temp + 2, (PROCDATA_CLOUDSHADOW*)vertexUserData[2]);

	mcemaths_mul_3_4(temp[0].texcoord, correctedContributes[0]);
	mcemaths_mul_3_4(temp[1].texcoord, correctedContributes[1]);
	mcemaths_mul_3_4(temp[2].texcoord, correctedContributes[2]);

	PROCDATA_CLOUDSHADOW* output = (PROCDATA_CLOUDSHADOW*)interpolatedUserData;

	mcemaths_quatcpy(output->texcoord, temp[0].texcoord);
	mcemaths_add_3_4_ip(output->texcoord, temp[1].texcoord);
	mcemaths_add_3_4_ip(output->texcoord, temp[2].texcoord);
}

void IP_CloudShadow::calcStep(void* interpolatedUserDataStep, const void* interpolatedUserDataStart, const void* interpolatedUserDataEnd, int stepCount) const
{
	PROCDATA_CLOUDSHADOW* start = (PROCDATA_CLOUDSHADOW*)interpolatedUserDataStart;
	PROCDATA_CLOUDSHADOW* end = (PROCDATA_CLOUDSHADOW*)interpolatedUserDataEnd;
	PROCDATA_CLOUDSHADOW* step = (PROCDATA_CLOUDSHADOW*)interpolatedUserDataStep;

	float reciprocalStepCount = 0 == stepCount ? 1.0f : 1.0f / (float)stepCount;
	mcemaths_sub_3_4(step->texcoord, end->texcoord, start->texcoord);
	mcemaths_mul_3_4(step->texcoord, reciprocalStepCount);
}

void IP_CloudShadow::correctInterpolation(void* interpolatedUserData, const void* interpolatedUserDataStart, float correctionFactor2) const
{
	PROCDATA_CLOUDSHADOW* output = (PROCDATA_CLOUDSHADOW*)interpolatedUserData;
	const PROCDATA_CLOUDSHADOW* start = (const PROCDATA_CLOUDSHADOW*)interpolatedUserDataStart;
	copyUserData(output, start);
	mcemaths_mul_3_4(output->texcoord, correctionFactor2);
}

void IP_CloudShadow::stepForward(void* interpolatedUserDataStart, const void* interpolatedUserDataStep, int stepCount) const
{
	PROCDATA_CLOUDSHADOW* start = (PROCDATA_CLOUDSHADOW*)interpolatedUserDataStart;
	const PROCDATA_CLOUDSHADOW* step = (PROCDATA_CLOUDSHADOW*)interpolatedUserDataStep;

	if(1 == stepCount)
	{
		mcemaths_add_3_4_ip(start->texcoord, step->texcoord);
	}
	else
	{
		mcemaths_step_3_4_ip(start->texcoord, step->texcoord, (float)stepCount);
	}
}

void FP_CloudShadow::preprocess(const PURESOFTUNIFORM* uniforms, const void** textures)
{
	m_cloudTex = (const PuresoftFBO*)textures[*(const int*)uniforms[9].data];
}

void FP_CloudShadow::process(const FragmentProcessorInput* input, FragmentProcessorOutput* output) const
{
	const PROCDATA_CLOUDSHADOW* inData = (const PROCDATA_CLOUDSHADOW*)input->user;

	PURESOFTBGRA textureColour;
	PuresoftSampler2D::get4(m_cloudTex, inData->texcoord[0], inData->texcoord[1], &textureColour);

	if(textureColour.elems.r < 150)
		output->discard();
}