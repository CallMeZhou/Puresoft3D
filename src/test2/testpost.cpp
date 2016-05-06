#include <intrin.h>
#include <memory.h>
#include <math.h>
#include "defs.h"
#include "mcemaths.h"
#include "testpost.h"
#include "fbo.h"

void PP_DepthofField::process(int threadIndex, int threadCount, PuresoftFBO* frame, PuresoftFBO* depth)
{
	// buffer entry for this thread
	uintptr_t frameBuffer = (uintptr_t)frame->getBuffer();
	int scanline = frame->getScanline();
	frameBuffer += threadIndex * scanline;

	const unsigned char f[] = {50,50,50,50,50,50,50,50};
	__asm{
		lea eax,f
			movq mm2,[eax]
	}

	for(int y = threadIndex; 
		y < frame->getHeight(); 
		y += threadCount)
	{
		PURESOFTBGRA* row = (PURESOFTBGRA*)frameBuffer;
		for(int x = 0; x < frame->getWidth(); x+=2)
		{
			__asm{
				mov eax,1
					movd mm1,eax
					mov edx,row
					movq mm0,[edx]
				paddb mm0,mm2
					movntq [edx],mm0
			}
			row+=2;
		}

		frameBuffer += scanline * threadCount;
	}
	_mm_empty();
}