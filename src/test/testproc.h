#pragma once
#include "proc.h"

#pragma once

#pragma pack(16)
__declspec(align(16)) struct PROCDATA_PLANET
{
	float tangent[4];
	float binormal[4];
	float normal[4];
	float worldPos[4];
	float texcoord[4];
	float shadowcoord[4];
};
#pragma pack()

class VP_Planet : public PuresoftVertexProcessor
{
public:
	void preprocess(const PURESOFTUNIFORM* uniforms);
	void process(const VertexProcessorInput* input, VertexProcessorOutput* output) const;

private:
	const float* m_PV;
	const float* m_M;
	const float* m_Mrot;
	const float* m_shadowPV;
};

class IP_Planet : public PuresoftInterpolationProcessor
{
public:
	size_t userDataBytes(void) const;
	void preprocess(const PURESOFTUNIFORM* uniforms);
	void interpolateByContributes(void* interpolatedUserData, const void** vertexUserData, const float* correctedContributes) const;
	void calcStep(void* interpolatedUserDataStep, const void* interpolatedUserDataStart, const void* interpolatedUserDataEnd, int stepCount) const;
	void correctInterpolation(void* interpolatedUserData, const void* interpolatedUserDataStart, float correctionFactor2) const;
	void stepForward(void* interpolatedUserDataStart, const void* interpolatedUserDataStep, int stepCount) const;
};

class FP_Earth : public PuresoftFragmentProcessor
{
public:
	void preprocess(const PURESOFTUNIFORM* uniforms, const void** textures);
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
	const PuresoftFBO* m_shadowTex;
};

class FP_Satellite : public PuresoftFragmentProcessor
{
public:
	void preprocess(const PURESOFTUNIFORM* uniforms, const void** textures);
	void process(const FragmentProcessorInput* input, FragmentProcessorOutput* output) const;

private:
	const float* m_lightPos;
	const float* m_cameraPos;
	const PuresoftFBO* m_diffuseTex;
	const PuresoftFBO* m_bumpTex;
	const PuresoftFBO* m_shadowTex;
};

#pragma pack(16)
__declspec(align(16)) struct PROCDATA_CLOUD
{
	float normal[4];
	float worldPos[4];
	float texcoord[4];
	float shadowcoord[4];
};
#pragma pack()

class VP_Cloud : public PuresoftVertexProcessor
{
public:
	void preprocess(const PURESOFTUNIFORM* uniforms);
	void process(const VertexProcessorInput* input, VertexProcessorOutput* output) const;

private:
	const float* m_PV;
	const float* m_M;
	const float* m_Mrot;
	const float* m_shadowPV;
};

class IP_Cloud : public PuresoftInterpolationProcessor
{
public:
	size_t userDataBytes(void) const;
	void preprocess(const PURESOFTUNIFORM* uniforms);
	void interpolateByContributes(void* interpolatedUserData, const void** vertexUserData, const float* correctedContributes) const;
	void calcStep(void* interpolatedUserDataStep, const void* interpolatedUserDataStart, const void* interpolatedUserDataEnd, int stepCount) const;
	void correctInterpolation(void* interpolatedUserData, const void* interpolatedUserDataStart, float correctionFactor2) const;
	void stepForward(void* interpolatedUserDataStart, const void* interpolatedUserDataStep, int stepCount) const;
};

class FP_Cloud : public PuresoftFragmentProcessor
{
public:
	void preprocess(const PURESOFTUNIFORM* uniforms, const void** textures);
	void process(const FragmentProcessorInput* input, FragmentProcessorOutput* output) const;

private:
	const float* m_lightPos;
	const float* m_cameraPos;
	const PuresoftFBO* m_diffuseTex;
	const PuresoftFBO* m_shadowTex;
};

#pragma pack(16)
__declspec(align(16)) struct PROCDATA_CLOUDSHADOW
{
	float texcoord[4];
};
#pragma pack()

class VP_CloudShadow : public PuresoftVertexProcessor
{
public:
	void preprocess(const PURESOFTUNIFORM* uniforms);
	void process(const VertexProcessorInput* input, VertexProcessorOutput* output) const;

private:
	const float* m_PV;
	const float* m_M;
};

class IP_CloudShadow : public PuresoftInterpolationProcessor
{
public:
	size_t userDataBytes(void) const;
	void preprocess(const PURESOFTUNIFORM* uniforms);
	void interpolateByContributes(void* interpolatedUserData, const void** vertexUserData, const float* correctedContributes) const;
	void calcStep(void* interpolatedUserDataStep, const void* interpolatedUserDataStart, const void* interpolatedUserDataEnd, int stepCount) const;
	void correctInterpolation(void* interpolatedUserData, const void* interpolatedUserDataStart, float correctionFactor2) const;
	void stepForward(void* interpolatedUserDataStart, const void* interpolatedUserDataStep, int stepCount) const;
};

class FP_CloudShadow : public PuresoftFragmentProcessor
{
	const PuresoftFBO* m_cloudTex;
public:
	void preprocess(const PURESOFTUNIFORM* uniforms, const void** textures);
	void process(const FragmentProcessorInput* input, FragmentProcessorOutput* output) const;
};