#include <math.h>
#include "interp.h"

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
