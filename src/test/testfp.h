#pragma once
#include "proc.h"
#include "testpd.h"

class MyTestFragmentProcessor :
	public PuresoftFragmentProcessor
{
public:
	static void* _stdcall createInstance(void);

public:
	MyTestFragmentProcessor(void);
	~MyTestFragmentProcessor(void);

	void release(void);
	void process(const FragmentProcessorInput* input, FragmentProcessorOutput* output, const void** uniforms, const void** textures);

private:
	RGBQUAD m_singleColour;
};

