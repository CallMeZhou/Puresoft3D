#pragma once
#include "fbo.h"

class PuresoftSampler2D
{
public:
	static void get(const PuresoftFBO* imageBuffer, float texcoordX, float texcoordY, void* data, size_t len);
	static void get1(const PuresoftFBO* imageBuffer, float texcoordX, float texcoordY, void* data);
	static void get4(const PuresoftFBO* imageBuffer, float texcoordX, float texcoordY, void* data);
};

