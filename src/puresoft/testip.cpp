#include "StdAfx.h"
#include "testip.h"

MyTestInterpolationProcessor::MyTestInterpolationProcessor(void)
{
	memset(&m_outputExtForLeft, 0, sizeof(m_outputExt));
	memset(&m_outputExtForRight, 0, sizeof(m_outputExt));
	memset(&m_outputExtDelta, 0, sizeof(m_outputExt));
}

MyTestInterpolationProcessor::~MyTestInterpolationProcessor(void)
{
}

void MyTestInterpolationProcessor::setInputExt(int idx, const void* ext)
{
	memcpy(m_inputExts + idx, ext, sizeof(MYTESTPROCDATA));
}

void MyTestInterpolationProcessor::process(MYTESTPROCDATA* outputExt, const float* contributes, const float* vert_z)
{
	outputExt->normal[0] = (m_inputExts[0].normal[0] * contributes[0] * vert_z[0] + m_inputExts[1].normal[0] * contributes[1] * vert_z[1] + m_inputExts[2].normal[0] * contributes[2] * vert_z[2]);
	outputExt->normal[1] = (m_inputExts[0].normal[1] * contributes[0] * vert_z[0] + m_inputExts[1].normal[1] * contributes[1] * vert_z[1] + m_inputExts[2].normal[1] * contributes[2] * vert_z[2]);
	outputExt->normal[2] = (m_inputExts[0].normal[2] * contributes[0] * vert_z[0] + m_inputExts[1].normal[2] * contributes[1] * vert_z[1] + m_inputExts[2].normal[2] * contributes[2] * vert_z[2]);
	outputExt->worldPos[0] = (m_inputExts[0].worldPos[0] * contributes[0] * vert_z[0] + m_inputExts[1].worldPos[0] * contributes[1] * vert_z[1] + m_inputExts[2].worldPos[0] * contributes[2] * vert_z[2]);
	outputExt->worldPos[1] = (m_inputExts[0].worldPos[1] * contributes[0] * vert_z[0] + m_inputExts[1].worldPos[1] * contributes[1] * vert_z[1] + m_inputExts[2].worldPos[1] * contributes[2] * vert_z[2]);
	outputExt->worldPos[2] = (m_inputExts[0].worldPos[2] * contributes[0] * vert_z[0] + m_inputExts[1].worldPos[2] * contributes[1] * vert_z[1] + m_inputExts[2].worldPos[2] * contributes[2] * vert_z[2]);
 	outputExt->texcoord[0] =  (m_inputExts[0].texcoord[0] * contributes[0] * vert_z[0] + m_inputExts[1].texcoord[0] * contributes[1] * vert_z[1] + m_inputExts[2].texcoord[0] * contributes[2] * vert_z[2]);
 	outputExt->texcoord[1] =  (m_inputExts[0].texcoord[1] * contributes[0] * vert_z[0] + m_inputExts[1].texcoord[1] * contributes[1] * vert_z[1] + m_inputExts[2].texcoord[1] * contributes[2] * vert_z[2]);
}

void MyTestInterpolationProcessor::scanlineBegin(int left, int right, int y, const float* contributesForLeft, const float* contributesForRight, const float* correctionFactor1)
{
	process(&m_outputExtForLeft, contributesForLeft, correctionFactor1);
	process(&m_outputExtForRight, contributesForRight, correctionFactor1);

	float reciprocalScanlineLength = right == left ? 1.0f : (1.0f / (float)(right - left));

	// calculate perspective correction factor 2
	m_correctionFactor2ForLeft = mcemaths_dot_3_4(contributesForLeft, correctionFactor1);
	m_correctionFactor2ForRight = mcemaths_dot_3_4(contributesForRight, correctionFactor1);
	m_correctionFactor2Delta = (m_correctionFactor2ForRight - m_correctionFactor2ForLeft) * reciprocalScanlineLength;

	mcemaths_sub_3_4(m_outputExtDelta.normal, m_outputExtForRight.normal, m_outputExtForLeft.normal);
	mcemaths_sub_3_4(m_outputExtDelta.worldPos, m_outputExtForRight.worldPos, m_outputExtForLeft.worldPos);
	mcemaths_sub_3_4(m_outputExtDelta.texcoord, m_outputExtForRight.texcoord, m_outputExtForLeft.texcoord);
	mcemaths_mul_3_4(m_outputExtDelta.normal, reciprocalScanlineLength);
	mcemaths_mul_3_4(m_outputExtDelta.worldPos, reciprocalScanlineLength);
	mcemaths_mul_3_4(m_outputExtDelta.texcoord, reciprocalScanlineLength);
}

void MyTestInterpolationProcessor::scanlineNext(float* correctionFactor2, const void** outputExt)
{
	*outputExt = &m_outputExt;


	memcpy(&m_outputExt, &m_outputExtForLeft, sizeof(m_outputExt));

	float _correctionFactor2 = 1.0f / m_correctionFactor2ForLeft;
	*correctionFactor2 = _correctionFactor2;

	m_correctionFactor2ForLeft += m_correctionFactor2Delta;

	mcemaths_mul_3_4(m_outputExt.normal, _correctionFactor2);
	mcemaths_mul_3_4(m_outputExt.worldPos, _correctionFactor2);
	mcemaths_mul_3_4(m_outputExt.texcoord, _correctionFactor2);


	mcemaths_add_3_4_ip(m_outputExtForLeft.normal, m_outputExtDelta.normal);
	mcemaths_add_3_4_ip(m_outputExtForLeft.worldPos, m_outputExtDelta.worldPos);
	mcemaths_add_3_4_ip(m_outputExtForLeft.texcoord, m_outputExtDelta.texcoord);
}