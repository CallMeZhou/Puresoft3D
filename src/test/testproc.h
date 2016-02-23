#pragma once
#include "proc.h"

#pragma once

#pragma pack(16)
__declspec(align(16)) struct PROCDATA_TEST
{
	float tangent[4];
	float binormal[4];
	float normal[4];
	float worldPos[4];
	float texcoord[4];
};
#pragma pack()

class VertexProcesserTEST : public PuresoftVertexProcessor
{
public:
	VertexProcesserTEST(void);
	~VertexProcesserTEST(void);

	void preprocess(const void** uniforms);
	void process(const VertexProcessorInput* input, VertexProcessorOutput* output) const;

private:
	const float* m_PV;
	const float* m_M;
	const float* m_Mrot;
};

class InterpolationProcessorTEST : public PuresoftInterpolationProcessor
{
public:
	InterpolationProcessorTEST(void);
	~InterpolationProcessorTEST(void);

	size_t userDataBytes(void) const;
	void preprocess(const void** uniforms);
	void interpolateByContributes(void* interpolatedUserData, const void** vertexUserData, const float* correctedContributes) const;
	void calcStep(void* interpolatedUserDataStep, const void* interpolatedUserDataStart, const void* interpolatedUserDataEnd, int stepCount) const;
	void interpolateBySteps(void* interpolatedUserData, void* interpolatedUserDataStart, const void* interpolatedUserDataStep, float correctionFactor2) const;
};

class FragmentProcessorTEST : public PuresoftFragmentProcessor
{
public:
	FragmentProcessorTEST(void);
	~FragmentProcessorTEST(void);

	void preprocess(const void** uniforms, const void** textures);
	void process(const FragmentProcessorInput* input, FragmentProcessorOutput* output) const;

private:
	const float* m_lightPos;
	const float* m_cameraPos;
	const float* m_texMatrix;
	const PuresoftFBO* m_diffuseTex;
	const PuresoftFBO* m_bumpTex;
	const PuresoftFBO* m_specularTex;
	const PuresoftFBO* m_nightTex;
	const PuresoftFBO* m_cloudTex;
};