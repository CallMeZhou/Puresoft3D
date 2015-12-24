#include <memory.h>
#include "mcemaths.h"
#include "testvp.h"

void* _stdcall MyTestVertexProcesser::createInstance(void)
{
	return new MyTestVertexProcesser;
}

MyTestVertexProcesser::MyTestVertexProcesser(void)
{
	memset(&m_outputExt, 0, sizeof(m_outputExt));
}

MyTestVertexProcesser::~MyTestVertexProcesser(void)
{
}

void MyTestVertexProcesser::release(void)
{
	delete this;
}

void MyTestVertexProcesser::process(const PuresoftVertexProcessor::VertexProcessorInput* input, PuresoftVertexProcessor::VertexProcessorOutput* output, const void** uniforms)
{
	const float* position = (const float*)input->data[0];
	const float* normals = (const float*)input->data[1];
	const float* texcoord = (const float*)input->data[2];

	const float* proj_view = (const float*)uniforms[0];
	const float* model = (const float*)uniforms[1];
	const float* modelRotate = (const float*)uniforms[2];

	mcemaths_transform_m4v4(m_outputExt.worldPos, model, position);

	mcemaths_quatcpy(output->position, m_outputExt.worldPos);
	m_outputExt.worldPos[3] = 0;

	mcemaths_transform_m4v4_ip(output->position, proj_view);

	mcemaths_transform_m4v4(m_outputExt.normal, modelRotate, normals);

	m_outputExt.texcoord[0] = texcoord[0];
	m_outputExt.texcoord[1] = texcoord[1];

	output->ext = &m_outputExt;
}
