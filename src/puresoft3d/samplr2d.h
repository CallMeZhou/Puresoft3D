#pragma once
#include "mcemaths.hpp"

class PuresoftSampler2D : public mcemaths::align_base_16
{
public:
	PuresoftSampler2D(unsigned int width, unsigned int scanline, unsigned int height, unsigned int elemLen, const void* buffer);
	~PuresoftSampler2D(void);

	void get(float texcoordX, float texcoordY, void* data, size_t len) const;
	void get4(float texcoordX, float texcoordY, void* data) const;

private:
	const char* m_buffer;
	const char** m_rowEntries;
	unsigned int m_width;
	unsigned int m_scanline;
	unsigned int m_height;
	unsigned int m_elemLen;
	__declspec(align(16)) float m_rasterizingFactors[4];
	__declspec(align(16)) float m_locatingFactors[4];

	inline const char* locate(float texcoordX, float texcoordY) const;
};

