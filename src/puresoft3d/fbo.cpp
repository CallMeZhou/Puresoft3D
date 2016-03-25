#include <windows.h>
#include <stdlib.h>
#include <assert.h>
#include <stdexcept>
#include "mcemaths.h"
#include "fbo.h"

using namespace std;

PuresoftFBO::PuresoftFBO(
	 int width, 
	 int scanline, 
	 int height, 
	 int elemLen, 
	bool topDown /* = false */, 
	void* externalBuffer /* = NULL */, 
	WRAPMODE wrapMode /* = CLAMP */, 
	int extraLayers /* = 0 */)
{
	if(1 != elemLen && 4 != elemLen)
	{
		throw std::invalid_argument("PuresoftFBO::PuresoftFBO, (1 != elemLen && 4 != elemLen)");
	}

	if(extraLayers && externalBuffer)
	{
		throw std::invalid_argument("PuresoftFBO::PuresoftFBO, (extraLayers > 0 && externalBuffer != NULL)");
	}

	m_topDown = topDown;
	m_wrapMode = wrapMode;
	m_width = width;
	m_maxCol = width - 1;
	m_scanline = scanline;
	m_height = height;
	m_maxRow = height - 1;
	m_elemLen = elemLen;
	m_bytes = m_scanline * m_height;

	if(NULL == (m_rowEntries = (void**)_aligned_malloc(sizeof(void*) * m_height, 16)))
	{
		throw bad_alloc("PuresoftFBO::PuresoftFBO");
	}

	m_isExternalBuffer = false;
	m_buffer = NULL;
	setBuffer(externalBuffer);

	for(size_t i = 0; i < MAX_FRAGTHREADS; i++)
	{
		m_workRanges[i].curRow = 
		m_workRanges[i].curCol = 0;
		m_workRanges[i].curRowEntry = 0;
		m_workRanges[i].writePoint = m_buffer;
		m_workRanges[i].overflow = false;
	}

	memset(m_extraLayers, 0, sizeof(m_extraLayers));
	m_extraLayers[0] = this;

	if(extraLayers > 0)
	{
		if(extraLayers > LAYER_MAX)
		{
			throw out_of_range("PuresoftFBO::PuresoftFBO");
		}

		for(int i = 1; i <= extraLayers; i++)
		{
			m_extraLayers[i] = new PuresoftFBO(width, scanline, height, elemLen, topDown, NULL, wrapMode, 0);
		}
	}
}


PuresoftFBO::~PuresoftFBO(void)
{
	if(!m_isExternalBuffer && m_buffer)
	{
		_aligned_free(m_buffer);
	}

	if(m_rowEntries)
	{
		_aligned_free(m_rowEntries);
	}

	for(int i = 1; i < LAYER_MAX; i++)
	{
		if(m_extraLayers[i])
		{
			delete m_extraLayers[i];
		}
	}
}

void PuresoftFBO::setCurRow(int idx,  int row)
{
	 int unused = 0;
	m_workRanges[idx].overflow = clampCoord(row, unused);
	m_workRanges[idx].curRow = row;
	if(m_topDown)
		m_workRanges[idx].curRowEntry = m_rowEntries[m_height - row - 1];
	else
		m_workRanges[idx].curRowEntry = m_rowEntries[row];
	m_workRanges[idx].writePoint = m_workRanges[idx].curRowEntry;
}

void PuresoftFBO::nextRow(int idx)
{
	if(m_topDown)
	{
		if(m_workRanges[idx].curRow <= 0)
		{
			if(WRAP == m_wrapMode)
			{
				setCurRow(idx, m_maxRow);
			}
		}
		else
		{
			setCurRow(idx, m_workRanges[idx].curRow - 1);
		}
	}
	else
	{
		if(m_workRanges[idx].curRow >= m_maxRow)
		{
			if(WRAP == m_wrapMode)
			{
				setCurRow(idx, 0);
			}
			else
			{
				m_workRanges[idx].overflow = true;
			}
		}
		else
		{
			setCurRow(idx, m_workRanges[idx].curRow + 1);
		}

	}
}

