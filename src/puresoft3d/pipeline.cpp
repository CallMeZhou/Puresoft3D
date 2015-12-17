#include <stdlib.h>
#include <memory.h>
#include <stdexcept>
#include "mcemaths.h"
#include "pipeline.h"
#include "proc.h"
#include "interp.h"

using namespace std;

PuresoftPipeline::PuresoftPipeline(int width, int height)
	: m_width(width)
	, m_height(height)
	, m_rasterizer(width, height)
	//, m_dip(new PuresoftDepthInterpolationProcessor)
	, m_depth(width, ((int)(width / 4.0f + 0.5f) * 4) * sizeof(float), height, sizeof(float))
{
	memset(m_textures, 0, sizeof(m_textures));
	memset(m_uniforms, 0, sizeof(m_uniforms));
	memset(m_fbos, 0, sizeof(m_fbos));
	m_processor = NULL;
}

PuresoftPipeline::~PuresoftPipeline(void)
{
	for(int i = 0; i < MAX_UNIFORMS; i++)
	{
		if(m_uniforms[i])
		{
			_aligned_free(m_uniforms[i]);
		}
	}

//	delete m_dip;
}

void PuresoftPipeline::setTexture(int idx, void* sampler)
{
	if(idx < 0 || idx >= MAX_TEXTURES)
	{
		throw std::out_of_range("PuresoftPipeline::setTexture");
	}

	m_textures[idx] = sampler;
}

void PuresoftPipeline::setViewport()
{
}

void PuresoftPipeline::setProcessor(PuresoftProcessor* proc)
{
	m_processor = proc;
}

void PuresoftPipeline::setFBO(int idx, PuresoftFBO* fbo)
{
	if(idx < 0 || idx >= MAX_FBOS)
	{
		throw std::out_of_range("PuresoftPipeline::setFBO");
	}

	m_fbos[idx] = fbo;
}

void PuresoftPipeline::setUniform(int idx, const void* data, size_t len)
{
	if(idx < 0 || idx >= MAX_UNIFORMS)
	{
		throw std::out_of_range("PuresoftPipeline::setUniform");
	}
	
	if(m_uniforms[idx])
	{
		_aligned_free(m_uniforms[idx]);
	}

	if(NULL == (m_uniforms[idx] = _aligned_malloc(len, 16)))
	{
		throw bad_alloc("PuresoftPipeline::setUniform");
	}

	memcpy(m_uniforms[idx], data, len);
}

