#pragma once
#include "rndr.h"

class PuresoftGdiRenderer : public PuresoftRenderer
{
public:
	PuresoftGdiRenderer(void);
	~PuresoftGdiRenderer(void);

	void  startup(uintptr_t canvasWindow, int width, int height);
	void  shutdown(void);
	void  setCanvas(uintptr_t canvasWindow);
	void  getDesc(PURESOFTIMGBUFF32* desc);
	void* swapBuffers(void);
	void  release(void);

private:
	int m_width;
	int m_height;
	volatile uintptr_t m_canvasWindow;
	void* m_frameBuffers[2];
	int m_backBuffer;
	volatile int m_visibleBuffer;
	uintptr_t m_startRenderingEvent;
	uintptr_t m_renderingCompletionEvent;
	uintptr_t m_renderingThread;
	void prepareVisibleFBOs(void);
	void destroyVisibleFBOs(void);
	static unsigned __stdcall renderingThread(void *param);
};

