#pragma once

#pragma pack(16)
__declspec(align(16)) struct PROCDATA_DEF05
{
};
#pragma pack()

class VertexProcesserDEF05 : public PuresoftVertexProcessor
{
public:
	VertexProcesserDEF05(void);
	~VertexProcesserDEF05(void);

	void preprocess(const PURESOFTUNIFORM* uniforms);
	void process(const VertexProcessorInput* input, VertexProcessorOutput* output) const;

private:
	const float* m_PV;
	const float* m_M;
	const float* m_Mrot;
};

class InterpolationProcessorDEF05 : public PuresoftInterpolationProcessor
{
public:
	InterpolationProcessorDEF05(void);
	~InterpolationProcessorDEF05(void);

	size_t userDataBytes(void) const;
	void preprocess(const PURESOFTUNIFORM* uniforms);
	void interpolateByContributes(void* interpolatedUserData, const void** vertexUserData, const float* correctedContributes) const;
	void calcStep(void* interpolatedUserDataStep, const void* interpolatedUserDataStart, const void* interpolatedUserDataEnd, int stepCount) const;
	void interpolateBySteps(void* interpolatedUserData, void* interpolatedUserDataStart, const void* interpolatedUserDataStep, float correctionFactor2) const;
};

class FragmentProcessorDEF05 : public PuresoftFragmentProcessor
{
public:
	FragmentProcessorDEF05(void);
	~FragmentProcessorDEF05(void);

	void preprocess(const PURESOFTUNIFORM* uniforms, const void** textures);
	void process(const FragmentProcessorInput* input, FragmentProcessorOutput* output) const;

private:
	const float* m_lightPos;
	const float* m_cameraPos;
	const PuresoftFBO* m_diffuseTex;
};