#pragma once

#pragma pack(16)
__declspec(align(16)) struct PROCDATA_DEF03
{
	float tangent[4];
	float binormal[4];
	float normal[4];
	float worldPos[4];
	float texcoord[4];
};
#pragma pack()

class VertexProcesserDEF03 : public PuresoftVertexProcessor
{
public:
	VertexProcesserDEF03(void);
	~VertexProcesserDEF03(void);

	void preprocess(const PURESOFTUNIFORM* uniforms);
	void process(const VertexProcessorInput* input, VertexProcessorOutput* output) const;

private:
	const float* m_PV;
	const float* m_M;
	const float* m_Mrot;
};

class InterpolationProcessorDEF03 : public PuresoftInterpolationProcessor
{
public:
	InterpolationProcessorDEF03(void);
	~InterpolationProcessorDEF03(void);

	size_t userDataBytes(void) const;
	void preprocess(const PURESOFTUNIFORM* uniforms);
	void interpolateByContributes(void* interpolatedUserData, const void** vertexUserData, const float* correctedContributes) const;
	void calcStep(void* interpolatedUserDataStep, const void* interpolatedUserDataStart, const void* interpolatedUserDataEnd, int stepCount) const;
	void correctInterpolation(void* interpolatedUserData, const void* interpolatedUserDataStart, float correctionFactor2) const;
	void stepForward(void* interpolatedUserDataStart, const void* interpolatedUserDataStep, int stepCount) const;
};

class FragmentProcessorDEF03 : public PuresoftFragmentProcessor
{
public:
	FragmentProcessorDEF03(void);
	~FragmentProcessorDEF03(void);

	void preprocess(const PURESOFTUNIFORM* uniforms, const void** textures);
	void process(const FragmentProcessorInput* input, FragmentProcessorOutput* output) const;

private:
	const float* m_lightPos;
	const float* m_cameraPos;
	const PuresoftFBO* m_diffuseTex;
	const PuresoftFBO* m_bumpTex;
};