void PuresoftFBO::setCurCol(int idx,  int col)
{
	 int unused = 0;
	m_workRanges[idx].overflow = clampCoord(unused, col);
	m_workRanges[idx].curCol = col;
	m_workRanges[idx].writePoint = (void*)((uintptr_t)m_workRanges[idx].curRowEntry + col * m_elemLen);
}

void PuresoftFBO::nextCol(int idx)
{
	if(m_workRanges[idx].curCol >= m_maxCol)
	{
		if(WRAP == m_wrapMode)
		{
			setCurCol(idx, 0);
		}
		else
		{
			m_workRanges[idx].overflow = true;
		}
	}
	else
	{
		m_workRanges[idx].writePoint = (void*)((uintptr_t)m_workRanges[idx].writePoint + m_elemLen);
		m_workRanges[idx].curCol++;
	}
}

void PuresoftFBO::read(int idx, void* data, size_t bytes) const
{
	memcpy(data, m_workRanges[idx].writePoint, bytes);
}

void PuresoftFBO::write(int idx, const void* data, size_t bytes)
{
	if(!m_workRanges[idx].overflow)
		memcpy(m_workRanges[idx].writePoint, data, bytes);
}

void PuresoftFBO::read1(int idx, void* data) const
{
	*(( char*)data) = *((const  char*)m_workRanges[idx].writePoint);
}

void PuresoftFBO::write1(int idx, const void* data)
{
	if(!m_workRanges[idx].overflow)
		*(( char*)m_workRanges[idx].writePoint) = *((const  char*)data);
}

void PuresoftFBO::read4(int idx, void* data) const
{
	*(( int*)data) = *((const  int*)m_workRanges[idx].writePoint);
}

void PuresoftFBO::write4(int idx, const void* data)
{
	if(!m_workRanges[idx].overflow)
		*(( int*)m_workRanges[idx].writePoint) = *((const  int*)data);
}

void PuresoftFBO::read16(int idx, void* dataAligned16Bytes) const
{
	mcemaths_quatcpy((float*)dataAligned16Bytes, (const float*)m_workRanges[idx].writePoint);
}

void PuresoftFBO::write16(int idx, const void* dataAligned16Bytes)
{
	if(!m_workRanges[idx].overflow)
		mcemaths_quatcpy((float*)m_workRanges[idx].writePoint, (const float*)dataAligned16Bytes);
}

void PuresoftFBO::directRead( int row,  int col, void* data, size_t bytes) const
{
	clampCoord(row, col);
	memcpy(data, (const void*)((uintptr_t)m_rowEntries[row] + col * m_elemLen), bytes);
}

void PuresoftFBO::directWrite( int row,  int col, const void* data, size_t bytes) // not thread safe
{
	if(validateCoord(row, col))
		memcpy((void*)((uintptr_t)m_rowEntries[row] + col * m_elemLen), data, bytes);
}

void PuresoftFBO::directRead1( int row,  int col, void* data) const
{
	clampCoord(row, col);
	*(( char*)data) = *((const char*)m_rowEntries[row] + col);
}

void PuresoftFBO::directWrite1( int row,  int col, const void* data) // not thread safe
{
	if(validateCoord(row, col))
		*((char*)m_rowEntries[row] + col) = *((const  char*)data);
}

void PuresoftFBO::directRead4( int row,  int col, void* data) const
{
	clampCoord(row, col);
	*(( int*)data) = *((const int*)m_rowEntries[row] + col);
}

void PuresoftFBO::directWrite4( int row,  int col, const void* data) // not thread safe
{
	if(validateCoord(row, col))
		*(( int*)m_rowEntries[row] + col) = *((const  int*)data);
}

