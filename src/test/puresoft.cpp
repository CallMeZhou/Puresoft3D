#include <windowsx.h>
#include <atlbase.h>
#include <atlconv.h>
#include <atlwin.h>
#include <WTypes.h>
#include <tchar.h>
#include <GdiPlus.h>

#if(_MSC_VER <= 1600) // VS2010 or ealier
#include "fixvec.hpp"
#else
#include <vector>
#endif
#include <map>

#include "pipeline.h"
#include "picldr.h"
#include "rndrddraw.h"
#include "libobjx.h"
#include "defproc.h"
#include "testproc.h"
#include "testobjs.h"

using namespace Gdiplus;
using namespace std;


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

const int W = 800;
const int H = 500;

const int SHDW_W = 480;
const int SHDW_H = 480;

const float PI = 3.1415927f;

ALIGN16 static const float bias[16] = 
{
	0.5f,    0,    0,    0, 
	0,    0.5f,    0,    0, 
	0,       0, 1.0f,    0, 
	0.5f, 0.5f, 0, 1.0f
};


class HighResolutionTimeCounter
{
	LARGE_INTEGER m_start;
	LARGE_INTEGER m_freq;
public:
	HighResolutionTimeCounter() 
	{
		QueryPerformanceFrequency(&m_freq);
		reset();
	}

	void reset() 
	{
		QueryPerformanceCounter(&m_start);
	}

	__int64 span()
	{ 
		LARGE_INTEGER end = { 0 };
		QueryPerformanceCounter(&end);
		return (end.QuadPart - m_start.QuadPart)*1000 / m_freq.QuadPart;
	}
};

HighResolutionTimeCounter highTimer;

