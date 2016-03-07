#include <windowsx.h>
#include <atlbase.h>
#include <atlconv.h>
#include <atlwin.h>
#include <WTypes.h>
#include <tchar.h>
#include <GdiPlus.h>

#include "fixvec.hpp"
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
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
//static void updateTexMatrix(mat4& m, float rad);

const int W = 1024;
const int H = 576;

const int SHDW_W = 2048;
const int SHDW_H = 2048;

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
	PuresoftRenderer* ddrawRender = NULL;
	try
	{
		ddrawRender = new PuresoftDDrawRenderer;
	}
	catch(...)
	{
		MessageBoxW(hWnd, L"Failed to initialize DDraw device. Fall back to GDI renderer.", L"Puresoft 3D", MB_OK);
		ddrawRender = NULL;
	}
	
	PuresoftPipeline pipeline((uintptr_t)hWnd, W, H, ddrawRender);//new PuresoftDDrawRenderer);

	vec4 lightPos(-3.0f, 0, 1.0f, 0), cameraPos(0, 0, 1.2f, 0);
	pipeline.setUniform(7, &lightPos, sizeof(lightPos));
	pipeline.setUniform(8, &cameraPos, sizeof(cameraPos));

	mat4 view, proj, proj_view;
	mcemaths_make_proj_perspective(proj, 1.0f, 300.0f, (float)W / H, 2 * PI * (30.0f / 360.0f));
	view.translation(-cameraPos.x, -cameraPos.y, -cameraPos.z);
	mcemaths_transform_m4m4(proj_view, proj, view);

	//////////////////////////////////////////////////////////////////////////
	// scene objects
	//////////////////////////////////////////////////////////////////////////
	SceneObject root(pipeline, NULL);
	Earth earth(pipeline, &root);
	Moon moon(pipeline, &root);
	Skybox skybox(pipeline, &root);

	//////////////////////////////////////////////////////////////////////////
	// shadow
	//////////////////////////////////////////////////////////////////////////
	mat4 light1View, light1Proj, light1pv, light1pvb;
//	mcemaths_make_proj_perspective(light1Proj, 1.0f, 300.0f, (float)SHDW_W / SHDW_H, 2 * PI * (45.0f / 360.0f));
	mcemaths_make_proj_orthographic(light1Proj, -10.0f, 20.0f, -3.0f, 3.0f, -3.0f, 3.0f);
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

	int progShadow = pipeline.createProgramme(
		pipeline.addProcessor(new VertexProcesserDEF05), 
		pipeline.addProcessor(new InterpolationProcessorDEF05), 
		pipeline.addProcessor(new FragmentProcessorDEF05));

	//////////////////////////////////////////////////////////////////////////
	// run main window's message loop
	//////////////////////////////////////////////////////////////////////////
	DWORD time0 = GetTickCount(), fcount = 0;
	MSG msg;
	mat4 rootTransform;
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
		SceneObject::m_usePrivateProgramme = false;
		pipeline.useProgramme(progShadow);

		earth.draw(pipeline);
		moon.draw(pipeline);

		// draw scene
		pipeline.setUniform(0, proj, sizeof(proj.elem));
		pipeline.setUniform(1, view, sizeof(view.elem));
		pipeline.setUniform(3, proj_view, sizeof(proj_view.elem));
		pipeline.setDepth();
		pipeline.clearDepth();
		pipeline.setViewport(W, H);
		SceneObject::m_usePrivateProgramme = true;
		SceneObject::m_shadowMaps[0] = texShadow;
		mcemaths_mat4cpy(SceneObject::m_shadowPVs[0], light1pvb);

		skybox.draw(pipeline);
		earth.draw(pipeline);
		moon.draw(pipeline);

		pipeline.swapBuffers();

