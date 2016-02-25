#pragma once
#include "fbo.h"

class PuresoftSamplerCube
{
public:
	static void get(const PuresoftFBO* imageBuffer, const float* direction, void* data, size_t len);
	static void get1(const PuresoftFBO* imageBuffer, const float* direction, void* data);
	static void get4(const PuresoftFBO* imageBuffer, const float* direction, void* data);
	static void get16(const PuresoftFBO* imageBuffer, const float* direction, void* data);
};

