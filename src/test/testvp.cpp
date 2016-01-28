#include <memory.h>
#include "mcemaths.h"
#include "testvp.h"

void* _stdcall MyTestVertexProcesser::createInstance(void)
{
	return new MyTestVertexProcesser;
}

MyTestVertexProcesser::MyTestVertexProcesser(void)
{
}

MyTestVertexProcesser::~MyTestVertexProcesser(void)
{
}

void MyTestVertexProcesser::release(void)
{
	delete this;
}

void MyTestVertexProcesser::process(const VertexProcessorInput* input, VertexProcessorOutput* output, const void** uniforms)
{
	MYTESTPROCDATA* userOutput = (MYTESTPROCDATA*)output->user;

	const float* position = (const float*)input->data[0];
	const float* normals = (const float*)input->data[1];
	const float* texcoord = (const float*)input->data[2];

	const float* proj_view = (const float*)uniforms[0];
	const float* model = (const float*)uniforms[1];
	const float* modelRotate = (const float*)uniforms[2];

	mcemaths_transform_m4v4(userOutput->worldPos, model, position);

	mcemaths_quatcpy(output->position, userOutput->worldPos);
	userOutput->worldPos[3] = 0;

	mcemaths_transform_m4v4_ip(output->position, proj_view);
	//output->position[3] = 1.0f;

	mcemaths_transform_m4v4(userOutput->normal, modelRotate, normals);

	userOutput->texcoord[0] = texcoord[0];
	userOutput->texcoord[1] = texcoord[1];
}
