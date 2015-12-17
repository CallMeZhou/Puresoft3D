#pragma once
#include "proc.h"
#include "testpd.h"

class MyTestInterpolationProcessor :
	public PuresoftInterpolationProcessor
{
public:
	MyTestInterpolationProcessor(void);
	~MyTestInterpolationProcessor(void);

	void setInputExt(int idx, const void* ext);

	void process(MYTESTPROCDATA* outputExt, const float* contributes, const float* vert_z);

	void scanlineBegin(int left, int right, int y, const float* contributesForLeft, const float* contributesForRight, const float* correctionFactor1);
	void scanlineNext(float* correctionFactor2, const void** outputExt);

private:
	MYTESTPROCDATA m_inputExts[3];
	MYTESTPROCDATA m_outputExt;
	MYTESTPROCDATA m_outputExtForLeft;
	MYTESTPROCDATA m_outputExtForRight;
	MYTESTPROCDATA m_outputExtDelta;
	float m_correctionFactor2ForLeft;
	float m_correctionFactor2ForRight;
	float m_correctionFactor2Delta;
};

