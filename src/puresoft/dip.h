#pragma once
#include "proc.h"

class PuresoftDepthInterpolationProcessor :
	public PuresoftInterpolationProcessor
{
public:
	PuresoftDepthInterpolationProcessor(void);
	~PuresoftDepthInterpolationProcessor(void);

	void setInputExt(int idx, const void* ext);
	void process(const void** outputExt, const float* contributes, const float* vert_z, float interp_z);

private:
	float* m_input;
	float m_output;
};

