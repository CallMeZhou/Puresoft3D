#pragma once
#include "mcemaths.hpp"
#include "proc.h"

class PuresoftInterpolater : public mcemaths::align_base_16
{
public:
	struct INTERPOLATIONSTARTSTEP
	{
		PuresoftInterpolationProcessor* proc;
		const void* vertexUserData[3];
		const int* vertices;
		const float* reciprocalWs;
		const float* projectedZs;
		int row;
		int leftColumn;
		int rightColumn;
		const int* leftVerts;
		const int* rightVerts;
		void* interpolatedUserDataStart;
		void* interpolatedUserDataStep;
		float correctionFactor2Start;
		float correctionFactor2Step;
		float projectedZStart;
		float projectedZStep;
	};

	struct INTERPOLATIONSTEPPING
	{
		PuresoftInterpolationProcessor* proc;
		void* interpolatedUserDataStart;
		void* interpolatedUserDataStep;
		float correctionFactor2Start;
		float correctionFactor2Step;
		float projectedZStart;
		float projectedZStep;
	};
public:
	PuresoftInterpolater(void);
	~PuresoftInterpolater(void);

	void interpolateStartAndStep(INTERPOLATIONSTARTSTEP* params);
	void interpolateNextStep(void* interpolatedUserData, float* interpolatedProjectedZ, INTERPOLATIONSTEPPING* params);

private:
	static void integerBasedTrianglelinearInterpolate(const int* verts, int x,  int y, float* contributes);
	static void integerBasedLineSegmentlinearInterpolate(const int* verts, int vert1, int vert2, int x,  int y, float* contributes);
};

