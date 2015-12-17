#pragma once
#include "vao.h"
#include "fbo.h"
#include "rasterizer.h"

class PuresoftProcessor;
class PuresoftInterpolationProcessor;
class PuresoftPipeline
{
public:
	static const size_t MAX_UNIFORMS = 16;
	static const size_t MAX_FBOS = 16;
	static const size_t MAX_TEXTURES = 16;

public:
	PuresoftPipeline(int width, int height);
	~PuresoftPipeline(void);

	// resource
	void setTexture(int idx, void* sampler);

	// draw
	void setViewport();
	void setProcessor(PuresoftProcessor* proc);
	void setFBO(int idx, PuresoftFBO* fbo);
	void setUniform(int idx, const void* data, size_t len);
	void drawVAO(PuresoftVAO* vao);

	// clear series
	void clearDepth(float furthest = 1.0f);

private:
	int m_width;
	int m_height;
	void* m_textures[MAX_TEXTURES];
	void* m_uniforms[MAX_UNIFORMS];
	PuresoftProcessor* m_processor;
	//PuresoftInterpolationProcessor* m_dip;
	PuresoftRasterizer m_rasterizer;
	PuresoftFBO* m_fbos[MAX_FBOS];
	PuresoftFBO m_depth;
};

