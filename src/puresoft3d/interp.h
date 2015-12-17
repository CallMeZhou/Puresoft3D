#pragma once

class PuresoftInterpolater
{
public:
public:
	PuresoftInterpolater(void);
	~PuresoftInterpolater(void);

	static void integerBasedTrianglelinearInterpolate(const int* verts, int x,  int y, float* contributes);
	static void integerBasedLineSegmentlinearInterpolate(const int* verts, int vert1, int vert2, int x,  int y, float* contributes);
};

