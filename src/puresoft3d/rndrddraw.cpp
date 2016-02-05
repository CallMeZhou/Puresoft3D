#include <windows.h>
#include <InitGuid.h>
#include <atlbase.h>
#include <ddraw.h>
#include <exception>
#include <assert.h>
#include "rndrddraw.h"
using namespace std;

static void throwHResultException(const char* file, size_t line, HRESULT hr)
{
	char errmsg[1024];

	sprintf_s(errmsg, 1024, "%s, %lu: HRESULT=0x%X", file, line, hr);

	throw bad_exception(errmsg);
}

#define THROWHRESULTEXCEPTION(hr) throwHResultException(__FILE__, __LINE__, hr)
#define THROWIFFAILED(comcall) if(FAILED(hr = (comcall))) THROWHRESULTEXCEPTION(hr)

PuresoftDDrawRenderer::PuresoftDDrawRenderer(void)
{
	m_width = m_height = 0;
	m_ddraw = m_primary = m_back = m_visible = m_dll = 0;
}

PuresoftDDrawRenderer::~PuresoftDDrawRenderer(void)
{
	assert(0 == m_ddraw);
}

void PuresoftDDrawRenderer::startup(uintptr_t canvasWindow, int width, int height)
{
	assert(0 == m_ddraw);
	assert(canvasWindow);

	if(NULL == (m_dll = (uintptr_t)LoadLibraryA("ddraw.dll")))
		THROWHRESULTEXCEPTION(GetLastError());

	typedef HRESULT (WINAPI *fpDirectDrawCreateEx)( GUID FAR * lpGuid, LPVOID *lplpDD, REFIID iid,IUnknown FAR *pUnkOuter);
	fpDirectDrawCreateEx __DirectDrawCreateEx;
	if(NULL == (__DirectDrawCreateEx = (fpDirectDrawCreateEx)GetProcAddress((HMODULE)m_dll, "DirectDrawCreateEx")))
		THROWHRESULTEXCEPTION(GetLastError());

	HRESULT hr;

	CComPtr<IDirectDraw7> ddraw;
	THROWIFFAILED(__DirectDrawCreateEx(NULL, (void**)&ddraw, IID_IDirectDraw7, NULL));

	HWND topLevelParent = (HWND)canvasWindow;
	while(GetParent(topLevelParent))
		topLevelParent = GetParent(topLevelParent);

	THROWIFFAILED(ddraw->SetCooperativeLevel(topLevelParent, DDSCL_NORMAL));

	CComPtr<IDirectDrawClipper> clipper;
	THROWIFFAILED(ddraw->CreateClipper(0, &clipper, NULL));
	THROWIFFAILED(clipper->SetHWnd(0, (HWND)canvasWindow));
	
	DDSURFACEDESC2 ddsc;
	memset(&ddsc, 0, sizeof(DDSURFACEDESC2));
	ddsc.dwSize = sizeof(DDSURFACEDESC2);
	ddsc.dwFlags = DDSD_CAPS;
	ddsc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_VIDEOMEMORY;
	CComPtr<IDirectDrawSurface7> primary;
	THROWIFFAILED(ddraw->CreateSurface(&ddsc, &primary, NULL));
	THROWIFFAILED(primary->SetClipper(clipper));

	ddsc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PITCH | DDSD_CAPS;
	ddsc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
	ddsc.dwWidth = width;
	ddsc.lPitch = width * 4;
	ddsc.dwHeight = height;
	CComPtr<IDirectDrawSurface7> back;
	THROWIFFAILED(ddraw->CreateSurface(&ddsc, &back, NULL));
	CComPtr<IDirectDrawSurface7> visible;
	THROWIFFAILED(ddraw->CreateSurface(&ddsc, &visible, NULL));

	m_ddraw = (uintptr_t)ddraw.Detach();
	m_primary = (uintptr_t)primary.Detach();
	m_back = (uintptr_t)back.Detach();
	m_visible = (uintptr_t)visible.Detach();
	m_canvas = canvasWindow;
	m_width = width;
	m_height = height;
}

void PuresoftDDrawRenderer::shutdown(void)
{
	assert(m_ddraw);

	((IDirectDrawSurface7*)m_back)->Release();
	((IDirectDrawSurface7*)m_visible)->Release();
	((IDirectDrawSurface7*)m_primary)->Release();
	((IDirectDrawSurface7*)m_ddraw)->Release();

	FreeLibrary((HMODULE)m_dll);

	m_width = m_height = 0;
	m_ddraw = m_primary = m_back = m_visible = m_dll = 0;
}

void PuresoftDDrawRenderer::setCanvas(uintptr_t canvasWindow)
{
	HRESULT hr;
	CComPtr<IDirectDrawClipper> clipper;
	THROWIFFAILED(((IDirectDrawSurface7*)m_primary)->GetClipper(&clipper));
	THROWIFFAILED(clipper->SetHWnd(0, (HWND)canvasWindow));
	m_canvas = canvasWindow;
}

void* PuresoftDDrawRenderer::swapBuffers(void)
{
	((IDirectDrawSurface7*)m_back)->Unlock(NULL);

	uintptr_t temp = m_back;
	m_back = m_visible;
	m_visible = temp;

	RECT destRect;
	GetClientRect((HWND)m_canvas, &destRect);
	ClientToScreen((HWND)m_canvas, (LPPOINT)&destRect);
	destRect.right = destRect.left + m_width - 1;
	destRect.bottom = destRect.top + m_height - 1;
	
	if(DDERR_SURFACELOST == ((IDirectDrawSurface7*)m_primary)->Blt(&destRect, (IDirectDrawSurface7*)m_visible, NULL, DDBLT_WAIT, NULL))
	{
		if(DDERR_SURFACELOST == ((IDirectDrawSurface7*)m_visible)->IsLost())
			((IDirectDrawSurface7*)m_visible)->Restore();
		if(DDERR_SURFACELOST == ((IDirectDrawSurface7*)m_back)->IsLost())
			((IDirectDrawSurface7*)m_back)->Restore();
		if(DDERR_SURFACELOST == ((IDirectDrawSurface7*)m_primary)->IsLost())
			((IDirectDrawSurface7*)m_primary)->Restore();
	}


	DDSURFACEDESC2 ddsc;
	memset(&ddsc, 0, sizeof(DDSURFACEDESC2));
	ddsc.dwSize = sizeof(DDSURFACEDESC2);
	HRESULT hr;
	// this line must throw on failure to avoid returning null pointer
	THROWIFFAILED(((IDirectDrawSurface7*)m_back)->Lock(NULL, &ddsc, DDLOCK_WAIT | DDLOCK_NOSYSLOCK, NULL));

	return ddsc.lpSurface;
}

void PuresoftDDrawRenderer::release(void)
{
	delete this;
}