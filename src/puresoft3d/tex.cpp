#include <stdlib.h>
#include "pipeline.h"

int PuresoftPipeline::createTexture(unsigned int width, unsigned int scanline, unsigned int height, unsigned int elemLen, void* content)
{
	size_t vacant = 0;
	for(; vacant < m_texPool.size(); vacant++)
	{
		if(!m_texPool[vacant])
			break;
	}

	if(m_texPool.size() == vacant)
	{
		m_texPool.push_back(NULL);
	}

	PuresoftFBO* fbo = new PuresoftFBO(width, scanline, height, elemLen, true);
	memcpy(fbo->getBuffer(), content, fbo->getBytes());
	m_texPool[vacant] = fbo;

	return (int)vacant;
}

void PuresoftPipeline::destroyTexture(int idx)
{
	if(idx < 0 || idx >= (int)m_texPool.size())
	{
		throw std::out_of_range("PuresoftPipeline::destroyTexture");
	}

	if(m_texPool[idx])
	{
		delete m_texPool[idx];
		m_texPool[idx] = NULL;
	}
}