#include "mcemaths.h"
#include "samplrproj.h"

ALIGN16 static const float bias[16] = 
{
	0.5f,    0,    0,    0, 
	   0, 0.5f,    0,    0, 
	   0,    0, 1.0f,    0, 
	0.5f, 0.5f,    0,    0
};

static void texcoordFromProjection(float* texcoord, const float* position, const float* projection)
{
	mcemaths_transform_m4v4(texcoord, projection, position);
	mcemaths_div_3_4(texcoord, texcoord[3]);
	mcemaths_transform_m4v4_ip(texcoord, bias);
}

float PuresoftSamplerProjection::get(const PuresoftFBO* imageBuffer, const float* position, const float* projection)
{
	ALIGN16 float texcoord[4], position_horm[4];
	mcemaths_quatcpy(position_horm, position);
	position_horm[3] = 1.0f;
	texcoordFromProjection(texcoord, position_horm, projection);

	float depthInShadowMap, shadowFactor = 0;
	unsigned int y_cntr = (unsigned int)((float)imageBuffer->getHeight() * texcoord[1] + 0.5f);
	unsigned int x_cntr = (unsigned int)((float)imageBuffer->getWidth() * texcoord[0] + 0.5f);

	for(unsigned int y = y_cntr - 2; y <= y_cntr + 2; y++)
	{
		for(unsigned int x = x_cntr - 2; x <= x_cntr + 2; x++)
		{
			imageBuffer->directRead4(y, x, &depthInShadowMap);
			shadowFactor += depthInShadowMap > texcoord[2] ? 1.0f : 0.15f;
		}
	}

	// the bigger the further from light source
	return shadowFactor / 25.0f;
}
/*
void PuresoftSamplerProjection::get1(const PuresoftFBO* imageBuffer, const float* position, const float* projection, void* data)
{
	ALIGN16 float texcoord[4], position_horm[4];
	mcemaths_quatcpy(position_horm, position);
	position_horm[3] = 1.0f;
	texcoordFromProjection(texcoord, position_horm, projection);

	imageBuffer->directRead1(
		(unsigned int)((float)imageBuffer->getHeight() * texcoord[1] + 0.5f), 
		(unsigned int)((float)imageBuffer->getWidth()  * texcoord[0] + 0.5f), 
		data);
}

void PuresoftSamplerProjection::get4(const PuresoftFBO* imageBuffer, const float* position, const float* projection, void* data)
{
	ALIGN16 float texcoord[4], position_horm[4];
	mcemaths_quatcpy(position_horm, position);
	position_horm[3] = 1.0f;
	texcoordFromProjection(texcoord, position_horm, projection);

	imageBuffer->directRead4(
		(unsigned int)((float)imageBuffer->getHeight() * texcoord[1] + 0.5f), 
		(unsigned int)((float)imageBuffer->getWidth()  * texcoord[0] + 0.5f), 
		data);
}

void PuresoftSamplerProjection::get16(const PuresoftFBO* imageBuffer, const float* position, const float* projection, void* data)
{
	ALIGN16 float texcoord[4], position_horm[4];
	mcemaths_quatcpy(position_horm, position);
	position_horm[3] = 1.0f;
	texcoordFromProjection(texcoord, position_horm, projection);

	imageBuffer->directRead16(
		(unsigned int)((float)imageBuffer->getHeight() * texcoord[1] + 0.5f), 
		(unsigned int)((float)imageBuffer->getWidth()  * texcoord[0] + 0.5f), 
		data);
}*/