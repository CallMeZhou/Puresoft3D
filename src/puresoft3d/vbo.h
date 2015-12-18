#pragma once

class PuresoftVBO
{
public:
	__declspec(align(64)) struct WORKRANGE
	{
		unsigned char* current;
		unsigned char* end;
	};

	static const size_t MAX_WORKRANGES = 16;

public:
	PuresoftVBO(size_t unitBytes, size_t unitCount);
	~PuresoftVBO(void);
	void updateContent(const void* src);
	void rewindRanges(int idx = -1); // -1 means rewind all
	void evenOutRanges(int rangeAmount = MAX_WORKRANGES);
	const void* next(size_t idx = 0);

private:
	size_t m_unitBytes;
	size_t m_unitCount;
	size_t m_bufferBytes;
	void* m_buffer;

	WORKRANGE m_workRanges[MAX_WORKRANGES];
};

