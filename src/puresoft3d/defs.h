#pragma once

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
	void* buff;
} PURESOFTIMGBUFF32;