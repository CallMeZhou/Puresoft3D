#include <stdlib.h>
#include <stdexcept>
#include <assert.h>
#include "vbo.h"

using namespace std;

PuresoftVBO::PuresoftVBO(size_t unitBytes, size_t unitCount)
{
	m_unitBytes = unitBytes;
	m_unitCount = unitCount;
	m_bufferBytes = unitBytes * unitCount;

	if(NULL == (m_buffer = _aligned_malloc(m_bufferBytes, 16)))
	{
		throw bad_alloc();
	}

	rewindRanges(-1);
}


PuresoftVBO::~PuresoftVBO(void)
{
	_aligned_free(m_buffer);
}

void PuresoftVBO::updateContent(const void* src)
{
	memcpy(m_buffer, src, m_bufferBytes);
}

void PuresoftVBO::rewindRanges(int idx /* = -1 */)
{
	if(idx < 0)
	{
		for(size_t i = 0; i < MAX_WORKRANGES; i++)
		{
			m_workRanges[i].current = (unsigned char*)m_buffer;
			m_workRanges[i].end = (unsigned char*)m_buffer + m_bufferBytes;
		}
	}
	else
	{
		assert(idx < MAX_WORKRANGES);
		m_workRanges[idx].current = (unsigned char*)m_buffer;
		m_workRanges[idx].end = (unsigned char*)m_buffer + m_bufferBytes;
	}
}

void PuresoftVBO::evenOutRanges(int rangeAmount /* = MAX_WORKRANGES */)
{
	assert((0 < rangeAmount) && (rangeAmount < MAX_WORKRANGES));

	size_t unitsPerRange = m_unitCount / rangeAmount;
	size_t bytesPerRange = unitsPerRange * m_unitBytes;

	m_workRanges[0].current = (unsigned char*)m_buffer;
	m_workRanges[0].end = (unsigned char*)m_buffer + bytesPerRange;
	for(int i = 1; i < rangeAmount; i++)
	{
		m_workRanges[i].current = m_workRanges[i - 1].end;
		m_workRanges[i].end = m_workRanges[i].current + bytesPerRange;
	}

	m_workRanges[rangeAmount - 1].end = (unsigned char*)m_buffer + m_bufferBytes;
}

const void* PuresoftVBO::next(size_t idx /* = 0 */)
{
	assert((0 <= idx) && (idx < MAX_WORKRANGES));

	WORKRANGE& range = m_workRanges[idx];

	if(range.current >= range.end)
	{
		return NULL;
	}

	const void* ret = range.current;
	range.current += m_unitBytes;

	return ret;
}