#include <math.h>
#include <stdexcept>
#include "mcemaths.h"
#include "interp.h"

using namespace std;
using namespace mcemaths;

#pragma pack(4)
typedef struct
{
	int x;
	int y;
} __POINT;
#pragma pack()

PuresoftInterpolater::PuresoftInterpolater(void)
{
}

PuresoftInterpolater::~PuresoftInterpolater(void)
{
}

void PuresoftInterpolater::setProcessor(PuresoftProcessor* proc)
{
	m_processor = proc;
}

void PuresoftInterpolater::scanlineBegin(int interpIdx, const SCANLINE_BEGIN_PARAMS* params)
{
	if(interpIdx < 0 || interpIdx >= PuresoftProcessor::MAX_FRAG_PIPES)
	{
		throw out_of_range("PuresoftInterpolater::scanlineBegin");
	}

	__declspec(align(16)) float contributesForLeft[4];
	__declspec(align(16)) float contributesForRight[4];
	contributesForLeft[3] = contributesForRight[3] = 0;

	integerBasedLineSegmentlinearInterpolate(params->vertices, params->leftVerts[0], params->leftVerts[1], params->leftColumn, params->row, contributesForLeft);
	integerBasedLineSegmentlinearInterpolate(params->vertices, params->rightVerts[0], params->rightVerts[1], params->rightColumn, params->row, contributesForRight);

	PuresoftInterpolationProcessor* proc = m_processor->getInterpProc(interpIdx);
	INTERPOLATION& interp = m_corrections[interpIdx];

	proc->setUserData(params->userData);

	// calculate interpolated ext-values for left and right end of scanline
	__declspec(align(16)) float correctedContributes[4];
	mcemaths_quatcpy(correctedContributes, contributesForLeft);
	mcemaths_mulvec_3_4(correctedContributes, params->reciprocalWs);
	proc->processLeftEnd(correctedContributes);
	mcemaths_quatcpy(correctedContributes, contributesForRight);
	mcemaths_mulvec_3_4(correctedContributes, params->reciprocalWs);
	proc->processRightEnd(correctedContributes);

	// calculate perspective correction factor 2
	float reciprocalScanlineLength = params->rightColumn == params->leftColumn ? 1.0f : (1.0f / (float)(params->rightColumn - params->leftColumn));
	interp.correctionFactor2ForLeft = mcemaths_dot_3_4(contributesForLeft, params->reciprocalWs);
	interp.correctionFactor2ForRight = mcemaths_dot_3_4(contributesForRight, params->reciprocalWs);
	interp.correctionFactor2Delta = (interp.correctionFactor2ForRight - interp.correctionFactor2ForLeft) * reciprocalScanlineLength;

	// calculate delta interpolation between left and right end of scanline
	proc->processDelta(reciprocalScanlineLength);

	__declspec(align(16)) float projZsWithcorrectionFactor1[4];
	mcemaths_quatcpy(projZsWithcorrectionFactor1, params->projectedZs);
	mcemaths_mulvec_3_4(projZsWithcorrectionFactor1, params->reciprocalWs);
	interp.projectedZForLeft = mcemaths_dot_3_4(contributesForLeft, projZsWithcorrectionFactor1);
	interp.projectedZForRight = mcemaths_dot_3_4(contributesForRight, projZsWithcorrectionFactor1);
	interp.projectedZDelta = (interp.projectedZForRight - interp.projectedZForLeft) * reciprocalScanlineLength;
}

void PuresoftInterpolater::scanlineNext(int interpIdx, float* interpZ, void* interpUserData)
{
	if(interpIdx < 0 || interpIdx >= PuresoftProcessor::MAX_FRAG_PIPES)
	{
		throw out_of_range("PuresoftInterpolater::scanlineBegin");
	}

	PuresoftInterpolationProcessor* proc = m_processor->getInterpProc(interpIdx);
	INTERPOLATION& interp = m_corrections[interpIdx];

	float _correctionFactor2 = 1.0f / interp.correctionFactor2ForLeft;
	interp.correctionFactor2ForLeft += interp.correctionFactor2Delta;

	proc->processOutput(_correctionFactor2, interpUserData);

	*interpZ = interp.projectedZForLeft * _correctionFactor2;
	interp.projectedZForLeft += interp.projectedZDelta;
}