void PuresoftFBO::directRead16( int row,  int col, void* dataAligned16Bytes) const // elemLen must be 16
{
	clampCoord(row, col);
	mcemaths_quatcpy((float*)dataAligned16Bytes, (const float*)((uintptr_t)m_rowEntries[row] + (col << 4)));
}

void PuresoftFBO::directWrite16( int row,  int col, const void* dataAligned16Bytes) // elemLen must be 16, not thread safe
{
	if(validateCoord(row, col))
		mcemaths_quatcpy((float*)((uintptr_t)m_rowEntries[row] + (col << 4)), (const float*)dataAligned16Bytes);
}

void PuresoftFBO::clear(const void* data, size_t bytes)
{
	 char* row = ( char*)m_buffer;

	for( int y = 0; y < m_height - 1; y++)
	{
		 char* col = ( char*)row;
		for( int x = 0; x < m_width; x++)
		{
			memcpy(col, data, bytes);
			col += m_elemLen;
		}
		row += m_scanline;
	}
}

void PuresoftFBO::clear1(const void* data)
{
	memset(m_buffer, *(( char*)data), m_bytes);
}

void PuresoftFBO::clear4(const void* data)
{
	 char* row = ( char*)m_buffer;

	for( int y = 0; y < m_height - 1; y++)
	{
		 char* col = ( char*)row;
		for( int x = 0; x < m_width; x++)
		{
			*(( int*)col) = *(( int*)data);
			col += m_elemLen;
		}
		row += m_scanline;
	}
}

void PuresoftFBO::clear16(const void* dataAligned16Bytes)
{
	assert(0 == m_bytes % 16);

	// clear16() requires every scanline starts at 16-byte boundary meaning the buffer length is a multiple of 16
	__asm{
		; load source data
		mov		eax,	dataAligned16Bytes
		movaps	xmm0,	[eax]
		; find loop times
		mov		eax,	this
		add		eax,	m_bytes
		mov		ecx,	[eax]
		shr		ecx,	4		; div 16
		; load dest address
		mov		eax,	this
		add		eax,	m_buffer
		mov		edx,	[eax]
		; fill buffer in sse way
lup:	movaps	[edx],	xmm0
		add		edx,	16
		loop	lup
	}
}

void PuresoftFBO::clearToZero(void)
{
	assert(0 == m_bytes % 16);

	__asm{
		; load source data
		xorps	xmm0, xmm0
		; find loop times
		mov		eax,	this
		add		eax,	m_bytes
		mov		ecx,	[eax]
		shr		ecx,	4		; div 16
		; load dest address
		mov		eax,	this
		add		eax,	m_buffer
		mov		edx,	[eax]
		; fill buffer in sse way
lup:	movaps	[edx],	xmm0
		add		edx,	16
		loop	lup
	}
}

void PuresoftFBO::setBuffer(void* externalBuffer /* = NULL */)
{
	if(externalBuffer)
	{
		if(!m_isExternalBuffer && m_buffer)
		{
			_aligned_free(m_buffer);
		}

		m_isExternalBuffer = true;
		m_buffer = externalBuffer;
	}
	else
	{
		if(m_isExternalBuffer || !m_buffer)
		{
			m_isExternalBuffer = false;
			if(NULL == (m_buffer = _aligned_malloc(m_bytes, 64)))
			{
				throw bad_alloc("PuresoftFBO::setBuffer");
			}
		}
	}

	uintptr_t p = (uintptr_t)m_buffer;
	for( int i = 0; i < m_height; i++)
	{
		m_rowEntries[i] = (void*)p;
		p += m_scanline;
	}
}

void* PuresoftFBO::getBuffer(void)
{
	return m_buffer;
}

const void* PuresoftFBO::getBuffer(void) const
{
	return m_buffer;
}

 int PuresoftFBO::getWidth(void) const
{
	return m_width;
}

 int PuresoftFBO::getHeight(void) const
{
	return m_height;
}

 int PuresoftFBO::getScanline(void) const
{
	return m_scanline;
}

 int PuresoftFBO::getElemLen(void) const
{
	return m_elemLen;
}

