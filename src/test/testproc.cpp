#include <memory.h>
#include "defs.h"
#include "mcemaths.h"
#include "samplr2d.h"
#include "testproc.h"

static void copyUserData(PROCDATA_TEST* dest, const PROCDATA_TEST* src)
{
	__asm
	{
		mov		eax,		src
		movaps	xmm0,		[eax]
		movaps	xmm1,		[eax + 16]
		movaps	xmm2,		[eax + 32]
		movaps	xmm3,		[eax + 48]
		movaps	xmm4,		[eax + 64]
		mov		eax,		dest
		movaps	[eax],		xmm0
		movaps	[eax + 16],	xmm1
		movaps	[eax + 32],	xmm2
		movaps	[eax + 48],	xmm3
		movaps	[eax + 64],	xmm4
	}
}

VertexProcesserTEST::VertexProcesserTEST(void)
{
}

VertexProcesserTEST::~VertexProcesserTEST(void)
{
}

void VertexProcesserTEST::preprocess(const PURESOFTUNIFORM* uniforms)
{
	m_PV = (const float*)uniforms[3].data;
	m_M = (const float*)uniforms[4].data;
	m_Mrot = (const float*)uniforms[5].data;
}

void VertexProcesserTEST::process(const VertexProcessorInput* input, VertexProcessorOutput* output) const
{
	PROCDATA_TEST* userOutput = (PROCDATA_TEST*)output->user;

	const float* position = (const float*)input->data[0];
	const float* tangent = (const float*)input->data[1];
	const float* binormal = (const float*)input->data[2];
	const float* normal = (const float*)input->data[3];
	const float* texcoord = (const float*)input->data[4];

	mcemaths_transform_m4v4(userOutput->worldPos, m_M, position);

	mcemaths_quatcpy(output->position, userOutput->worldPos);
	userOutput->worldPos[3] = 0;

	mcemaths_transform_m4v4_ip(output->position, m_PV);

	mcemaths_transform_m4v4(userOutput->tangent, m_Mrot, tangent);
	mcemaths_transform_m4v4(userOutput->binormal, m_Mrot, binormal);
	mcemaths_transform_m4v4(userOutput->normal, m_Mrot, normal);

	userOutput->texcoord[0] = texcoord[0];
	userOutput->texcoord[1] = texcoord[1];
}

InterpolationProcessorTEST::InterpolationProcessorTEST(void)
{
}

InterpolationProcessorTEST::~InterpolationProcessorTEST(void)
{
}

size_t InterpolationProcessorTEST::userDataBytes(void) const
{
	return sizeof(PROCDATA_TEST);
}

void InterpolationProcessorTEST::preprocess(const PURESOFTUNIFORM* uniforms)
{
}

