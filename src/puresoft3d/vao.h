#pragma once
#include "config.h"
#include "vbo.h"

class PuresoftVAO
{
public:
	PuresoftVAO(void);
	~PuresoftVAO(void);

	PuresoftVBO* attachVBO(unsigned int idx, PuresoftVBO* vbo);
	PuresoftVBO* detachVBO(unsigned int idx);

	void rewindAll(void);
	PuresoftVBO* getVBO(unsigned int idx);
	PuresoftVBO** getVBOs(void);

private:
	PuresoftVBO* m_vbos[MAX_VBOS];
};

