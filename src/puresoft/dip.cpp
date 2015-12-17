#include "StdAfx.h"
#include "dip.h"

PuresoftDepthInterpolationProcessor::PuresoftDepthInterpolationProcessor(void)
{
	if(NULL == (m_input = (float*)_aligned_malloc(sizeof(float) * 4, 16)))
	{
		throw bad_alloc("PuresoftDepthInterpolationProcessor::PuresoftDepthInterpolationProcessor");
	}

	m_input[3] = 0.0f;
}

PuresoftDepthInterpolationProcessor::~PuresoftDepthInterpolationProcessor(void)
{
	_aligned_free(m_input);
}

void PuresoftDepthInterpolationProcessor::setInputExt(int idx, const void* ext)
{
	m_input[idx] = *((const float*)ext);
}

void PuresoftDepthInterpolationProcessor::process(const void** outputExt, const float* contributes, const float* vert_z, float interp_z)
{
	m_output = mcemaths_dot_3_4(m_input, contributes);
	*outputExt = &m_output;
}
