#pragma once
#include "fbo.h"

class PuresoftSamplerProjection
{
public:
	static void get(const PuresoftFBO* imageBuffer, const float* position, const float* projection, void* data, size_t len);
	static void get1(const PuresoftFBO* imageBuffer, const float* position, const float* projection, void* data);
	static void get4(const PuresoftFBO* imageBuffer, const float* position, const float* projection, void* data);
	static void get16(const PuresoftFBO* imageBuffer, const float* position, const float* projection, void* data);
};