#pragma once
#include "proc.h"

class PP_DepthofField : public PuresoftPostProcessor
{
public:
	void process(int threadIndex, int threadCount, PuresoftFBO* frame, PuresoftFBO* depth);
};