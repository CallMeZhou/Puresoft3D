#include <atlbase.h>
#include <stdlib.h>
#include <memory.h>
#include "samplr2d.h"

PuresoftSampler2D::PuresoftSampler2D(unsigned int width, unsigned int scanline, unsigned int height, unsigned int elemLen, const void* buffer)
{
	m_locatingFactors[0] = (float)elemLen;
	m_locatingFactors[1] = (float)scanline;
	m_locatingFactors[2] = m_locatingFactors[3] = 0;

	m_rasterizingFactors[0] = (float)(width - 1);
	m_rasterizingFactors[1] = (float)(height - 1);
	m_rasterizingFactors[2] = m_rasterizingFactors[3] = 0;

	m_buffer = (const char*)buffer;
}

PuresoftSampler2D::~PuresoftSampler2D(void)
{
}

void PuresoftSampler2D::get(float texcoordX, float texcoordY, void* data, size_t len) const
{
	memcpy(data, m_buffer + locate(texcoordX, texcoordY), len);
}

void PuresoftSampler2D::get4(float texcoordX, float texcoordY, void* data) const
{
	*(unsigned int*)data = *(const unsigned int*)(m_buffer + locate(texcoordX, texcoordY));
}

size_t PuresoftSampler2D::locate(float texcoordX, float texcoordY) const
{
	__declspec(align(16)) float texcoords[4];

	texcoords[0] = texcoordX;
	texcoords[1] = texcoordY;
	texcoords[2] = texcoords[3] = 0;
	mcemaths_clamp_3_4(texcoords, 0, 1.0f);

	// X = texcoordX * m_width, Y = texcoordY * m_height
	mcemaths_mulvec_3_4(texcoords, m_rasterizingFactors);

	// remove fractional part (!must!)
	texcoords[0] = (float)(int)texcoords[0];
	texcoords[1] = (float)(int)texcoords[1];

	// locate = X * elemLen + Y * scanlineLen
	return (size_t)mcemaths_dot_3_4(texcoords, m_locatingFactors);
}
