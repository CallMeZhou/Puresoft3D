#pragma once
#include "fbo.h"

class PuresoftSamplerProjection
{
public:
	static float get(const PuresoftFBO* imageBuffer, const float* projection);
};