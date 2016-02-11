#include <stdlib.h>
#include <memory.h>
#include <stdexcept>
#include "samplr2d.h"

PuresoftSampler2D::PuresoftSampler2D(unsigned int width, unsigned int scanline, unsigned int height, unsigned int elemLen, const void* buffer)
{
	m_buffer = (const char*)buffer;
	m_width = width;
	m_height = height;
	m_elemLen = elemLen;

	if(NULL == (m_rowEntries = (const char**)malloc(sizeof(char*) * height)))
	{
		throw std::bad_alloc("PuresoftSampler2D::PuresoftSampler2D");
	}

	for(unsigned int i = 0; i < height; i++)
	{
		m_rowEntries[i] = m_buffer + i * scanline;
	}
}

PuresoftSampler2D::~PuresoftSampler2D(void)
{
	free(m_rowEntries);
}

void PuresoftSampler2D::get(float texcoordX, float texcoordY, void* data, size_t len) const
{
	memcpy(data, locate(texcoordX, texcoordY), len);
}

void PuresoftSampler2D::get4(float texcoordX, float texcoordY, void* data) const
{
	*(unsigned int*)data = *(const unsigned int*)locate(texcoordX, texcoordY);
}

const char* PuresoftSampler2D::locate(float texcoordX, float texcoordY) const
{
	unsigned int x = (size_t)((float)m_width * texcoordX), y = (size_t)((float)m_height * texcoordY);
	if(x > m_width)
		x = m_width;
	if(y > m_height)
		y = m_height;
	return m_rowEntries[y] + x * m_elemLen;
}
