#include "mcemaths.h"
#include "samplrproj.h"

ALIGN16 static const float bias[16] = 
{
	0.5f,    0,    0,    0, 
	   0, 0.5f,    0,    0, 
	   0,    0, 0.5f,    0, 
	0.5f, 0.5f, 0.5f,    0
};

static void texcoordFromProjection(float* texcoord, const float* position, const float* projection)
{
	mcemaths_transform_m4v4(texcoord, projection, position);
	mcemaths_div_3_4(texcoord, texcoord[3]);
	mcemaths_transform_m4v4_ip(texcoord, bias);
}

void PuresoftSamplerProjection::get(const PuresoftFBO* imageBuffer, const float* position, const float* projection, void* data, size_t len)
{
	ALIGN16 float texcoord[4];
	texcoordFromProjection(texcoord, position, projection);

	imageBuffer->directRead(
		(unsigned int)((float)imageBuffer->getHeight() * texcoord[1] + 0.5f), 
		(unsigned int)((float)imageBuffer->getWidth()  * texcoord[0] + 0.5f), 
		data, len);
}

void PuresoftSamplerProjection::get1(const PuresoftFBO* imageBuffer, const float* position, const float* projection, void* data)
{
	ALIGN16 float texcoord[4];
	texcoordFromProjection(texcoord, position, projection);

	imageBuffer->directRead1(
		(unsigned int)((float)imageBuffer->getHeight() * texcoord[1] + 0.5f), 
		(unsigned int)((float)imageBuffer->getWidth()  * texcoord[0] + 0.5f), 
		data);
}

void PuresoftSamplerProjection::get4(const PuresoftFBO* imageBuffer, const float* position, const float* projection, void* data)
{
	ALIGN16 float texcoord[4];
	texcoordFromProjection(texcoord, position, projection);

	imageBuffer->directRead4(
		(unsigned int)((float)imageBuffer->getHeight() * texcoord[1] + 0.5f), 
		(unsigned int)((float)imageBuffer->getWidth()  * texcoord[0] + 0.5f), 
		data);
}

void PuresoftSamplerProjection::get16(const PuresoftFBO* imageBuffer, const float* position, const float* projection, void* data)
{
	ALIGN16 float texcoord[4];
	texcoordFromProjection(texcoord, position, projection);

	imageBuffer->directRead16(
		(unsigned int)((float)imageBuffer->getHeight() * texcoord[1] + 0.5f), 
		(unsigned int)((float)imageBuffer->getWidth()  * texcoord[0] + 0.5f), 
		data);
}