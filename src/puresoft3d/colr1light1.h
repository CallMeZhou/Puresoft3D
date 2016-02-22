#pragma once

#pragma pack(16)
__declspec(align(16)) struct PROCDATA_DEF02
{
	float normal[4];
	float worldPos[4];
	float colour[4];
};
#pragma pack()

class VertexProcesserDEF02 : public PuresoftVertexProcessor
{
public:
	VertexProcesserDEF02(void);
	~VertexProcesserDEF02(void);

	void preprocess(const void** uniforms);
	void process(const VertexProcessorInput* input, VertexProcessorOutput* output) const;

private:
	const float* m_PV;
	const float* m_M;
	const float* m_Mrot;
};

class InterpolationProcessorDEF02 : public PuresoftInterpolationProcessor
{
public:
	InterpolationProcessorDEF02(void);
	~InterpolationProcessorDEF02(void);

	size_t userDataBytes(void) const;
	void preprocess(const void** uniforms);
	void interpolateByContributes(void* interpolatedUserData, const void** vertexUserData, const float* correctedContributes) const;
	void calcStep(void* interpolatedUserDataStep, const void* interpolatedUserDataStart, const void* interpolatedUserDataEnd, int stepCount) const;
	void interpolateBySteps(void* interpolatedUserData, void* interpolatedUserDataStart, const void* interpolatedUserDataStep, float correctionFactor2) const;
};

class FragmentProcessorDEF02 : public PuresoftFragmentProcessor
{
public:
	FragmentProcessorDEF02(void);
	~FragmentProcessorDEF02(void);

	void preprocess(const void** uniforms, const void** textures);
	void process(const FragmentProcessorInput* input, FragmentProcessorOutput* output) const;

private:
	const float* m_lightPos;
	const float* m_cameraPos;
	const PuresoftFBO* m_diffuseTex;
};