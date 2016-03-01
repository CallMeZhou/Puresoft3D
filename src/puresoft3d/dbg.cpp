#include "pipeline.h"

void PuresoftPipeline::saveTexture(int idx, const wchar_t* path, bool dataIsFloat)
{
	if(-1 == idx)
	{
		m_depth->saveAsBmpFile(path, dataIsFloat);
	}
	else
	{
		if(idx >= (int)m_texPool.size())
		{
			throw std::out_of_range("PuresoftPipeline::saveTexture");
		}

		m_texPool[idx]->saveAsBmpFile(path, dataIsFloat);
	}
}
