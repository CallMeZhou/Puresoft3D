#include <windows.h>
#include <math.h>
#include "mcemaths.hpp"
#include "testfp.h"
#include "samplr2d.h"

using namespace mcemaths;

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

RGBQUAD ambience = {10, 10, 10, 0};
void MyTestFragmentProcessor::process(const FragmentProcessorInput* input, FragmentProcessorOutput* output, const void** uniforms, const void** textures)
{
	RGBQUAD outputColour;

	const float* lightPos = (const float*)uniforms[4];
	const float* cameraPos = (const float*)uniforms[5];
	int diffuse = *(const int*)uniforms[6];

	const MYTESTPROCDATA* inData = (const MYTESTPROCDATA*)input->user;

	PuresoftSampler2D::get4((const PuresoftFBO*)textures[diffuse], inData->texcoord[1], inData->texcoord[0], &outputColour);

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
	lambert = max(lambert, 0.0f);

	float specular = mcemaths_dot_3_4(H, inData->normal);
	specular = max(specular, 0.0f);
	specular = pow(specular, 50.0f);

	int specularColour = (int)(255.0f * specular * 0.5f);

	int red = (outputColour.rgbRed + specularColour) * lambert + outputColour.rgbRed / 10;
	int green = (outputColour.rgbGreen + specularColour) * lambert + outputColour.rgbGreen / 10;
	int blue = (outputColour.rgbBlue + specularColour) * lambert + outputColour.rgbBlue / 10;

	outputColour.rgbRed = min(red, 255);
	outputColour.rgbGreen = min(green, 255);
	outputColour.rgbBlue = min(blue, 255);

	output->write(0, &outputColour, sizeof(outputColour));
}
