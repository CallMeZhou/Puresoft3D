#pragma once

#pragma pack(16)
__declspec(align(16)) struct PROCDATA_DEF01
{
	float normal[4];
	float worldPos[4];
	float texcoord[4];
};
#pragma pack()

class VertexProcesserDEF01 : public PuresoftVertexProcessor
{
public:
	VertexProcesserDEF01(void);
	~VertexProcesserDEF01(void);

	void preprocess(const PURESOFTUNIFORM* uniforms);
	void process(const VertexProcessorInput* input, VertexProcessorOutput* output) const;

private:
	const float* m_PV;
	const float* m_M;
	const float* m_Mrot;
};

class InterpolationProcessorDEF01 : public PuresoftInterpolationProcessor
{
public:
	InterpolationProcessorDEF01(void);
	~InterpolationProcessorDEF01(void);

	size_t userDataBytes(void) const;
	void preprocess(const PURESOFTUNIFORM* uniforms);
	void interpolateByContributes(void* interpolatedUserData, const void** vertexUserData, const float* correctedContributes) const;
	void calcStep(void* interpolatedUserDataStep, const void* interpolatedUserDataStart, const void* interpolatedUserDataEnd, int stepCount) const;
	void correctInterpolation(void* interpolatedUserData, const void* interpolatedUserDataStart, float correctionFactor2) const;
	void stepForward(void* interpolatedUserDataStart, const void* interpolatedUserDataStep, int stepCount) const;
};

class FragmentProcessorDEF01 : public PuresoftFragmentProcessor
{
public:
	FragmentProcessorDEF01(void);
	~FragmentProcessorDEF01(void);

	void preprocess(const PURESOFTUNIFORM* uniforms, const void** textures);
	void process(const FragmentProcessorInput* input, FragmentProcessorOutput* output) const;

private:
	const float* m_lightPos;
	const float* m_cameraPos;
	const PuresoftFBO* m_diffuseTex;
};