size_t PuresoftFBO::getBytes(void) const
{
	return m_bytes;
}

PuresoftFBO* PuresoftFBO::getExtraLayer(LAYER layer)
{
	return m_extraLayers[layer];
}

const PuresoftFBO* PuresoftFBO::getExtraLayer(LAYER layer) const
{
	return m_extraLayers[layer];
}

void PuresoftFBO::saveAsBmpFile(const wchar_t* path, bool dataIsFloat) const
{
	
	BITMAPFILEHEADER header1 = {0x4D42, 0, 0, 0, sizeof(BITMAPINFOHEADER)};
	BITMAPINFOHEADER header2 = {sizeof(BITMAPINFOHEADER), m_width, m_height, 1, ( short)m_elemLen * 8, 0, 0, 0, 0, 0, 0};

	FILE *fp;
	if(0 == _wfopen_s(&fp, path, L"w+b"))
	{
		if(dataIsFloat)
		{
			 int chans = m_elemLen / sizeof(float);
			 int scanline = (m_width * chans * 8 + 31) / 32 * 4;
			 int bytes = scanline * m_height;

			 char palette[1024];
			for(int i = 0; i < 256; i++)
			{
				palette[i * 4    ] = 
				palette[i * 4 + 1] = 
				palette[i * 4 + 2] = 
				palette[i * 4 + 3] = ( char)i;
			};

			header1.bfOffBits += sizeof(palette);
			header2.biBitCount = chans * 8;

			if(1 == chans)
			{
				header2.biCompression = BI_BITFIELDS;
			}

			 char* convertBuffer = ( char*)malloc(bytes);
			if(convertBuffer)
			{
				 char* destLine = convertBuffer;
				const  char* srcLine = (const  char*)m_buffer;
				for( int y = 0; y < m_height; y++)
				{
					for( int x = 0; x < m_width; x++)
					{
						for( int chan = 0; chan < chans; chan++)
						{
							destLine[x + chan] = ( char)(((const float*)srcLine)[x + chan] * 255.0f);
						}
					}

					destLine += scanline;
					srcLine += m_scanline;
				}

				fwrite(&header1, sizeof(header1), 1, fp);
				fwrite(&header2, sizeof(header2), 1, fp);
				fwrite(palette, sizeof(palette), 1, fp);
				fwrite(convertBuffer, bytes, 1, fp);
				free(convertBuffer);
			}
		}
		else
		{
			fwrite(&header1, sizeof(header1), 1, fp);
			fwrite(&header2, sizeof(header2), 1, fp);
			for(int y = m_height - 1; y >=0; y--)
				fwrite(m_rowEntries[y], m_scanline, 1, fp);
		}
		fclose(fp);
	}
}

void PuresoftFBO::saveAsRawFile(const wchar_t* path) const
{
	FILE *fp;
	if(0 == _wfopen_s(&fp, path, L"w+b"))
	{
		fwrite(m_buffer, m_bytes, 1, fp);
		fclose(fp);
	}
}

bool PuresoftFBO::clampCoord(int& row,  int& col) const
{
	bool overflow = false;

	if(CLAMP == m_wrapMode)
	{
		if(row > m_maxRow)
		{
			row = m_maxRow;
			overflow = true;
		}
		else if(row < 0)
		{
			row = 0;
			overflow = true;
		}
		
		if(col > m_maxCol)
		{
			col = m_maxCol;
			overflow = true;
		}
		else if(col < 0)
		{
			col = 0;
			overflow = true;
		}
	}
	else // WRAP
	{
		row = row % m_maxRow;

		if(row < 0)
			row += m_maxRow;

		col = col % m_maxCol;

		if(col < 0)
			col += m_maxCol;
	}

	return overflow;
}

bool PuresoftFBO::validateCoord(int row,  int col) const
{
	if(row > m_maxRow || row < 0)
		return false;

	if(col > m_maxCol || col < 0)
		return false;

	return true;
}