// 		pipeline.setUniform(0, light1Proj, sizeof(light1Proj.elem));
// 		pipeline.setUniform(1, light1View, sizeof(light1View.elem));
// 		pipeline.setDepth(texShadow);
// 		pipeline.setViewport(SHDW_W, SHDW_H);



		fcount++;
		DWORD timeSpan = GetTickCount() - time0;
		if(timeSpan > 2000)
		{
			char frate[64];
			sprintf_s(frate, 64, "%.1f", 1000.0f * (float)fcount / (float)timeSpan);
			SetWindowTextA(hWnd, frate);

			fcount = 0;
			time0 = GetTickCount();
		}
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
	case WM_TIMER:
		break;
	default:
		return DefWindowProc(wnd, message, wParam, lParam);
	}
	return 0;
}
/*
static void updateTexMatrix(mat4& m, float rad)
{
	mat4 push, pop, rot, temp;
	push.translation(vec4(0.5f, 0.5f, 0, 0));
	pop.translation(vec4(-0.5f, -0.5f, 0, 0));
	rot.rotation(vec4(0, 0, 1.0f, 0), rad);
	
	mcemaths_transform_m4m4(temp, rot, pop);
	mcemaths_transform_m4m4(m, push, temp);
}


static float rotRad = 0, trotrad = 0, rotRad3 = 2 * PI;

static void drawObjects(PuresoftPipeline& pipeline, const mat4& proj, const mat4& view)
{

	rotRad += 0.1f * (float)highTimer.Now() / 1000.0f;
	trotrad += 0.02f * (float)highTimer.Now() / 1000.0f;
	highTimer.Start();

	if(rotRad > 2 * PI)
	{
		rotRad = 0;
	}

	if(trotrad > 1.0f)
	{
		trotrad = 0;
	}

	pipeline.setUniform(0, proj, sizeof(proj.elem));
	pipeline.setUniform(1, view, sizeof(view.elem));
	pipeline.setDepth();
	pipeline.setViewport(W, H);

	pipeline.useProgramme(skyProc);

	pipeline.disable(BEHAVIOR_UPDATE_DEPTH | BEHAVIOR_TEST_DEPTH);
	pipeline.drawVAO(&vao2, true);
	pipeline.enable(BEHAVIOR_UPDATE_DEPTH | BEHAVIOR_TEST_DEPTH);

	pipeline.useProgramme(fullProc);

	mat4 rot;
	rot.rotation(vec4(0, 1.0f, 0, 0), rotRad);
	//scale.scaling(1.0f, 1.0f, 1.0f);
	//tran.translation(0, 0, -150.0f);
	tran.translation(0, 0, -1.2f);
	mcemaths_transform_m4m4(model, rot, scale);
	mcemaths_transform_m4m4_r_ip(tran, model);
	pipeline.setUniform(4, model, sizeof(model.elem));
	mat4 modelRotate = rot;
	pipeline.setUniform(5, modelRotate, sizeof(modelRotate.elem));

	mat4 texMatrix;
	texMatrix.translation(trotrad, 0, 0);
	pipeline.setUniform(14, texMatrix, sizeof(texMatrix.elem));

	pipeline.setUniform(9, &texEarthDiff, sizeof(texEarthDiff));
	pipeline.setUniform(10, &texEarthBump, sizeof(texEarthBump));

	pipeline.clearDepth();

	pipeline.drawVAO(&vao1);

	mat4 scale3, tran3, rot3;
	scale3.scaling(0.07f, 0.07f, 0.07f);
	tran3.translation(0.6f, 0, 0);
	mcemaths_transform_m4m4(model, tran3, scale3);
	rotRad3 = 1.3f * PI;
	//		rotRad3 -= 0.4f * (float)highTimer.Now() / 1000.0f;
	//		if(rotRad3 < 0)
	//		{
	//			rotRad3 = 2 * PI;
	//		}
	rot3.rotation(vec4(0, 1.0f, 0, 0), rotRad3);
	mcemaths_transform_m4m4_r_ip(rot3, model);
	mcemaths_transform_m4m4_r_ip(tran, model);
	pipeline.setUniform(4, model, sizeof(model.elem));
	pipeline.setUniform(5, rot3, sizeof(modelRotate.elem));

	pipeline.setUniform(9, &texMoonDiff, sizeof(texEarthDiff));
	pipeline.setUniform(10, &texMoonBump, sizeof(texEarthBump));

	pipeline.useProgramme(simpTexProc);
	pipeline.drawVAO(&vao1);

	pipeline.swapBuffers();
}*/