#pragma once

typedef struct 
{
	size_t capacity; 
	void* data;
} PURESOFTUNIFORM;

typedef struct
{
	union
	{
		struct
		{
			unsigned char bgra[4];
		} ary;

		struct
		{
			unsigned char b;
			unsigned char g;
			unsigned char r;
			unsigned char a;
		} elems;

		unsigned int i32;
	};
} PURESOFTBGRA;

typedef struct
{
	unsigned int width;
	unsigned int scanline;
	unsigned int height;
	unsigned int elemLen;
	void* pixels;
} PURESOFTIMGBUFF32;