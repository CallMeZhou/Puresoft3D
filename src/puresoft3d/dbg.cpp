#include "pipeline.h"

void PuresoftPipeline::saveTexture(int idx, const wchar_t* path, bool dataIsFloat)
{
	if(-1 == idx)
	{
		m_depth->saveAsBmpFile(path, dataIsFloat);
	}
	else if(-2 == idx)
	{
		m_display->saveAsBmpFile(path, dataIsFloat);
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

void PuresoftPipeline::saveTextureAsRaw(int idx, const wchar_t* path)
{
	if(-1 == idx)
	{
		m_depth->saveAsRawFile(path);
	}
	else if(-2 == idx)
	{
		m_display->saveAsRawFile(path);
	}
	else
	{
		if(idx >= (int)m_texPool.size())
		{
			throw std::out_of_range("PuresoftPipeline::saveTextureAsRaw");
		}

		m_texPool[idx]->saveAsRawFile(path);
	}
}