/*

p.x = p1.x * c1 + p2.x * c2 + p3.x * (1 - c1 - c2)
p.y = p1.y * c1 + p2.y * c2 + p3.y * (1 - c1 - c2)

p.x = p1.x * c1 + p2.x * c2 + p3.x - p3.x * c1 - p3.x * c2
p.x = p1.x * c1 - p3.x * c1 + p3.x + p2.x * c2 - p3.x * c2
p.x - p3.x = (p1.x - p3.x) * c1 + (p2.x - p3.x) * c2
c2 = ((p.x - p3.x) - (p1.x - p3.x) * c1) / (p2.x - p3.x)
c2 = ((p.y - p3.y) - (p1.y - p3.y) * c1) / (p2.y - p3.y)
((p.x - p3.x) - (p1.x - p3.x) * c1) / (p2.x - p3.x) - ((p.y - p3.y) - (p1.y - p3.y) * c1) / (p2.y - p3.y) = 0

(p.x - p3.x) / (p2.x - p3.x) - c1 * (p1.x - p3.x) / (p2.x - p3.x) - (p.y - p3.y) / (p2.y - p3.y) + c1 * (p1.y - p3.y) / (p2.y - p3.y) = 0
c1  * ((p1.y - p3.y) / (p2.y - p3.y) - (p1.x - p3.x) / (p2.x - p3.x)) = ((p.y - p3.y) / (p2.y - p3.y) - (p.x - p3.x) / (p2.x - p3.x))
c1 = ((p.y - p3.y) / (p2.y - p3.y) - (p.x - p3.x) / (p2.x - p3.x)) / (((p1.y - p3.y) / (p2.y - p3.y) - (p1.x - p3.x) / (p2.x - p3.x)))
*/
void PuresoftInterpolater::integerBasedTrianglelinearInterpolate(const int* verts, int x, int y, float* contributes)
{
	// in compiler we trust ...

	const __POINT& p1 = *(const __POINT*)(verts);
	const __POINT& p2 = *(const __POINT*)(verts + 2);
	const __POINT& p3 = *(const __POINT*)(verts + 4);

	if(p2.y == p3.y)
	{
		contributes[0] = (float)(y - p3.y) / (float)(p1.y - p3.y);
	}
	else if(p2.x == p3.x)
	{
		contributes[0] = (float)(x - p3.x) / (float)(p1.x - p3.x);
	}
	else if(p1.x == p3.x)
	{
		contributes[0] = (((float)p2.x - (float)p3.x) * ((float)y - (float)p3.y) - ((float)p2.y - (float)p3.y) * ((float)x - (float)p3.x)) / (((float)p2.x - (float)p3.x) * ((float)p1.y - (float)p3.y));
	}
	else if(p1.y == p3.y)
	{
		contributes[0] = (((float)p2.y - (float)p3.y) * ((float)x - (float)p3.x) - ((float)p2.x - (float)p3.x) * ((float)y - (float)p3.y)) / (((float)p2.y - (float)p3.y) * ((float)p1.x - (float)p3.x));
	}
	else
	{
		contributes[0] = (((float)y - (float)p3.y) / ((float)p2.y - (float)p3.y) - ((float)x - (float)p3.x) / ((float)p2.x - (float)p3.x)) / ((((float)p1.y - (float)p3.y) / ((float)p2.y - (float)p3.y) - ((float)p1.x - (float)p3.x) / ((float)p2.x - (float)p3.x)));		
	}

	if(p2.x == p3.x)
	{
		contributes[1] = (((float)y - (float)p3.y) - ((float)p1.y - (float)p3.y) * contributes[0]) / ((float)p2.y - (float)p3.y);
	}
	else
	{
		contributes[1] = (((float)x - (float)p3.x) - ((float)p1.x - (float)p3.x) * contributes[0]) / ((float)p2.x - (float)p3.x);
	}

	contributes[2] = 1.0f - contributes[0] - contributes[1];
}

void PuresoftInterpolater::integerBasedLineSegmentlinearInterpolate(const int* verts, int vert1, int vert2, int x,  int y, float* contributes)
{
	const __POINT& p1 = *(((const __POINT*)verts) + vert1);
	const __POINT& p2 = *(((const __POINT*)verts) + vert2);

	int dx = p1.x - p2.x, dy = p1.y - p2.y;
	contributes[vert1] = labs(dx) > labs(dy) ? (((float)x - (float)p2.x) / (float)dx) : (((float)y - (float)p2.y) / (float)dy);
	contributes[vert2] = 1.0f - contributes[vert1];
	contributes[3 - vert1 - vert2] = 0;
}
