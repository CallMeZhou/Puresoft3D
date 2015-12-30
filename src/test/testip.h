#pragma once
#include "proc.h"
#include "testpd.h"

class MyTestInterpolationProcessor :
	public PuresoftInterpolationProcessor
{
public:
	static void* _stdcall createInstance(void);

public:
	MyTestInterpolationProcessor(void);
	~MyTestInterpolationProcessor(void);

	void release(void);
	void setUserData(const void* const data[3]);
	void processLeftEnd(const float* correctedContributes);
	void processRightEnd(const float* correctedContributes);
	void processDelta(float reciprocalScanlineLength);
	void processOutput(float correctionFactor2, void* interpData);

private:
	MYTESTPROCDATA m_inputs[3];
	MYTESTPROCDATA m_outputExt;
	MYTESTPROCDATA m_outputExtForLeft;
	MYTESTPROCDATA m_outputExtForRight;
	MYTESTPROCDATA m_outputExtDelta;

	void processEndData(MYTESTPROCDATA* data, const float* correctedContributes);
};

