#include <memory.h>
#include "proc.h"
#include "mcemaths.h"

void PuresoftInterpolationProcessor::scanlineBegin(int left, int right, int y, const float* contributesForLeft, const float* contributesForRight, const float* correctionFactor1)
{
	// calculate interpolated ext-values for left and right end of scanline
	__declspec(align(16)) float correctedContributes[4];
	mcemaths_quatcpy(correctedContributes, contributesForLeft);
	mcemaths_mulvec_3_4(correctedContributes, correctionFactor1);
	processLeftEnd(correctedContributes);
	mcemaths_quatcpy(correctedContributes, contributesForRight);
	mcemaths_mulvec_3_4(correctedContributes, correctionFactor1);
	processRightEnd(correctedContributes);

	// calculate perspective correction factor 2
	float reciprocalScanlineLength = right == left ? 1.0f : (1.0f / (float)(right - left));
	m_correctionFactor2ForLeft = mcemaths_dot_3_4(contributesForLeft, correctionFactor1);
	m_correctionFactor2ForRight = mcemaths_dot_3_4(contributesForRight, correctionFactor1);
	m_correctionFactor2Delta = (m_correctionFactor2ForRight - m_correctionFactor2ForLeft) * reciprocalScanlineLength;

	// calculate delta interpolation between left and right end of scanline
	processDelta(reciprocalScanlineLength);
}

void PuresoftInterpolationProcessor::scanlineNext(float* correctionFactor2, const void** outputExt)
{
	float _correctionFactor2 = 1.0f / m_correctionFactor2ForLeft;
	*correctionFactor2 = _correctionFactor2;

	m_correctionFactor2ForLeft += m_correctionFactor2Delta;

	*outputExt = processOutput(_correctionFactor2);
}