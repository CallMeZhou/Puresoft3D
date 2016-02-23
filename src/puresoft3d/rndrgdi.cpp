#include <windows.h>
#include <wtypes.h>
#include <GdiPlus.h>
#include <assert.h>
#include <process.h>
#include <stdexcept>
#include "rndrgdi.h"

using namespace Gdiplus;
using namespace std;

PuresoftGdiRenderer::PuresoftGdiRenderer(void)
{
	m_width = m_height = 0;
	memset(m_frameBuffers, 0, sizeof(m_frameBuffers));
	m_backBuffer = m_visibleBuffer = -1;
	m_startRenderingEvent = m_renderingCompletionEvent = m_renderingThread = 0;
}


PuresoftGdiRenderer::~PuresoftGdiRenderer(void)
{
	assert(0 == m_renderingThread);
}

void PuresoftGdiRenderer::startup(uintptr_t canvasWindow, int width, int height)
{
	assert(0 == m_renderingThread);

	m_canvasWindow = canvasWindow;
	m_width = width;
	m_height = height;

	size_t bytes = width * 4 * height;

	if(NULL == (m_frameBuffers[0] = _aligned_malloc(bytes, 16)))
	{
		throw bad_alloc("PuresoftGdiRenderer::startup m_frameBuffers[0]");
	}

	if(NULL == (m_frameBuffers[1] = _aligned_malloc(bytes, 16)))
	{
		throw bad_alloc("PuresoftGdiRenderer::startup m_frameBuffers[1]");
	}

	memset(m_frameBuffers[0], 0, bytes);
	memset(m_frameBuffers[1], 0, bytes);

	m_backBuffer = 0;
	m_visibleBuffer = 1;

	if((uintptr_t)INVALID_HANDLE_VALUE == (m_startRenderingEvent = (uintptr_t)CreateEventW(NULL, FALSE, FALSE, NULL)))
	{
		throw bad_alloc("PuresoftGdiRenderer::startup m_startRenderingEvent");
	}

	if((uintptr_t)INVALID_HANDLE_VALUE == (m_renderingCompletionEvent = (uintptr_t)CreateEventW(NULL, FALSE, TRUE, NULL)))
	{
		throw bad_alloc("PuresoftGdiRenderer::startup m_renderingCompletionEvent");
	}

	m_renderingThread = _beginthreadex(NULL, 0, renderingThread, this, 0, NULL);
}

void PuresoftGdiRenderer::shutdown(void)
{
	assert(m_renderingThread);

	WaitForSingleObject((HANDLE)m_renderingCompletionEvent, INFINITE);

	m_visibleBuffer = -1;
	SetEvent((HANDLE)m_startRenderingEvent);

	WaitForSingleObject((HANDLE)m_renderingThread, INFINITE);

	CloseHandle((HANDLE)m_startRenderingEvent);
	CloseHandle((HANDLE)m_renderingCompletionEvent);
	CloseHandle((HANDLE)m_renderingThread);
	_aligned_free(m_frameBuffers[0]);
	_aligned_free(m_frameBuffers[1]);

	m_width = m_height = 0;
	memset(m_frameBuffers, 0, sizeof(m_frameBuffers));
	m_backBuffer = m_visibleBuffer = -1;
	m_startRenderingEvent = m_renderingCompletionEvent = m_renderingThread = 0;
}

void PuresoftGdiRenderer::setCanvas(uintptr_t canvasWindow)
{
	m_canvasWindow = canvasWindow;
}

void PuresoftGdiRenderer::getDesc(PURESOFTIMGBUFF32* desc)
{
	desc->width = (unsigned int)m_width;
	desc->height = (unsigned int)m_height;
	desc->scanline = desc->width * 4;
	desc->elemLen = 4;
}

void* PuresoftGdiRenderer::swapBuffers(void)
{
	WaitForSingleObject((HANDLE)m_renderingCompletionEvent, INFINITE);

	int temp = m_backBuffer;
	m_backBuffer = m_visibleBuffer;
	m_visibleBuffer = temp;
	SetEvent((HANDLE)m_startRenderingEvent);

	return m_frameBuffers[m_backBuffer];
}

void PuresoftGdiRenderer::release(void)
{
	delete this;
}

unsigned __stdcall PuresoftGdiRenderer::renderingThread(void *param)
{
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	PuresoftGdiRenderer* pThis = (PuresoftGdiRenderer*)param;
	int width = pThis->m_width;
	int stride = width * 4;
	int height = pThis->m_height;

	while(true)
	{
		WaitForSingleObject((HANDLE)pThis->m_startRenderingEvent, INFINITE);

		if(pThis->m_visibleBuffer < 0)
		{
			break;
		}

		if(0 == pThis->m_canvasWindow)
		{
			SetEvent((HANDLE)pThis->m_renderingCompletionEvent);
			continue;
		}

		HDC hdc = GetDC((HWND)pThis->m_canvasWindow);

		Graphics grp(hdc);
		grp.DrawImage(&Bitmap(width, height, stride, PixelFormat32bppRGB, (BYTE*)pThis->m_frameBuffers[pThis->m_visibleBuffer]), 0, 0);

		ReleaseDC((HWND)pThis->m_canvasWindow, hdc);

		SetEvent((HANDLE)pThis->m_renderingCompletionEvent);
	}

	GdiplusShutdown(gdiplusToken);
	return 0;
}
