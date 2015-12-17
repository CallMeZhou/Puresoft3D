#include "StdAfx.h"
#include "vao.h"

PuresoftVAO::PuresoftVAO(void)
{
	memset(m_vbos, 0, sizeof(m_vbos));
}


PuresoftVAO::~PuresoftVAO(void)
{
}

PuresoftVBO* PuresoftVAO::attachVBO(unsigned int idx, PuresoftVBO* vbo)
{
	if(idx >= MAX_VBOS)
	{
		throw new std::out_of_range("attachVBO");
	}

	PuresoftVBO* old = m_vbos[idx];
	m_vbos[idx] = vbo;
	return old;
}

PuresoftVBO* PuresoftVAO::detachVBO(unsigned int idx)
{
	if(idx >= MAX_VBOS)
	{
		throw new std::out_of_range("detachVBO");
	}

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
			(*p)->rewind();
		}
	}
}

PuresoftVBO* PuresoftVAO::getVBO(unsigned int idx)
{
	if(idx >= MAX_VBOS)
	{
		throw new std::out_of_range("getVBO");
	}

	return m_vbos[idx];
}

PuresoftVBO** PuresoftVAO::getVBOs(void)
{
	return m_vbos;
}
