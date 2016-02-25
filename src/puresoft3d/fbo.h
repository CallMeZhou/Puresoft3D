#pragma once
#include <stddef.h>
#include "config.h"

class PuresoftFBO
{
public:
	__declspec(align(64)) struct WORKRANGE
	{
		unsigned int curRow;
		unsigned int curCol;
		void* curRowEntry;
		void* writePoint;
	};

	enum WRAPMODE {CLAMP, WRAP};
	enum LAYER {LAYER_DEFAULT, LAYER_XPOS = LAYER_DEFAULT, LAYER_XNEG, LAYER_YPOS, LAYER_YNEG, LAYER_ZPOS, LAYER_ZNEG, LAYER_MAX};

public:
	PuresoftFBO(unsigned int width, unsigned int scanline, unsigned int height, unsigned int elemLen, bool topDown = false, void* externalBuffer = NULL, WRAPMODE wrapMode = CLAMP, int extraLayers = 0);
	~PuresoftFBO(void);

	// sequential r/w
	void setCurRow(int idx, unsigned int row);
	void nextRow(int idx);
	void setCurCol(int idx, unsigned int col);
	void nextCol(int idx);
	void read(int idx, void* data, size_t bytes) const;
	void write(int idx, const void* data, size_t bytes);
	void read1(int idx, void* data) const;
	void write1(int idx, const void* data);
	void read4(int idx, void* data) const;
	void write4(int idx, const void* data);
	void read16(int idx, void* dataAligned16Bytes) const;  // elemLen must be 16
	void write16(int idx, const void* dataAligned16Bytes); // elemLen must be 16

	// random r/w
	void directRead(unsigned int row, unsigned int col, void* data, size_t bytes) const;
	void directWrite(unsigned int row, unsigned int col, const void* data, size_t bytes); // not thread safe
	void directRead1(unsigned int row, unsigned int col, void* data) const;
	void directWrite1(unsigned int row, unsigned int col, const void* data); // not thread safe
	void directRead4(unsigned int row, unsigned int col, void* data) const;
	void directWrite4(unsigned int row, unsigned int col, const void* data); // not thread safe
	void directRead16(unsigned int row, unsigned int col, void* dataAligned16Bytes) const; // elemLen must be 16
	void directWrite16(unsigned int row, unsigned int col, const void* dataAligned16Bytes); // elemLen must be 16, not thread safe

	// clearing up, must accord with elemLen
	void clear(const void* data, size_t bytes);
	void clear1(const void* data);
	void clear4(const void* data);
	void clear16(const void* dataAligned16Bytes);
	void clearToZero(void);

	// buffer manipulation and raw r/w
	void setBuffer(void* externalBuffer = NULL);
	void* getBuffer(void);
	const void* getBuffer(void) const;

	// attributes
	unsigned int getWidth(void) const;
	unsigned int getHeight(void) const;
	unsigned int getScanline(void) const;
	unsigned int getElemLen(void) const;
	size_t getBytes(void) const;

	// multi-layer support
	PuresoftFBO* getExtraLayer(LAYER layer);
	const PuresoftFBO* getExtraLayer(LAYER layer) const;

private:
	bool m_topDown;
	WRAPMODE m_wrapMode;
	unsigned int m_width;
	unsigned int m_maxCol;
	unsigned int m_scanline;
	unsigned int m_height;
	unsigned int m_maxRow;
	unsigned int m_elemLen;
	size_t m_bytes;
	void* m_buffer;
	void** m_rowEntries;
	bool m_isExternalBuffer;

	WORKRANGE m_workRanges[MAX_FRAGTHREADS];

	void clampCoord(unsigned int& row, unsigned int& col) const;

// multi-layer texture support (i.e., cube map)
private:
	PuresoftFBO* m_extraLayers[LAYER_MAX];
};