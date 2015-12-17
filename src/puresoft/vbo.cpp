#include "StdAfx.h"
#include "vbo.h"

PuresoftVBO::PuresoftVBO(size_t unitBytes, size_t unitCount)
{
	m_unitBytes = unitBytes;
	m_bufferBytes = m_unitBytes * unitCount;

	if(NULL == (m_buffer = _aligned_malloc(m_bufferBytes, 16)))
	{
		throw bad_alloc("PuresoftVBO::updateContent");
	}

	m_currentPosition = (unsigned char*)m_buffer;
	m_endPosition = m_currentPosition + m_bufferBytes;
}


PuresoftVBO::~PuresoftVBO(void)
{
	_aligned_free(m_buffer);
}

void PuresoftVBO::updateContent(const void* src)
{
	memcpy(m_buffer, src, m_bufferBytes);
}

void PuresoftVBO::rewind(void)
{
	m_currentPosition = (unsigned char*)m_buffer;
}

const void* PuresoftVBO::next(void)
{
	if(m_currentPosition >= m_endPosition)
	{
		return NULL;
	}

	const void* ret = m_currentPosition;
	m_currentPosition += m_unitBytes;

	return ret;
}