#pragma once
#include <stddef.h>
#include "config.h"

class PuresoftFBO
{
public:
	__declspec(align(64)) struct WORKRANGE
	{
		int curRow;
		int curCol;
		void* curRowEntry;
		void* writePoint;
		bool overflow;
	};

	enum WRAPMODE {CLAMP, WRAP};
	enum LAYER {LAYER_DEFAULT, LAYER_XPOS = LAYER_DEFAULT, LAYER_XNEG, LAYER_YPOS, LAYER_YNEG, LAYER_ZPOS, LAYER_ZNEG, LAYER_MAX};

public:
	PuresoftFBO( int width,  int scanline,  int height,  int elemLen, bool topDown = false, void* externalBuffer = NULL, WRAPMODE wrapMode = CLAMP, int extraLayers = 0);
	~PuresoftFBO(void);

	// sequential r/w
	void setCurRow(int idx,  int row);
	void nextRow(int idx);
	void setCurCol(int idx,  int col);
	void nextCol(int idx);
	void read(int idx, void* data, size_t bytes) const;
	void write(int idx, const void* data, size_t bytes);
	void read1(int idx, void* data) const;
	void write1(int idx, const void* data);
	void read4(int idx, void* data) const;
	void write4(int idx, const void* data);
	void blend4(int idx, const unsigned char* bgra);
	void read16(int idx, void* dataAligned16Bytes) const;  // elemLen must be 16
	void write16(int idx, const void* dataAligned16Bytes); // elemLen must be 16
	void blend16(int idx, const float* bgra);

	// random r/w
	void directRead( int row,  int col, void* data, size_t bytes) const;
	void directWrite( int row,  int col, const void* data, size_t bytes); // not thread safe
	void directRead1( int row,  int col, void* data) const;
	void directWrite1( int row,  int col, const void* data); // not thread safe
	void directRead4( int row,  int col, void* data) const;
	void directWrite4( int row,  int col, const void* data); // not thread safe
	void directRead16( int row,  int col, void* dataAligned16Bytes) const; // elemLen must be 16
	void directWrite16( int row,  int col, const void* dataAligned16Bytes); // elemLen must be 16, not thread safe

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
	 int getWidth(void) const;
	 int getHeight(void) const;
	 int getScanline(void) const;
	 int getElemLen(void) const;
	size_t getBytes(void) const;

	// multi-layer support
	PuresoftFBO* getExtraLayer(LAYER layer);
	const PuresoftFBO* getExtraLayer(LAYER layer) const;

	// debuggabilities
	void saveAsBmpFile(const wchar_t* path, bool dataIsFloat) const;
	void saveAsRawFile(const wchar_t* path) const;

private:
	bool m_topDown;
	WRAPMODE m_wrapMode;
	int m_width;
	int m_maxCol;
	int m_scanline;
	int m_height;
	int m_maxRow;
	int m_elemLen;
	size_t m_bytes;
	void* m_buffer;
	void** m_rowEntries;
	bool m_isExternalBuffer;

	WORKRANGE m_workRanges[MAX_FRAGTHREADS];

	bool clampCoord(int& row,  int& col) const;
	bool validateCoord(int row,  int col) const;

// multi-layer texture support (i.e., cube map)
private:
	PuresoftFBO* m_extraLayers[LAYER_MAX];
};