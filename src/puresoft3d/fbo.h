#pragma once
#include <stddef.h>
#include "config.h"

class PuresoftFBO
{
public:
	__declspec(align(64)) struct WORKRANGE
	{
		size_t curRowEntry;
		void* writePoint;
	};

public:
	PuresoftFBO(unsigned int width, unsigned int scanline, unsigned int height, unsigned int elemLen, bool topDown = false, void* externalBuffer = NULL);
	~PuresoftFBO(void);

	void setCurRow(int idx, unsigned int row);
	void nextRow(int idx);
	void setCurCol(int idx, unsigned int col);
	void nextCol(int idx);
	void read(int idx, void* data, size_t bytes) const;
	void write(int idx, const void* data, size_t bytes);
	void read4(int idx, void* data) const;
	void write4(int idx, const void* data);
	void read16(int idx, void* dataAligned16Bytes) const;  // elemLen must be 16
	void write16(int idx, const void* dataAligned16Bytes); // elemLen must be 16

	// must accord with elemLen
	void clear(const void* data, size_t bytes);
	void clear4(const void* data);
	void clear16(const void* dataAligned16Bytes);

	void setBuffer(void* externalBuffer = NULL);
	void* getBuffer(void);
	const void* getBuffer(void) const;

	unsigned int getWidth(void) const;
	unsigned int getHeight(void) const;
	unsigned int getScanline(void) const;
	unsigned int getElemLen(void) const;

private:
	bool m_topDown;
	unsigned int m_width;
	unsigned int m_scanline;
	unsigned int m_height;
	unsigned int m_elemLen;
	size_t m_bytes;
	void* m_buffer;
	bool m_isExternalBuffer;

	WORKRANGE m_workRanges[MAX_FRAGTHREADS];
};