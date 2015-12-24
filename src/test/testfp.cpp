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

const RGBQUAD ambience = {10, 10, 10, 0};
void MyTestFragmentProcessor::process(const FragmentProcessorInput* input, FragmentProcessorOutput* output, const void** uniforms, const void** textures)
{
	const float* lightPos = (const float*)uniforms[4];
	const float* cameraPos = (const float*)uniforms[5];
	int diffuse = *(const int*)uniforms[6];

	const MYTESTPROCDATA* inData = (const MYTESTPROCDATA*)input->ext;
	((PuresoftSampler2D*)textures[diffuse])->get4(inData->texcoord[0], inData->texcoord[1], &m_singleColour);

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
	if(lambert < 0.2f)
	{
		lambert = 0.2f;
	}

	float specular = mcemaths_dot_3_4(H, inData->normal);
	if(specular < 0)
	{
		specular = 0;
	}
	specular = pow(specular, 6.0f);

	int red = m_singleColour.rgbRed * lambert + m_singleColour.rgbRed * specular + ambience.rgbRed;
	int green = m_singleColour.rgbGreen * lambert + m_singleColour.rgbGreen * specular + ambience.rgbGreen;
	int blue = m_singleColour.rgbBlue * lambert + m_singleColour.rgbBlue * specular + ambience.rgbBlue;

	m_singleColour.rgbRed = min(red, 255);
	m_singleColour.rgbGreen = min(green, 255);
	m_singleColour.rgbBlue = min(blue, 255);

	output->data[0] = &m_singleColour;
	output->dataSizes[0] = sizeof(RGBQUAD);
}
