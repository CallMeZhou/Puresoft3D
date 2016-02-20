#include <windows.h>
#include <wtypes.h>
#include <gdiplus.h>
#include <stdexcept>
#include "samplr2d.h"
#include "pipeline.h"

using namespace std;
using namespace Gdiplus;

#define WIDTHBYTES(bits) (((bits) + 31) / 32 * 4); 

void PuresoftPipeline::setTexture(int idx, const wchar_t* filepath)
{
	if(idx < 0 || idx >= MAX_TEXTURES)
	{
		throw out_of_range("PuresoftPipeline::setTexture");
	}

	if(m_textures[idx].buff)
	{
		throw bad_exception("PuresoftPipeline::setTexture");
	}

	Bitmap pic(filepath);
	if(Ok != pic.GetLastStatus())
	{
		throw bad_exception("PuresoftPipeline::setTexture");
	}

	PURESOFTIMGBUFF32& imgBuff = m_textures[idx];
	imgBuff.width = pic.GetWidth();
	imgBuff.scanline = WIDTHBYTES(imgBuff.width * 4);
	imgBuff.height = pic.GetHeight();
	imgBuff.buff = malloc(imgBuff.scanline * imgBuff.height);

	pic.RotateFlip(RotateNoneFlipY);

	Rect r(0, 0, pic.GetWidth(), pic.GetHeight());
	BitmapData bmpdata;
	bmpdata.Width = imgBuff.width;
	bmpdata.Stride = imgBuff.scanline;
	bmpdata.Height = imgBuff.height;
	bmpdata.Scan0 = imgBuff.buff;
	bmpdata.PixelFormat = PixelFormat32bppARGB;
	bmpdata.Reserved = 0;
	pic.LockBits(&r, ImageLockModeUserInputBuf, PixelFormat32bppRGB, &bmpdata);
	pic.UnlockBits(&bmpdata);

	PuresoftSampler2D *sampler = new PuresoftSampler2D(imgBuff.width, imgBuff.scanline, imgBuff.height, 4, imgBuff.buff);

	m_
	pipeline.setTexture(0, &diffuseSmplr);

}
