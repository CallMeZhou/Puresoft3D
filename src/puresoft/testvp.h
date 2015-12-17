#pragma once
#include "proc.h"
#include "testpd.h"

class MyTestVertexProcesser :
	public PuresoftVertexProcessor
{
public:
	MyTestVertexProcesser(void);
	~MyTestVertexProcesser(void);

	void process(const VertexProcessorInput* input, VertexProcessorOutput* output, const void** uniforms);

private:
	MYTESTPROCDATA m_outputExt;
};