int APIENTRY _tWinMain(HINSTANCE inst, HINSTANCE, LPTSTR, int nCmdShow)
{
	USES_CONVERSION;

	//////////////////////////////////////////////////////////////////////////
	// create and show main window
	//////////////////////////////////////////////////////////////////////////
	WNDCLASSEX wcex = {0};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.hInstance		= inst;
	wcex.hIcon			= NULL;//LoadIcon(inst, MAKEINTRESOURCE(IDI_PURESOFT));
	wcex.hCursor		= NULL;//LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;//MAKEINTRESOURCE(IDC_PURESOFT);
	wcex.lpszClassName	= _T("mainwnd");
	wcex.hIconSm		= NULL;//LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	RegisterClassEx(&wcex);

	RECT wndrect = {0, 0, W, H};
	AdjustWindowRect(&wndrect, WS_OVERLAPPEDWINDOW, FALSE);
	HWND hWnd = CreateWindow(_T("mainwnd"), _T("test"), WS_OVERLAPPEDWINDOW, wndrect.left, wndrect.top, 
		wndrect.right - wndrect.left, wndrect.bottom - wndrect.top, NULL, NULL, inst, NULL);
	CWindow(hWnd).CenterWindow();
	ShowWindow(hWnd, nCmdShow);

	//////////////////////////////////////////////////////////////////////////
	// basic initialization of pipeline
	PuresoftRenderer* ddrawRender = new PuresoftDDrawRenderer;
	try
	{
		ddrawRender->startup((uintptr_t)hWnd, W, H);
	}
	catch(...)
	{
		MessageBoxW(hWnd, L"Failed to initialize DDraw device. Fall back to GDI renderer.", L"Puresoft 3D", MB_OK);
		ddrawRender = NULL;
	}
	
	PuresoftPipeline pipeline((uintptr_t)hWnd, W, H, ddrawRender);

	vec4 lightPos(-3.0f, 0, 1.0f, 0), cameraPos(0, 0, 2.2f, 0);
	pipeline.setUniform(7, &lightPos, sizeof(lightPos));
	pipeline.setUniform(8, &cameraPos, sizeof(cameraPos));

	mat4 view, proj, proj_view;
	mcemaths_make_proj_perspective(proj, 0.1f, 10.0f, (float)W / H, 2 * PI * (30.0f / 360.0f));
	view.translation(-cameraPos.x, -cameraPos.y, -cameraPos.z);
	mcemaths_transform_m4m4(proj_view, proj, view);

	//////////////////////////////////////////////////////////////////////////
	// scene objects
	//////////////////////////////////////////////////////////////////////////
	SceneObject root(pipeline, NULL);
	Earth earth(pipeline, &root);
	Cloud cloud(pipeline, &root);
	Moon moon(pipeline, &root);
	Skybox skybox(pipeline, &root);

	//////////////////////////////////////////////////////////////////////////
	// shadow
	//////////////////////////////////////////////////////////////////////////
	mat4 light1View, light1Proj, light1pv, light1pvb;
	mcemaths_make_proj_perspective(light1Proj, 0.1f, 10.0f, (float)SHDW_W / SHDW_H, 2 * PI * (30.0f / 360.0f));
	mcemaths_make_view_traditional(light1View, lightPos, vec4(), vec4(0, 1.0f, 0, 0));
	mcemaths_transform_m4m4(light1pv, light1Proj, light1View);
	mcemaths_transform_m4m4(light1pvb, bias, light1pv);
	PURESOFTIMGBUFF32 shadowBuffer;
	shadowBuffer.width = SHDW_W;
	shadowBuffer.height = SHDW_H;
	shadowBuffer.elemLen = 4; // 1 float
	shadowBuffer.scanline = SHDW_W * 4;
	shadowBuffer.pixels = NULL;
	int texShadow = pipeline.createTexture(&shadowBuffer);

	SceneObject::m_defaultShadowProgramme = pipeline.createProgramme(
		pipeline.addProcessor(new VertexProcesserDEF05), 
		pipeline.addProcessor(new InterpolationProcessorDEF05), 
		pipeline.addProcessor(new FragmentProcessorDEF05));

	//////////////////////////////////////////////////////////////////////////
	// run main window's message loop
	//////////////////////////////////////////////////////////////////////////
	DWORD time0 = GetTickCount(), fcount = 0;
	MSG msg;
	mat4 rootTransform;
	char perf[1024];
	float fps = 0;
	while (true)
	{
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if(WM_QUIT == msg.message)
			break;

		// update objects' positions
		root.update((float)highTimer.span() / 1000.0f, rootTransform);
		highTimer.reset();

		// create shadow map
		pipeline.setUniform(0, light1Proj, sizeof(light1Proj.elem));
		pipeline.setUniform(1, light1View, sizeof(light1View.elem));
		pipeline.setUniform(3, light1pv, sizeof(light1pv.elem));
		pipeline.setDepth(texShadow);
		pipeline.clearDepth();
		pipeline.setViewport(SHDW_W, SHDW_H);
		SceneObject::m_useShadowProgramme = true;

		earth.draw(pipeline);
		moon.draw(pipeline);
		cloud.draw(pipeline);

		// draw scene
		pipeline.setUniform(0, proj, sizeof(proj.elem));
		pipeline.setUniform(1, view, sizeof(view.elem));
		pipeline.setUniform(3, proj_view, sizeof(proj_view.elem));
		pipeline.setDepth();
		pipeline.clearDepth();
		pipeline.setViewport(W, H);
		SceneObject::m_useShadowProgramme = false;
		SceneObject::m_shadowMaps[0] = texShadow;
		mcemaths_mat4cpy(SceneObject::m_shadowPVs[0], light1pvb);

		skybox.draw(pipeline);
		earth.draw(pipeline);
		moon.draw(pipeline);
		cloud.draw(pipeline);

		pipeline.swapBuffers();

		unsigned int pushCalled, pushSpinned, popCalled, popSpinned;
		pipeline.getTaskQCounters(&pushCalled, &pushSpinned, &popCalled, &popSpinned);
		sprintf(perf, "pushCalled: %lu, pushSpinned: %lu, popCalled: %lu, popSpinned: %lu, fps: %.1f", pushCalled, pushSpinned, popCalled, popSpinned, fps);

		fcount++;
		DWORD timeSpan = GetTickCount() - time0;
		if(timeSpan > 2000)
		{
			fps = 1000.0f * (float)fcount / (float)timeSpan;
			fcount = 0;
			time0 = GetTickCount();
		}

		SetWindowTextA(hWnd, perf);
	}

	return (int) msg.wParam;
}

LRESULT CALLBACK WndProc(HWND wnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		break;
	case WM_SIZE:
		break;;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;
			hdc = BeginPaint(wnd, &ps);
			EndPaint(wnd, &ps);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(wnd, message, wParam, lParam);
	}
	return 0;
}