#pragma once

class PuresoftFBO
{
public:
	PuresoftFBO(unsigned int width, unsigned int scanline, unsigned int height, unsigned int elemLen);
	~PuresoftFBO(void);

	void setCurRow(unsigned int row);
	void nextRow(void);
	void setCurCol(unsigned int col);
	void nextCol(void);
	void read(void* data, size_t bytes) const;
	void write(const void* data, size_t bytes);
	void read4(void* data) const;
	void write4(const void* data);
	void read16(void* dataAligned16Bytes) const;  // elemLen must be 16
	void write16(const void* dataAligned16Bytes); // elemLen must be 16

	// must accord with elemLen
	void clear(const void* data, size_t bytes);
	void clear4(const void* data);
	void clear16(const void* dataAligned16Bytes);

	void* getBuffer(void);
	const void* getBuffer(void) const;

	unsigned int getWidth(void) const;
	unsigned int getHeight(void) const;
	unsigned int getScanline(void) const;
	unsigned int getElemLen(void) const;

private:
	unsigned int m_width;
	unsigned int m_scanline;
	unsigned int m_height;
	unsigned int m_elemLen;
	size_t m_bytes;
	size_t m_curRowEntry;
	void* m_writePoint;
	void* m_buffer;
};

