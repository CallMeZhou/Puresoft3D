#include <memory.h>
#include "mcemaths.h"
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

void MyTestInterpolationProcessor::processLeftEnd(const float* correctedContributes)
{
	processEndData(&m_outputExtForLeft, correctedContributes);
}

void MyTestInterpolationProcessor::processRightEnd(const float* correctedContributes)
{
	processEndData(&m_outputExtForRight, correctedContributes);
}

void MyTestInterpolationProcessor::processDelta(float reciprocalScanlineLength)
{
	mcemaths_sub_3_4(m_outputExtDelta.normal, m_outputExtForRight.normal, m_outputExtForLeft.normal);
	mcemaths_sub_3_4(m_outputExtDelta.worldPos, m_outputExtForRight.worldPos, m_outputExtForLeft.worldPos);
	mcemaths_sub_3_4(m_outputExtDelta.texcoord, m_outputExtForRight.texcoord, m_outputExtForLeft.texcoord);
	mcemaths_mul_3_4(m_outputExtDelta.normal, reciprocalScanlineLength);
	mcemaths_mul_3_4(m_outputExtDelta.worldPos, reciprocalScanlineLength);
	mcemaths_mul_3_4(m_outputExtDelta.texcoord, reciprocalScanlineLength);
}

void* MyTestInterpolationProcessor::processOutput(float correctionFactor2)
{
	memcpy(&m_outputExt, &m_outputExtForLeft, sizeof(m_outputExt));

	mcemaths_mul_3_4(m_outputExt.normal, correctionFactor2);
	mcemaths_mul_3_4(m_outputExt.worldPos, correctionFactor2);
	mcemaths_mul_3_4(m_outputExt.texcoord, correctionFactor2);

	mcemaths_add_3_4_ip(m_outputExtForLeft.normal, m_outputExtDelta.normal);
	mcemaths_add_3_4_ip(m_outputExtForLeft.worldPos, m_outputExtDelta.worldPos);
	mcemaths_add_3_4_ip(m_outputExtForLeft.texcoord, m_outputExtDelta.texcoord);

	return &m_outputExt;
}

void MyTestInterpolationProcessor::processEndData(MYTESTPROCDATA* data, const float* correctedContributes)
{
	/*
	A clearer way to do it. For your reference.

	m_outputExtForLeft.normal[0] = (m_inputExts[0].normal[0] * contributes[0] * vert_z[0] + m_inputExts[1].normal[0] * contributes[1] * vert_z[1] + m_inputExts[2].normal[0] * contributes[2] * vert_z[2]);
	m_outputExtForLeft.normal[1] = (m_inputExts[0].normal[1] * contributes[0] * vert_z[0] + m_inputExts[1].normal[1] * contributes[1] * vert_z[1] + m_inputExts[2].normal[1] * contributes[2] * vert_z[2]);
	m_outputExtForLeft.normal[2] = (m_inputExts[0].normal[2] * contributes[0] * vert_z[0] + m_inputExts[1].normal[2] * contributes[1] * vert_z[1] + m_inputExts[2].normal[2] * contributes[2] * vert_z[2]);
	m_outputExtForLeft.worldPos[0] = (m_inputExts[0].worldPos[0] * contributes[0] * vert_z[0] + m_inputExts[1].worldPos[0] * contributes[1] * vert_z[1] + m_inputExts[2].worldPos[0] * contributes[2] * vert_z[2]);
	m_outputExtForLeft.worldPos[1] = (m_inputExts[0].worldPos[1] * contributes[0] * vert_z[0] + m_inputExts[1].worldPos[1] * contributes[1] * vert_z[1] + m_inputExts[2].worldPos[1] * contributes[2] * vert_z[2]);
	m_outputExtForLeft.worldPos[2] = (m_inputExts[0].worldPos[2] * contributes[0] * vert_z[0] + m_inputExts[1].worldPos[2] * contributes[1] * vert_z[1] + m_inputExts[2].worldPos[2] * contributes[2] * vert_z[2]);
	m_outputExtForLeft.texcoord[0] =  (m_inputExts[0].texcoord[0] * contributes[0] * vert_z[0] + m_inputExts[1].texcoord[0] * contributes[1] * vert_z[1] + m_inputExts[2].texcoord[0] * contributes[2] * vert_z[2]);
	m_outputExtForLeft.texcoord[1] =  (m_inputExts[0].texcoord[1] * contributes[0] * vert_z[0] + m_inputExts[1].texcoord[1] * contributes[1] * vert_z[1] + m_inputExts[2].texcoord[1] * contributes[2] * vert_z[2]);
	*/

	MYTESTPROCDATA temp[3];
	memcpy(&temp, m_inputExts, sizeof(MYTESTPROCDATA) * 3);

	mcemaths_mul_3_4(temp[0].normal, correctedContributes[0]);
	mcemaths_mul_3_4(temp[0].worldPos, correctedContributes[0]);
	mcemaths_mul_3_4(temp[0].texcoord, correctedContributes[0]);

	mcemaths_mul_3_4(temp[1].normal, correctedContributes[1]);
	mcemaths_mul_3_4(temp[1].worldPos, correctedContributes[1]);
	mcemaths_mul_3_4(temp[1].texcoord, correctedContributes[1]);

	mcemaths_mul_3_4(temp[2].normal, correctedContributes[2]);
	mcemaths_mul_3_4(temp[2].worldPos, correctedContributes[2]);
	mcemaths_mul_3_4(temp[2].texcoord, correctedContributes[2]);

	mcemaths_quatcpy(data->normal, temp[0].normal);
	mcemaths_quatcpy(data->worldPos, temp[0].worldPos);
	mcemaths_quatcpy(data->texcoord, temp[0].texcoord);

	mcemaths_add_3_4_ip(data->normal, temp[1].normal);
	mcemaths_add_3_4_ip(data->worldPos, temp[1].worldPos);
	mcemaths_add_3_4_ip(data->texcoord, temp[1].texcoord);

	mcemaths_add_3_4_ip(data->normal, temp[2].normal);
	mcemaths_add_3_4_ip(data->worldPos, temp[2].worldPos);
	mcemaths_add_3_4_ip(data->texcoord, temp[2].texcoord);
}