void InterpolationProcessorTEST::interpolateByContributes(void* interpolatedUserData, const void** vertexUserData, const float* correctedContributes) const
{
	__declspec(align(16)) PROCDATA_TEST temp[3];
	copyUserData(temp,     (PROCDATA_TEST*)vertexUserData[0]);
	copyUserData(temp + 1, (PROCDATA_TEST*)vertexUserData[1]);
	copyUserData(temp + 2, (PROCDATA_TEST*)vertexUserData[2]);

	mcemaths_mul_3_4(temp[0].tangent, correctedContributes[0]);
	mcemaths_mul_3_4(temp[0].binormal, correctedContributes[0]);
	mcemaths_mul_3_4(temp[0].normal, correctedContributes[0]);
	mcemaths_mul_3_4(temp[0].worldPos, correctedContributes[0]);
	mcemaths_mul_3_4(temp[0].texcoord, correctedContributes[0]);

	mcemaths_mul_3_4(temp[1].tangent, correctedContributes[1]);
	mcemaths_mul_3_4(temp[1].binormal, correctedContributes[1]);
	mcemaths_mul_3_4(temp[1].normal, correctedContributes[1]);
	mcemaths_mul_3_4(temp[1].worldPos, correctedContributes[1]);
	mcemaths_mul_3_4(temp[1].texcoord, correctedContributes[1]);

	mcemaths_mul_3_4(temp[2].tangent, correctedContributes[2]);
	mcemaths_mul_3_4(temp[2].binormal, correctedContributes[2]);
	mcemaths_mul_3_4(temp[2].normal, correctedContributes[2]);
	mcemaths_mul_3_4(temp[2].worldPos, correctedContributes[2]);
	mcemaths_mul_3_4(temp[2].texcoord, correctedContributes[2]);

	PROCDATA_TEST* output = (PROCDATA_TEST*)interpolatedUserData;

	mcemaths_quatcpy(output->tangent, temp[0].tangent);
	mcemaths_quatcpy(output->binormal, temp[0].binormal);
	mcemaths_quatcpy(output->normal, temp[0].normal);
	mcemaths_quatcpy(output->worldPos, temp[0].worldPos);
	mcemaths_quatcpy(output->texcoord, temp[0].texcoord);

	mcemaths_add_3_4_ip(output->tangent, temp[1].tangent);
	mcemaths_add_3_4_ip(output->binormal, temp[1].binormal);
	mcemaths_add_3_4_ip(output->normal, temp[1].normal);
	mcemaths_add_3_4_ip(output->worldPos, temp[1].worldPos);
	mcemaths_add_3_4_ip(output->texcoord, temp[1].texcoord);

	mcemaths_add_3_4_ip(output->tangent, temp[2].tangent);
	mcemaths_add_3_4_ip(output->binormal, temp[2].binormal);
	mcemaths_add_3_4_ip(output->normal, temp[2].normal);
	mcemaths_add_3_4_ip(output->worldPos, temp[2].worldPos);
	mcemaths_add_3_4_ip(output->texcoord, temp[2].texcoord);
}

void InterpolationProcessorTEST::calcStep(void* interpolatedUserDataStep, const void* interpolatedUserDataStart, const void* interpolatedUserDataEnd, int stepCount) const
{
	PROCDATA_TEST* start = (PROCDATA_TEST*)interpolatedUserDataStart;
	PROCDATA_TEST* end = (PROCDATA_TEST*)interpolatedUserDataEnd;
	PROCDATA_TEST* step = (PROCDATA_TEST*)interpolatedUserDataStep;

	float reciprocalStepCount = 0 == stepCount ? 1.0f : 1.0f / (float)stepCount;
	mcemaths_sub_3_4(step->tangent, end->tangent, start->tangent);
	mcemaths_sub_3_4(step->binormal, end->binormal, start->binormal);
	mcemaths_sub_3_4(step->normal, end->normal, start->normal);
	mcemaths_sub_3_4(step->worldPos, end->worldPos, start->worldPos);
	mcemaths_sub_3_4(step->texcoord, end->texcoord, start->texcoord);
	mcemaths_mul_3_4(step->tangent, reciprocalStepCount);
	mcemaths_mul_3_4(step->binormal, reciprocalStepCount);
	mcemaths_mul_3_4(step->normal, reciprocalStepCount);
	mcemaths_mul_3_4(step->worldPos, reciprocalStepCount);
	mcemaths_mul_3_4(step->texcoord, reciprocalStepCount);
}

