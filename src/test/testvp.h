#pragma once
#include "proc.h"
#include "testpd.h"

class MyTestVertexProcesser :
	public PuresoftVertexProcessor
{
public:
	static void* _stdcall createInstance(void);

public:
	MyTestVertexProcesser(void);
	~MyTestVertexProcesser(void);

	void release(void);
	void process(const VertexProcessorInput* input, VertexProcessorOutput* output, const void** uniforms);
};

