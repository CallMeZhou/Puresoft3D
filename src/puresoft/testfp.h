#pragma once
#include "proc.h"
#include "testpd.h"

class MyTestFragmentProcessor :
	public PuresoftFragmentProcessor
{
public:
	MyTestFragmentProcessor(void);
	~MyTestFragmentProcessor(void);

	void process(const FragmentProcessorInput* input, FragmentProcessorOutput* output, const void** uniforms, const void** textures);
};

