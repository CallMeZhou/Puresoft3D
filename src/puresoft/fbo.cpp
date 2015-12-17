#include "StdAfx.h"
#include "fbo.h"

PuresoftFBO::PuresoftFBO(unsigned int width, unsigned int scanline, unsigned int height, unsigned int elemLen)
{
	m_width = width;
	m_scanline = scanline;
	m_height = height;
	m_elemLen = elemLen;

	m_bytes = m_scanline * m_height;
	if(NULL == (m_buffer = _aligned_malloc(m_bytes, 16)))
	{
		throw bad_alloc("PuresoftFBO::PuresoftFBO");
	}

	m_writePoint = m_buffer;
}


PuresoftFBO::~PuresoftFBO(void)
{
	_aligned_free(m_buffer);
}

void PuresoftFBO::setCurRow(unsigned int row)
{
	if(row >= m_height)
	{
		throw std::out_of_range("PuresoftFBO::setCurRow");
	}

	m_curRowEntry = row * m_scanline;
	m_writePoint = (char*)m_buffer + m_curRowEntry;
}

void PuresoftFBO::nextRow(void)
{
	m_curRowEntry += m_scanline;
	m_writePoint = (char*)m_buffer + m_curRowEntry;
}

void PuresoftFBO::setCurCol(unsigned int col)
{
	m_writePoint = (char*)m_buffer + m_curRowEntry + col * m_elemLen;
}

void PuresoftFBO::nextCol(void)
{
	m_writePoint = (char*)m_writePoint + m_elemLen;
}

void PuresoftFBO::read(void* data, size_t bytes) const
{
	memcpy(data, m_writePoint, bytes);
}

void PuresoftFBO::write(const void* data, size_t bytes)
{
	memcpy(m_writePoint, data, bytes);
}

void PuresoftFBO::read4(void* data) const
{
	*((unsigned int*)data) = *((const unsigned int*)m_writePoint);
}

void PuresoftFBO::write4(const void* data)
{
	*((unsigned int*)m_writePoint) = *((const unsigned int*)data);
}

void PuresoftFBO::read16(void* dataAligned16Bytes) const
{
	mcemaths_quatcpy((float*)dataAligned16Bytes, (const float*)m_writePoint);
}

void PuresoftFBO::write16(const void* dataAligned16Bytes)
{
	mcemaths_quatcpy((float*)m_writePoint, (const float*)dataAligned16Bytes);
}

void PuresoftFBO::clear(const void* data, size_t bytes)
{
	setCurRow(0);

	for(unsigned int y = 0; y < m_height - 1; y++)
	{
		for(unsigned int x = 0; x < m_width; x++)
		{
			write(data, bytes);
			nextCol();
		}
		nextRow();
	}
}

void PuresoftFBO::clear4(const void* data)
{
	setCurRow(0);

	for(unsigned int y = 0; y < m_height - 1; y++)
	{
		for(unsigned int x = 0; x < m_width; x++)
		{
			write4(data);
			nextCol();
		}
		nextRow();
	}
}

void PuresoftFBO::clear16(const void* dataAligned16Bytes)
{
	setCurRow(0);

	for(unsigned int y = 0; y < m_height - 1; y++)
	{
		for(unsigned int x = 0; x < m_width; x++)
		{
			write16(dataAligned16Bytes);
			nextCol();
		}
		nextRow();
	}
}

void* PuresoftFBO::getBuffer(void)
{
	return m_buffer;
}

const void* PuresoftFBO::getBuffer(void) const
{
	return m_buffer;
}

unsigned int PuresoftFBO::getWidth(void) const
{
	return m_width;
}

unsigned int PuresoftFBO::getHeight(void) const
{
	return m_height;
}

unsigned int PuresoftFBO::getScanline(void) const
{
	return m_scanline;
}

unsigned int PuresoftFBO::getElemLen(void) const
{
	return m_elemLen;
}