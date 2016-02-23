#pragma once
#include <stddef.h>
#include "rndr.h"

class PuresoftDDrawRenderer : public PuresoftRenderer
{
public:
	PuresoftDDrawRenderer(void);
	~PuresoftDDrawRenderer(void);

	void  startup(uintptr_t canvasWindow, int width, int height);
	void  shutdown(void);
	void  setCanvas(uintptr_t canvasWindow);
	void  getDesc(PURESOFTIMGBUFF32* desc);
	void* swapBuffers(void);
	void  release(void);

private:
	int m_width;
	int m_height;
	uintptr_t m_dll;
	uintptr_t m_canvas;
	uintptr_t m_ddraw;
	uintptr_t m_primary;
	uintptr_t m_back;
	uintptr_t m_visible;
};

