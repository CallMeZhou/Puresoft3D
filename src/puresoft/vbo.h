#pragma once

class PuresoftVBO
{
public:
	PuresoftVBO(size_t unitBytes, size_t unitCount);
	~PuresoftVBO(void);
	void updateContent(const void* src);
	void rewind(void);
	const void* next(void);

private:
	size_t m_unitBytes;
	size_t m_bufferBytes;
	void* m_buffer;
	unsigned char* m_currentPosition;
	unsigned char* m_endPosition;
};

