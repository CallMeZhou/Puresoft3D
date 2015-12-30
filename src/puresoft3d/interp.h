#pragma once
#include "mcemaths.hpp"
#include "proc.h"

class PuresoftInterpolater : public mcemaths::align_base_16
{
public:
	struct SCANLINE_BEGIN_PARAMS // from rasterization
	{
		const int* vertices;
		const float* reciprocalWs;
		const float* projectedZs;
		int row;
		int leftColumn;
		int rightColumn;
		const int* leftVerts;
		const int* rightVerts;
		const void* userData[3];
	};

public:
	PuresoftInterpolater(void);
	~PuresoftInterpolater(void);

	void setProcessor(PuresoftProcessor* proc);
	void scanlineBegin(int interpIdx, const SCANLINE_BEGIN_PARAMS* params);
	void scanlineNext(int interpIdx, float* interpZ, void* interpUserData);

private:
	__declspec(align(64)) struct INTERPOLATION
	{
		float correctionFactor2ForLeft;
		float correctionFactor2ForRight;
		float correctionFactor2Delta;
		float projectedZForLeft;
		float projectedZForRight;
		float projectedZDelta;
	};

	INTERPOLATION m_corrections[PuresoftProcessor::MAX_FRAG_PIPES];
	PuresoftProcessor* m_processor;

	static void integerBasedTrianglelinearInterpolate(const int* verts, int x,  int y, float* contributes);
	static void integerBasedLineSegmentlinearInterpolate(const int* verts, int vert1, int vert2, int x,  int y, float* contributes);
};

