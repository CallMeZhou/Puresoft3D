#include <memory.h>
#include <stdexcept>
#include <assert.h>
#include "vao.h"

using namespace std;

PuresoftVAO::PuresoftVAO(void)
{
	memset(m_vbos, 0, sizeof(m_vbos));
}

PuresoftVAO::~PuresoftVAO(void)
{
}

PuresoftVBO* PuresoftVAO::attachVBO(unsigned int idx, PuresoftVBO* vbo)
{
	assert(idx < MAX_VBOS);

	PuresoftVBO* old = m_vbos[idx];
	m_vbos[idx] = vbo;
	return old;
}

PuresoftVBO* PuresoftVAO::detachVBO(unsigned int idx)
{
	assert(idx < MAX_VBOS);

	PuresoftVBO* old = m_vbos[idx];
	m_vbos[idx] = NULL;
	return old;
}

void PuresoftVAO::rewindAll(void)
{
	PuresoftVBO** p = m_vbos;
	for(size_t i = 0; i < MAX_VBOS; i++, p++)
	{
		if(*p)
		{
			(*p)->rewindRanges();
		}
	}
}

PuresoftVBO* PuresoftVAO::getVBO(unsigned int idx)
{
	assert(idx < MAX_VBOS);

	return m_vbos[idx];
}

PuresoftVBO** PuresoftVAO::getVBOs(void)
{
	return m_vbos;
}
