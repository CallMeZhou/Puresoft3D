#include "samplr2d.h"

void PuresoftSampler2D::get(const PuresoftFBO* imageBuffer, float texcoordX, float texcoordY, void* data, size_t len)
{
	imageBuffer->directRead(
		(unsigned int)((float)imageBuffer->getHeight() * texcoordY + 0.5f), 
		(unsigned int)((float)imageBuffer->getWidth()  * texcoordX + 0.5f), 
		data, len);
}

void PuresoftSampler2D::get1(const PuresoftFBO* imageBuffer, float texcoordX, float texcoordY, void* data)
{
	imageBuffer->directRead1(
		(unsigned int)((float)imageBuffer->getHeight() * texcoordY + 0.5f), 
		(unsigned int)((float)imageBuffer->getWidth()  * texcoordX + 0.5f), 
		data);
}

void PuresoftSampler2D::get4(const PuresoftFBO* imageBuffer, float texcoordX, float texcoordY, void* data)
{
	imageBuffer->directRead4(
		(unsigned int)((float)imageBuffer->getHeight() * texcoordY + 0.5f), 
		(unsigned int)((float)imageBuffer->getWidth()  * texcoordX + 0.5f), 
		data);
}

void PuresoftSampler2D::get16(const PuresoftFBO* imageBuffer, float texcoordX, float texcoordY, void* data)
{
	imageBuffer->directRead16(
		(unsigned int)((float)imageBuffer->getHeight() * texcoordY + 0.5f), 
		(unsigned int)((float)imageBuffer->getWidth()  * texcoordX + 0.5f), 
		data);
}
