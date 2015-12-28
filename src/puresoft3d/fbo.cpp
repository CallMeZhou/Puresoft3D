#include <stdlib.h>
#include <stdexcept>
#include "mcemaths.h"
#include "fbo.h"

using namespace std;

PuresoftFBO::PuresoftFBO(unsigned int width, unsigned int scanline, unsigned int height, unsigned int elemLen)
{
	m_width = width;
	m_scanline = scanline;
	m_height = height;
	m_elemLen = elemLen;

	m_bytes = m_scanline * m_height;
	if(NULL == (m_buffer = _aligned_malloc(m_bytes, 64)))
	{
		throw bad_alloc("PuresoftFBO::PuresoftFBO");
	}

	for(size_t i = 0; i < MAX_WORKRANGES; i++)
	{
		m_workRanges[i].curRowEntry = 0;
		m_workRanges[i].writePoint = m_buffer;
	}
}


PuresoftFBO::~PuresoftFBO(void)
{
	_aligned_free(m_buffer);
}

void PuresoftFBO::setCurRow(int idx, unsigned int row)
{
	m_workRanges[idx].curRowEntry = row * m_scanline;
	m_workRanges[idx].writePoint = (char*)m_buffer + m_workRanges[idx].curRowEntry;
}

void PuresoftFBO::nextRow(int idx)
{
	m_workRanges[idx].curRowEntry += m_scanline;
	m_workRanges[idx].writePoint = (char*)m_buffer + m_workRanges[idx].curRowEntry;
}

void PuresoftFBO::setCurCol(int idx, unsigned int col)
{
	m_workRanges[idx].writePoint = (char*)m_buffer + m_workRanges[idx].curRowEntry + col * m_elemLen;
}

void PuresoftFBO::nextCol(int idx)
{
	m_workRanges[idx].writePoint = (char*)m_workRanges[idx].writePoint + m_elemLen;
}

void PuresoftFBO::read(int idx, void* data, size_t bytes) const
{
	memcpy(data, m_workRanges[idx].writePoint, bytes);
}

void PuresoftFBO::write(int idx, const void* data, size_t bytes)
{
	memcpy(m_workRanges[idx].writePoint, data, bytes);
}

void PuresoftFBO::read4(int idx, void* data) const
{
	*((unsigned int*)data) = *((const unsigned int*)m_workRanges[idx].writePoint);
}

void PuresoftFBO::write4(int idx, const void* data)
{
	*((unsigned int*)m_workRanges[idx].writePoint) = *((const unsigned int*)data);
}

void PuresoftFBO::read16(int idx, void* dataAligned16Bytes) const
{
	mcemaths_quatcpy((float*)dataAligned16Bytes, (const float*)m_workRanges[idx].writePoint);
}

void PuresoftFBO::write16(int idx, const void* dataAligned16Bytes)
{
	mcemaths_quatcpy((float*)m_workRanges[idx].writePoint, (const float*)dataAligned16Bytes);
}

void PuresoftFBO::clear(const void* data, size_t bytes)
{
	unsigned char* row = (unsigned char*)m_buffer;

	for(unsigned int y = 0; y < m_height - 1; y++)
	{
		unsigned char* col = (unsigned char*)row;
		for(unsigned int x = 0; x < m_width; x++)
		{
			memcpy(col, data, bytes);
			col += m_elemLen;
		}
		row += m_scanline;
	}
}

void PuresoftFBO::clear4(const void* data)
{
	unsigned char* row = (unsigned char*)m_buffer;

	for(unsigned int y = 0; y < m_height - 1; y++)
	{
		unsigned char* col = (unsigned char*)row;
		for(unsigned int x = 0; x < m_width; x++)
		{
			*((unsigned int*)col) = *((unsigned int*)data);
			col += m_elemLen;
		}
		row += m_scanline;
	}
}

void PuresoftFBO::clear16(const void* dataAligned16Bytes)
{
	// clear16() requires every scanline starts at 16-byte boundary meaning the buffer length is a multiple of 16
	__asm{
		; load source data
		mov		eax,	dataAligned16Bytes
		movaps	xmm0,	[eax]
		; find loop times
		mov		eax,	this
		add		eax,	m_bytes
		mov		ecx,	[eax]
		shr		ecx,	4		; div 16
		; load dest address
		mov		eax,	this
		add		eax,	m_buffer
		mov		edx,	[eax]
		; fill buffer in sse way
lup:	movaps	[edx],	xmm0
		add		edx,	16
		dec		ecx
		jnz		lup
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