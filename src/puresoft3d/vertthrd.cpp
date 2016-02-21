#include "pipeline.h"

bool PuresoftPipeline::processVertices(PuresoftVBO** vbos, VertexProcessorOutput* output, float* reciprocalWs, float* projectedZs)
{
	// vertex processor input
	VertexProcessorInput vertInput;

	for(int i = 0; i < 3; i++)
	{
		// collect element data from each available vbo
		for(size_t j = 0; j < MAX_VBOS; j++)
		{
			if(vbos[j])
			{
				if(NULL == (vertInput.data[j] = vbos[j]->next(0)))
				{
					// done with all vertices
					return false;
				}
			}
		}

		// call Vertex Processor
		m_vp->process(&vertInput, &output[i]);

		// homogenizing division (reciprocal W is negative reciprocal Z)
		float reciprocalW = 1.0f / output[i].position[3];
		mcemaths_mul_3_4(output[i].position, reciprocalW);

		// collect projected Zs, we'll interpolate it for depth later
		projectedZs[i] = output[i].position[2];

		// save -(1/Z) as one of the two factors for interpolation perspective correction
		// BTW, we do interpolation perspective correction in this way:
		// V_interp = Z_interp * (contibute_1 * V_1 / Z1 + ... contibute_n * V_n / Zn)
		// we'll call '1/z' correction-factor-1 and 'z_interp' correction-factor-2 at other places
		reciprocalWs[i] = reciprocalW;
	}

	return true;
}

bool PuresoftPipeline::isBackFace(float* vert0, float* vert1, float* vert2)
{
	// determine back face in naive way :-(
	__declspec(align(16)) float vec0[4], vec1[4], cross[4], test[4] = {0, 0, 1.0f, 0};
	mcemaths_sub_3_4(vec0, vert1, vert0);
	mcemaths_sub_3_4(vec1, vert2, vert1);
	mcemaths_cross_3(cross, vec0, vec1);
	mcemaths_norm_3_4(cross);
	return (mcemaths_dot_3_4(test, cross) < 0);
}
