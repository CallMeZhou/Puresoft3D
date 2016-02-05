#pragma once
#include <stdlib.h>

class PuresoftRenderer
{
public:
	virtual void  startup(uintptr_t canvasWindow, int width, int height) = 0;
	virtual void  shutdown(void) = 0;
	virtual void  setCanvas(uintptr_t canvasWindow) = 0;
	virtual void* swapBuffers(void) = 0;
	virtual void  release(void) = 0;
};