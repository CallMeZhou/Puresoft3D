#include "mcemaths.h"
#include "samplrproj.h"

const float POISSON_DISK_X[4] = 
{
	-0.94201624f,
	 0.94558609f, 
	-0.094184101f,
	 0.34495938f
};

const float POISSON_DISK_Y[4] = 
{
	-0.39906216f,
	-0.76890725f,
	-0.92938870f,
	 0.29387760f
};

float PuresoftSamplerProjection::get(const PuresoftFBO* imageBuffer, const float* projection)
{
	ALIGN16 float texcoord[4];
	mcemaths_quatcpy(texcoord, projection);
	mcemaths_div_3_4(texcoord, texcoord[3]);

	float depthInShadowMap, shadowFactor = 1.0f;
	for(int i = 0; i < 4; i++)
	{
		unsigned int y = (unsigned int)((float)imageBuffer->getHeight() * (texcoord[1] + POISSON_DISK_X[i] / 700.0f));
		unsigned int x = (unsigned int)((float)imageBuffer->getWidth()  * (texcoord[0] + POISSON_DISK_Y[i] / 700.0f));
		imageBuffer->directRead4(y, x, &depthInShadowMap);
		if(depthInShadowMap < texcoord[2])
			shadowFactor -=  0.2f;
	}

	// the bigger the further from light source
	return shadowFactor;
}