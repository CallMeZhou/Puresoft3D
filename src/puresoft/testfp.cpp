#include "StdAfx.h"
#include "testfp.h"
#include "samplr2d.h"

MyTestFragmentProcessor::MyTestFragmentProcessor(void)
{
}

MyTestFragmentProcessor::~MyTestFragmentProcessor(void)
{
}

const RGBQUAD ambience = {10, 10, 10, 0};
void MyTestFragmentProcessor::process(const FragmentProcessorInput* input, FragmentProcessorOutput* output, const void** uniforms, const void** textures)
{
	static RGBQUAD singleColour;

	//singleColour = *(const RGBQUAD*)uniforms[3];
	const float* lightPos = (const float*)uniforms[4];
	const float* cameraPos = (const float*)uniforms[5];
	int diffuse = *(const int*)uniforms[6];

	const MYTESTPROCDATA* inData = (const MYTESTPROCDATA*)input->ext;
	((PuresoftSampler2D*)textures[diffuse])->get4(inData->texcoord[0], inData->texcoord[1], &singleColour);

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

	int red = singleColour.rgbRed * lambert + singleColour.rgbRed * specular + ambience.rgbRed;
	int green = singleColour.rgbGreen * lambert + singleColour.rgbGreen * specular + ambience.rgbGreen;
	int blue = singleColour.rgbBlue * lambert + singleColour.rgbBlue * specular + ambience.rgbBlue;

	singleColour.rgbRed = min(red, 255);
	singleColour.rgbGreen = min(green, 255);
	singleColour.rgbBlue = min(blue, 255);

	output->data[0] = &singleColour;
	output->dataSizes[0] = sizeof(RGBQUAD);
}
