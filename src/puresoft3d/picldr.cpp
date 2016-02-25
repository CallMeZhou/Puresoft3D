#include <windows.h>
#include <wtypes.h>
#include <gdiplus.h>
#include <shlwapi.h>
#include <stdexcept>
#include <atlbase.h>
#include "picldr.h"

using namespace std;
using namespace Gdiplus;

#define WIDTHBYTES(bits) (((bits) + 31) / 32 * 4); 

PuresoftDefaultPictureLoader::PuresoftDefaultPictureLoader(void)
{
	m_gdiplbmp = 
	m_bufstream = 0;
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup((ULONG_PTR*)&m_gdipltok, &gdiplusStartupInput, NULL);
}


PuresoftDefaultPictureLoader::~PuresoftDefaultPictureLoader()
{
	close();

	if(m_gdipltok)
	{
		GdiplusShutdown((ULONG_PTR)m_gdipltok);
	}
}

void PuresoftDefaultPictureLoader::loadFromFile(const wchar_t* path, PURESOFTIMGBUFF32* imageInfo)
{
	if(m_gdiplbmp)
	{
		throw std::bad_exception("PuresoftDefaultPictureLoader::loadFromFile, already open");
	}

	Bitmap* bmp = new Bitmap(path);
	if(Ok != bmp->GetLastStatus())
	{
		delete bmp;
		throw std::bad_exception("PuresoftDefaultPictureLoader::loadFromFile, failed to open");
	}

	bmp->RotateFlip(RotateNoneFlipY);

	m_gdiplbmp = (uintptr_t)bmp;

	if(imageInfo)
	{
		imageInfo->width = bmp->GetWidth();
		imageInfo->scanline = WIDTHBYTES(imageInfo->width * 32);
		imageInfo->height = bmp->GetHeight();
		imageInfo->elemLen = 4;
		imageInfo->pixels = NULL;
	}
}

void PuresoftDefaultPictureLoader::loadFromBuffer(const void* buffer, unsigned int bytes, PURESOFTIMGBUFF32* imageInfo)
{
	if(m_gdiplbmp)
	{
		throw std::bad_exception("PuresoftDefaultPictureLoader::loadFromFile, already open");
	}

	CComPtr<IStream> strm = SHCreateMemStream((const BYTE*)buffer, bytes);
	if(!strm)
	{
		throw std::bad_exception("PuresoftDefaultPictureLoader::loadFromFile, SHCreateMemStream");
	}

	Bitmap* bmp = new Bitmap(strm);
	if(Ok != bmp->GetLastStatus())
	{
		delete bmp;
		throw std::bad_exception("PuresoftDefaultPictureLoader::loadFromFile, failed to open");
	}

	bmp->RotateFlip(RotateNoneFlipY);

	m_gdiplbmp = (uintptr_t)bmp;
	m_bufstream = (uintptr_t)strm.Detach();

	if(imageInfo)
	{
		imageInfo->width = bmp->GetWidth();
		imageInfo->scanline = WIDTHBYTES(imageInfo->width * 32);
		imageInfo->height = bmp->GetHeight();
		imageInfo->elemLen = 4;
	}
}

void PuresoftDefaultPictureLoader::retrievePixel(PURESOFTIMGBUFF32* image)
{
	if(!m_gdiplbmp)
	{
		throw std::bad_exception("PuresoftDefaultPictureLoader::retrievePixel, not open");
	}

	Bitmap* bmp = (Bitmap*)m_gdiplbmp;

	image->width = bmp->GetWidth();
	image->scanline = WIDTHBYTES(image->width * 32);
	image->height = bmp->GetHeight();
	image->elemLen = 4;

	BitmapData bmpdata;
	bmpdata.Width = image->width;
	bmpdata.Stride = image->scanline;
	bmpdata.Height = image->height;
	bmpdata.PixelFormat = PixelFormat32bppARGB;
	bmpdata.Scan0 = image->pixels;
	bmpdata.Reserved = 0;
	Status s = bmp->LockBits(&Rect(0, 0, image->width, image->height), ImageLockModeUserInputBuf | ImageLockModeRead, PixelFormat32bppARGB, &bmpdata);
	bmp->UnlockBits(&bmpdata);
}

void PuresoftDefaultPictureLoader::close(void)
{
	if(m_gdiplbmp)
	{
		delete (Bitmap*)m_gdiplbmp;
		m_gdiplbmp = 0;
	}

	if(m_bufstream)
	{
		((IStream*)m_bufstream)->Release();
		m_bufstream = 0;
	}
}