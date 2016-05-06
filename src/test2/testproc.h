#pragma once
#include "proc.h"

/*
Uniform Positions

0	model
1	model-rotation
2	view
3	projection
4	p-v
5	p-v-m
6	shadow : p-v

20	(shared lighting) light position
21	(shared lighting) light direction
22	(shared lighting) camera position
23	(shared lighting) shadow texture

30	(single colour) ambient colour
31	(single colour) diffuse colour
32	(single colour) specular colour
33	(single colour) specular exponent

40	(basic textures) diffuse
41	(basic textures) bump
42	(basic textures) specular colour
43	(basic textures) specular exponent

VBOs

0	position
1	tangent
2	binormal
3	normal
4	texcoord
*/

#pragma once

//////////////////////////////////////////////////////////////////////////
// for light-source objects

class VP_PositionOnly : public PuresoftVertexProcessor
{
public:
	void preprocess(const PURESOFTUNIFORM* uniforms);
	void process(const VertexProcessorInput* input, VertexProcessorOutput* output) const;
private:
	const float* m_PVM;
};

// shadow creator shares this
class IP_Null : public PuresoftInterpolationProcessor
{
public:
	size_t userDataBytes(void) const;
	void preprocess(const PURESOFTUNIFORM* uniforms);
	void interpolateByContributes(void* interpolatedUserData, const void** vertexUserData, const float* correctedContributes) const;
	void calcStep(void* interpolatedUserDataStep, const void* interpolatedUserDataStart, const void* interpolatedUserDataEnd, int stepCount) const;
	void correctInterpolation(void* interpolatedUserData, const void* interpolatedUserDataStart, float correctionFactor2) const;
	void stepForward(void* interpolatedUserDataStart, const void* interpolatedUserDataStep, int stepCount) const;
};

class FP_SingleColourNoLighting : public PuresoftFragmentProcessor
{
public:
	void preprocess(const PURESOFTUNIFORM* uniforms, const void** textures);
	void process(const FragmentProcessorInput* input, FragmentProcessorOutput* output) const;

private:
	const float* m_diffuse;
};

//////////////////////////////////////////////////////////////////////////
// for single colour objects

__declspec(align(16)) struct PROCDATA_SINGLECOLOUR
{
	float normal[4];
	float worldPos[4];
	float shadowcoord[4];
};

class VP_SingleColour : public PuresoftVertexProcessor
{
public:
	void preprocess(const PURESOFTUNIFORM* uniforms);
	void process(const VertexProcessorInput* input, VertexProcessorOutput* output) const;
private:
	const float* m_M;
	const float* m_MR;
	const float* m_PVM;
	const float* m_shadowPV;
};

class IP_SingleColour : public PuresoftInterpolationProcessor
{
public:
	size_t userDataBytes(void) const;
	void preprocess(const PURESOFTUNIFORM* uniforms);
	void interpolateByContributes(void* interpolatedUserData, const void** vertexUserData, const float* correctedContributes) const;
	void calcStep(void* interpolatedUserDataStep, const void* interpolatedUserDataStart, const void* interpolatedUserDataEnd, int stepCount) const;
	void correctInterpolation(void* interpolatedUserData, const void* interpolatedUserDataStart, float correctionFactor2) const;
	void stepForward(void* interpolatedUserDataStart, const void* interpolatedUserDataStep, int stepCount) const;
};

class FP_SingleColour : public PuresoftFragmentProcessor
{
public:
	void preprocess(const PURESOFTUNIFORM* uniforms, const void** textures);
	void process(const FragmentProcessorInput* input, FragmentProcessorOutput* output) const;

private:
	const float* m_lightPos;
	const float* m_lightDir;
	const float* m_cameraPos;
	const float* m_ambient;
	const float* m_diffuse;
	const float* m_specularColour;
	const float* m_specularExponent;
	const PuresoftFBO* m_shadowTex;
};

//////////////////////////////////////////////////////////////////////////
// for single texture objects

#pragma pack(16)
__declspec(align(16)) struct PROCDATA_DIFFUSEONLY
{
	float normal[4];
	float worldPos[4];
	float texcoord[4];
	float shadowcoord[4];
};
#pragma pack()

class VP_DiffuseOnly : public PuresoftVertexProcessor
{
public:
	void preprocess(const PURESOFTUNIFORM* uniforms);
	void process(const VertexProcessorInput* input, VertexProcessorOutput* output) const;

private:
	const float* m_M;
	const float* m_MR;
	const float* m_PV;
	const float* m_PVM;
	const float* m_shadowPV;
};

class IP_DiffuseOnly : public PuresoftInterpolationProcessor
{
public:
	size_t userDataBytes(void) const;
	void preprocess(const PURESOFTUNIFORM* uniforms);
	void interpolateByContributes(void* interpolatedUserData, const void** vertexUserData, const float* correctedContributes) const;
	void calcStep(void* interpolatedUserDataStep, const void* interpolatedUserDataStart, const void* interpolatedUserDataEnd, int stepCount) const;
	void correctInterpolation(void* interpolatedUserData, const void* interpolatedUserDataStart, float correctionFactor2) const;
	void stepForward(void* interpolatedUserDataStart, const void* interpolatedUserDataStep, int stepCount) const;
};

class FP_DiffuseOnly : public PuresoftFragmentProcessor
{
public:
	void preprocess(const PURESOFTUNIFORM* uniforms, const void** textures);
	void process(const FragmentProcessorInput* input, FragmentProcessorOutput* output) const;

private:
	const float* m_lightPos;
	const float* m_lightDir;
	const float* m_cameraPos;
	const float* m_ambient;
	const float* m_specularExponent;
	const PuresoftFBO* m_diffuseTex;
	const PuresoftFBO* m_shadowTex;
};

//////////////////////////////////////////////////////////////////////////
// for shadow map creating

class VP_Shadow : public PuresoftVertexProcessor
{
public:
	void preprocess(const PURESOFTUNIFORM* uniforms);
	void process(const VertexProcessorInput* input, VertexProcessorOutput* output) const;

private:
	const float* m_PVM;
};

class FP_Null : public PuresoftFragmentProcessor
{
public:
	void preprocess(const PURESOFTUNIFORM* uniforms, const void** textures);
	void process(const FragmentProcessorInput* input, FragmentProcessorOutput* output) const;
};