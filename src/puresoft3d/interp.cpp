#include <atlbase.h>
#include <math.h>
#include <stdexcept>
#include "mcemaths.h"
#include "interp.h"

using namespace std;
using namespace mcemaths;

#pragma pack(4)
typedef struct
{
	float x;
	float y;
} __POINT;
#pragma pack()

PuresoftInterpolater::PuresoftInterpolater(void)
{
}

PuresoftInterpolater::~PuresoftInterpolater(void)
{
}

void PuresoftInterpolater::interpolateStartAndStep(INTERPOLATIONSTARTSTEP* params)
{
	__declspec(align(16)) float contributesForLeft[4];
	__declspec(align(16)) float contributesForRight[4];
	contributesForLeft[3] = contributesForRight[3] = 0;

	lineSegmentlinearInterpolate(params->vertices, params->leftVerts[0],  params->leftVerts[1],  (float)params->leftColumn,  (float)params->row, contributesForLeft);
	lineSegmentlinearInterpolate(params->vertices, params->rightVerts[0], params->rightVerts[1], (float)params->rightColumn, (float)params->row, contributesForRight);

	// calculate interpolated ext-values for left and right end of scanline
	__declspec(align(16)) float correctedContributesForLeft[8];
	float* correctedContributesForRight = correctedContributesForLeft + 4;
	mcemaths_quatcpy(correctedContributesForLeft, contributesForLeft);
	mcemaths_quatcpy(correctedContributesForRight, contributesForRight);
	mcemaths_mulvec_3_4(correctedContributesForLeft, params->reciprocalWs);
	mcemaths_mulvec_3_4(correctedContributesForRight, params->reciprocalWs);
	params->proc->interpolateByContributes(params->interpolatedUserDataStart, params->vertexUserData, correctedContributesForLeft);
	params->proc->interpolateByContributes(params->interpolatedUserDataStep, params->vertexUserData, correctedContributesForRight);
	int stepCount = params->rightColumn - params->leftColumn;
	params->proc->calcStep(params->interpolatedUserDataStep, params->interpolatedUserDataStart, params->interpolatedUserDataStep, stepCount);

	float reciprocalScanlineLength = 1.0f / (float)stepCount;

	params->projectedZStart = mcemaths_dot_3_4(correctedContributesForLeft, params->projectedZs);
	params->projectedZStep  = mcemaths_dot_3_4(correctedContributesForRight, params->projectedZs);
	params->projectedZStep  = (params->projectedZStep - params->projectedZStart) * reciprocalScanlineLength;

	// calculate perspective correction factor 2
	__declspec(align(16)) float temp[4];
	__asm{
		lea		eax,	correctedContributesForLeft
		lea		edx,	temp
		movaps	xmm0,	[eax]
		haddps	xmm0,	xmm0
		haddps	xmm0,	xmm0
		movaps	xmm1,	[eax + 16]
		haddps	xmm1,	xmm1
		haddps	xmm1,	xmm1
		movss	xmm1,	xmm0
		movaps	[edx],	xmm1
	}
	params->correctionFactor2Start = temp[0];
	params->correctionFactor2Step = temp[2];
// the 2 lines below are replaced by the above asm code
//	params->correctionFactor2Start = mcemaths_dot_3_4(contributesForLeft, params->reciprocalWs);
//	params->correctionFactor2Step = mcemaths_dot_3_4(contributesForRight, params->reciprocalWs);
	params->correctionFactor2Step = (params->correctionFactor2Step - params->correctionFactor2Start) * reciprocalScanlineLength;

	if(params->leftColumnSkipping > 0)
	{
		params->correctionFactor2Start += params->correctionFactor2Step * params->leftColumnSkipping;
		params->projectedZStart += params->projectedZStep * params->leftColumnSkipping;
		params->proc->stepForward(params->interpolatedUserDataStart, params->interpolatedUserDataStep, params->leftColumnSkipping);
	}
}

void PuresoftInterpolater::interpolateNextStepForZ(float* interpolatedProjectedZ, INTERPOLATIONSTEPPING* params)
{
	*interpolatedProjectedZ = params->projectedZStart / params->correctionFactor2Start;
	params->projectedZStart += params->projectedZStep;
}

void PuresoftInterpolater::interpolateNextStepForUserData(void* interpolatedUserData, INTERPOLATIONSTEPPING* params, int stepCount)
{
	float _correctionFactor2 = 1.0f / params->correctionFactor2Start;

	params->proc->correctInterpolation(interpolatedUserData, params->interpolatedUserDataStart, _correctionFactor2);
	params->proc->stepForward(params->interpolatedUserDataStart, params->interpolatedUserDataStep, stepCount);
}

void PuresoftInterpolater::interpolateNextStepForCorrection(INTERPOLATIONSTEPPING* params)
{
	params->correctionFactor2Start += params->correctionFactor2Step;
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
void PuresoftInterpolater::integerBasedTrianglelinearInterpolate(const float* verts, int x, int y, float* contributes)
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
*/

void PuresoftInterpolater::lineSegmentlinearInterpolate(const float* verts, int vert1, int vert2, float x,  float y, float* contributes)
{
	const __POINT& p1 = *(((const __POINT*)verts) + vert1);
	const __POINT& p2 = *(((const __POINT*)verts) + vert2);

	float dx = p1.x - p2.x, dy = p1.y - p2.y;
	contributes[vert1] = fabs(dx) > fabs(dy) ? ((x - p2.x) / dx) : ((y - p2.y) / dy);
	contributes[vert2] = 1.0f - contributes[vert1];
	contributes[3 - vert1 - vert2] = 0;
}