void PuresoftPipeline::drawVAO(PuresoftVAO* vao)
{
	if(!m_processor)
	{
		return;
	}

	const PuresoftRasterizer::RESULT* rasterResult = m_rasterizer.getResultPtr();

	// get pointers to all vbos, reset all data pointers
	PuresoftVBO** vbos = vao->getVBOs();
	vao->rewindAll();

	// input data structure for Vertex Processor
	PuresoftVertexProcessor::VertexProcessorInput vertInput;
	// output data structure for Vertex Processor
	PuresoftVertexProcessor::VertexProcessorOutput vertOutput[3];
	// input data structure for Fragment Processor
	PuresoftFragmentProcessor::FragmentProcessorInput fragInput;
	// output data structure for Vertex Processor
	PuresoftFragmentProcessor::FragmentProcessorOutput fragOuput;
	// this is for interpolation perspective correction, this is the divider
	__declspec(align(16)) float correctionFactor1[4] = {0};
	// projected Z coordinates (divided by W), collect them for interpolation, interpolate for depth
	__declspec(align(16)) float projZs[4] = {0};
	// vertex contribute values for left / right end of a scanline
	__declspec(align(16)) float contributesForLeft[4] = {0};
	__declspec(align(16)) float contributesForRight[4] = {0};

	// vbo processing start
	while(true)
	{
		// process 3 elements
		// input:     vbo
		// processor: Vertex Processor
		// output:    vertOutput
		// next step: rasterization
		for(int i = 0; i < 3; i++)
		{
			// collect element data from each available vbo
			for(size_t j = 0; j < PuresoftVAO::MAX_VBOS; j++)
			{
				if(vbos[j])
				{
					if(NULL == (vertInput.data[j] = vbos[j]->next()))
					{
						return;
					}
				}
			}

			// call Vertex Processor
			m_processor->m_vertProc->process(&vertInput, &vertOutput[i], (const void**)m_uniforms);

			// check Vertex Processor's output at Interpolation Processor
			// we don't touch Vertex Processor output except the 'position'
			// we use 'position' to do rasterization
			m_processor->m_interpProc->setInputExt(i, vertOutput[i].ext);

			// process the 'position'

			// homogenizing division (reciprocal W is negative reciprocal Z)
			float reciprocalW = 1.0f / vertOutput[i].position[3];
			mcemaths_mul_3_4(vertOutput[i].position, reciprocalW);

			// collect projected Zs, we'll interpolate it for depth later
			projZs[i] = vertOutput[i].position[2];

			// save -(1/Z) as one of the two factors for interpolation perspective correction
			// BTW, we do interpolation perspective correction in this way:
			// V_interp = z * (contibute_1 * V_1 / z + ... contibute_n * V_n / z)
			// we'll call '/z' correction factor 1 and 'z*' correction factor 2
			correctionFactor1[i] = reciprocalW;
		}

		// cull back face in naive way
 		__declspec(align(16)) float vec0[4], vec1[4], cross[4], test[4] = {0, 0, 1.0f, 0};
 		mcemaths_sub_3_4(vec0, vertOutput[1].position, vertOutput[0].position);
 		mcemaths_sub_3_4(vec1, vertOutput[2].position, vertOutput[1].position);
 		mcemaths_cross_3(cross, vec0, vec1);
 		mcemaths_norm_3_4(cross);
 		if(mcemaths_dot_3_4(test, cross) < 0)
 		{
 			continue;
 		}

		// do rasterization
		// output: const PuresoftRasterizer::RESULT* rasterResult
		m_rasterizer.pushTriangle(vertOutput[0].position, vertOutput[1].position, vertOutput[2].position);

		// process rasterization result, scanline by scanline

		int y = rasterResult->firstRow;

		// set current row to all attached fbos
		for(size_t i = 0; i < MAX_FBOS; i++)
		{
			if(m_fbos[i])
			{
				m_fbos[i]->setCurRow(y);
			}
		}

		m_depth.setCurRow(y);

		// for each scanline
		for(; y <= rasterResult->lastRow; y++)
		{
			int x = rasterResult->m_rows[y].left;

			// set starting column to all attached fbos
			for(size_t i = 0; i < MAX_FBOS; i++)
			{
				if(m_fbos[i])
				{
					m_fbos[i]->setCurCol(x);
				}
			}

			m_depth.setCurCol(x);

			// calculate vertex contributes for the first and last pixel of scanline
			PuresoftInterpolater::integerBasedLineSegmentlinearInterpolate((const int*)rasterResult->vertices, rasterResult->m_rows[y].leftVerts[0], rasterResult->m_rows[y].leftVerts[1], rasterResult->m_rows[y].left, y, contributesForLeft);
			PuresoftInterpolater::integerBasedLineSegmentlinearInterpolate((const int*)rasterResult->vertices, rasterResult->m_rows[y].rightVerts[0], rasterResult->m_rows[y].rightVerts[1], rasterResult->m_rows[y].right, y, contributesForRight);

			// calculate interpolated values for the first and last pixel of scanline
			// scanlineBegin calculates delta interpolated values as well
			m_processor->m_interpProc->scanlineBegin(rasterResult->m_rows[y].left, rasterResult->m_rows[y].right, y, contributesForLeft, contributesForRight, correctionFactor1);

			__declspec(align(16)) float projZsWithcorrectionFactor1[4];
			mcemaths_quatcpy(projZsWithcorrectionFactor1, projZs);
			mcemaths_mulvec_3_4(projZsWithcorrectionFactor1, correctionFactor1);
			float projZForLeft = mcemaths_dot_3_4(contributesForLeft, projZsWithcorrectionFactor1);
			float projZForRight = mcemaths_dot_3_4(contributesForRight, projZsWithcorrectionFactor1);
			float projZDelta = (projZForRight - projZForLeft) / (float)(rasterResult->m_rows[y].right == rasterResult->m_rows[y].left ? 1 : rasterResult->m_rows[y].right - rasterResult->m_rows[y].left);

			// process rasterization result of a scanline, column by column

			for(; x <= rasterResult->m_rows[y].right; x++)
			{
				// get interpolated values as well as the other perspective correction factor
				float correctionFactor2;
				m_processor->m_interpProc->scanlineNext(&correctionFactor2, &fragInput.ext);
				fragInput.position[0] = x;
				fragInput.position[1] = y;

				// interpolate for projected Z. we'll use it for depth
				float newDepth = projZForLeft * correctionFactor2;
				projZForLeft += projZDelta;

				// get current depth from the depth buffer and do depth test
				float currentDepth;
				m_depth.read4(&currentDepth);
				if(newDepth < currentDepth) // depth test passed
				{
					// update depth buffer
					m_depth.write4(&newDepth);
					m_depth.nextCol();

					// go ahead with Fragment Processor
					m_processor->m_fragProc->process(&fragInput, &fragOuput, (const void**)m_uniforms, (const void**)m_textures);

					// update each attached fbo with Fragment Processor output
					for(size_t i = 0; i < MAX_FBOS; i++)
					{
						if(m_fbos[i])
						{
							m_fbos[i]->write(fragOuput.data[i], fragOuput.dataSizes[i]);
							m_fbos[i]->nextCol();
						}
					}
				}
				else // depth test passed, skip everything of the current column
				{
					for(size_t i = 0; i < MAX_FBOS; i++)
					{
						if(m_fbos[i])
						{
							m_fbos[i]->nextCol();
						}
					}

					m_depth.nextCol();
				}
			}

			// for each attached fbo: go to next row
			for(size_t i = 0; i < MAX_FBOS; i++)
			{
				if(m_fbos[i])
				{
					m_fbos[i]->nextRow();
				}
			}

			m_depth.nextRow();
		}
	}
}

void PuresoftPipeline::clearDepth(float furthest /* = 1.0f */)
{
	__declspec(align(16)) float temp[4] = {furthest, furthest, furthest, furthest};

	m_depth.setCurRow(0);
	for(int y = 0; y < m_height; y++)
	{
		for(int x = 0; x < m_width; x+=4)
		{
			m_depth.write16(temp);
			m_depth.nextCol();
			m_depth.nextCol();
			m_depth.nextCol();
			m_depth.nextCol();
		}
		m_depth.nextRow();
	}
}
