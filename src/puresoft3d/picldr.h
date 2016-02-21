#pragma once
#include <stdint.h>
#include "defs.h"

class PuresoftPictureLoader
{
public:
	virtual ~PuresoftPictureLoader() {}

	virtual void loadFromFile(const wchar_t* path, PURESOFTIMGBUFF32* imageInfo = NULL) = 0;
	virtual void loadFromBuffer(const void* buffer, unsigned int bytes, PURESOFTIMGBUFF32* imageInfo = NULL) = 0;
	virtual void retrievePixel(PURESOFTIMGBUFF32* imageInfo) = 0;
	virtual void close(void) = 0;
};

class PuresoftDefaultPictureLoader : public PuresoftPictureLoader
{
public:
	PuresoftDefaultPictureLoader(void);
	~PuresoftDefaultPictureLoader();
	void loadFromFile(const wchar_t* path, PURESOFTIMGBUFF32* imageInfo);
	void loadFromBuffer(const void* buffer, unsigned int bytes, PURESOFTIMGBUFF32* imageInfo);
	void retrievePixel(PURESOFTIMGBUFF32* image);
	void close(void);
private:
	uintptr_t m_gdipltok;
	uintptr_t m_gdiplbmp;
	uintptr_t m_bufstream;
};