void InterpolationProcessorTEST::interpolateBySteps(void* interpolatedUserData, void* interpolatedUserDataStart, const void* interpolatedUserDataStep, float correctionFactor2) const
{
	PROCDATA_TEST* output = (PROCDATA_TEST*)interpolatedUserData;
	PROCDATA_TEST* start = (PROCDATA_TEST*)interpolatedUserDataStart;
	copyUserData(output, start);
	mcemaths_mul_3_4(output->tangent, correctionFactor2);
	mcemaths_mul_3_4(output->binormal, correctionFactor2);
	mcemaths_mul_3_4(output->normal, correctionFactor2);
	mcemaths_mul_3_4(output->worldPos, correctionFactor2);
	mcemaths_mul_3_4(output->texcoord, correctionFactor2);

	const PROCDATA_TEST* step = (PROCDATA_TEST*)interpolatedUserDataStep;

	mcemaths_add_3_4_ip(start->tangent, step->normal);
	mcemaths_add_3_4_ip(start->binormal, step->normal);
	mcemaths_add_3_4_ip(start->normal, step->normal);
	mcemaths_add_3_4_ip(start->worldPos, step->worldPos);
	mcemaths_add_3_4_ip(start->texcoord, step->texcoord);
}

FragmentProcessorTEST::FragmentProcessorTEST(void)
{
}

FragmentProcessorTEST::~FragmentProcessorTEST(void)
{
}

void FragmentProcessorTEST::preprocess(const PURESOFTUNIFORM* uniforms, const void** textures)
{
	m_lightPos = (const float*)uniforms[7].data;
	m_cameraPos = (const float*)uniforms[8].data;
	m_diffuseTex = (const PuresoftFBO*)textures[*(const int*)uniforms[9].data];
	m_bumpTex = (const PuresoftFBO*)textures[*(const int*)uniforms[10].data];
	m_specularTex = (const PuresoftFBO*)textures[*(const int*)uniforms[11].data];
	m_nightTex = (const PuresoftFBO*)textures[*(const int*)uniforms[12].data];
	m_cloudTex = (const PuresoftFBO*)textures[*(const int*)uniforms[13].data];
	m_texMatrix = (const float*)uniforms[14].data;
}

void FragmentProcessorTEST::process(const FragmentProcessorInput* input, FragmentProcessorOutput* output) const
{
	ALIGN16 float outputColour[4];
	ALIGN16 float bumpNormal[4];
	ALIGN16 float nightColour[4];
	ALIGN16 float texcoord[4];
	PURESOFTBGRA bytesColour;

	const PROCDATA_TEST* inData = (const PROCDATA_TEST*)input->user;

	mcemaths_quatcpy(texcoord, inData->texcoord);
	texcoord[2] = 0;
	texcoord[3] = 1.0f;
	mcemaths_transform_m4v4_ip(texcoord, m_texMatrix);

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

	PuresoftSampler2D::get4(m_cloudTex, texcoord[0], texcoord[1], &bytesColour);
	float cloudControl = bytesColour.elems.r / 255.0f;

	PuresoftSampler2D::get4(m_specularTex, inData->texcoord[0], inData->texcoord[1], &bytesColour);
	float specularControl = bytesColour.elems.r / 255.0f;

	mcemaths_div_3_4(bumpNormal, 255.0f);
	mcemaths_mul_3_4(bumpNormal, 2.0f);
	mcemaths_sub_4by1(bumpNormal, 1.0f);

	ALIGN16 float tbn[16];
	mcemaths_make_tbn(tbn, inData->tangent, inData->binormal, inData->normal);
	//mcemaths_mat4transpose(tbn);

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

	ALIGN16 float cloud[4] = {255.0f, 255.0f, 255.0f, 0};
	mcemaths_mul_3_4(cloud, cloudControl);
	mcemaths_add_3_4_ip(outputColour, cloud);

	mcemaths_mul_3_4(outputColour, lambert);

	if(lambert < 0.2f)
	{
		mcemaths_add_3_4_ip(outputColour, nightColour);
	}

	mcemaths_clamp_3_4(outputColour, 0, 255.0f);

	bytesColour.elems.r = (unsigned char)outputColour[2];
	bytesColour.elems.g = (unsigned char)outputColour[1];
	bytesColour.elems.b = (unsigned char)outputColour[0];
	bytesColour.elems.a = 0;

	output->write4(0, &bytesColour);
}