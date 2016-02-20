#include <windows.h>
#include <math.h>
#include "mcemaths.hpp"
#include "testfp.h"
#include "samplr2d.h"
#include "bgra.h"

using namespace mcemaths;

template<typename T>
T opt_pow(T x, unsigned int n)
{
	T pw = (T)1;
	while (n > 0)
	{
		if (n & 1)
			pw *= x;
		x *= x;
		n >>= 1;
	}

	return pw;
}

void* _stdcall MyTestFragmentProcessor::createInstance(void)
{
	return new MyTestFragmentProcessor;
}

MyTestFragmentProcessor::MyTestFragmentProcessor(void)
{
}

MyTestFragmentProcessor::~MyTestFragmentProcessor(void)
{
}

void MyTestFragmentProcessor::release()
{
	delete this;
}

void MyTestFragmentProcessor::process(const FragmentProcessorInput* input, FragmentProcessorOutput* output, const void** uniforms, const void** textures)
{
	__declspec(align(16)) float outputColour[4];
	PURESOFTBGRA bytesColour;

	const float* lightPos = (const float*)uniforms[4];
	const float* cameraPos = (const float*)uniforms[5];
	int diffuse = *(const int*)uniforms[6];

	const MYTESTPROCDATA* inData = (const MYTESTPROCDATA*)input->user;
	((PuresoftSampler2D*)textures[diffuse])->get4(inData->texcoord[0], inData->texcoord[1], &bytesColour);
	outputColour[0] = bytesColour.elems.b;
	outputColour[1] = bytesColour.elems.g;
	outputColour[2] = bytesColour.elems.r;
	outputColour[3] = bytesColour.elems.a;

	vec4 L;
	mcemaths_sub_3_4(L, lightPos, inData->worldPos);
	float distance = mcemaths_len_3_4(L);
	mcemaths_div_3_4(L, distance);

	vec4 E;
	mcemaths_sub_3_4(E, cameraPos, inData->worldPos);
	mcemaths_norm_3_4(E);
	vec4 H;
	mcemaths_add_3_4(H, E, L);
	mcemaths_norm_3_4(H);

	float lambert = mcemaths_dot_3_4(L, inData->normal);

	float specular = mcemaths_dot_3_4(H, inData->normal);
	specular = max(specular, 0.0f);
	//specular = pow(specular, 50.0f);
	specular = opt_pow(specular, 50);


	mcemaths_add_1to4(outputColour, 255.0f * specular);
	mcemaths_mul_3_4(outputColour, lambert);
	mcemaths_clamp_3_4(outputColour, 0, 255.0f);
	bytesColour.elems.r = (unsigned char)outputColour[2];
	bytesColour.elems.g = (unsigned char)outputColour[1];
	bytesColour.elems.b = (unsigned char)outputColour[0];
	bytesColour.elems.a = 0;

	output->write(0, &bytesColour, sizeof(bytesColour));
}