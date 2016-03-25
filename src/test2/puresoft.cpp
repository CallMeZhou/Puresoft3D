#include <windowsx.h>
#include <atlbase.h>
#include <atlconv.h>
#include <atlwin.h>
#include <WTypes.h>
#include <tchar.h>
#include "fixvec.hpp"
#include "mcemaths.hpp"
#include "pipeline.h"
#include "rndrddraw.h"
#include "testproc.h"
#include "loadscene.h"
#include "input.h"

using namespace std;
using namespace mcemaths;


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

const int W = 1024;
const int H = 768;

const int SHDW_W = 2048;
const int SHDW_H = 2048;

const float PI = 3.1415927f;

ALIGN16 static const float bias[16] = 
{
	0.5f,    0,    0,    0, 
	0,    0.5f,    0,    0, 
	0,       0, 1.0f,    0, 
	0.5f, 0.5f,    0, 1.0f
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

	// start keyboard and mouse input module
	Input input;
	input.startup(hWnd);

	//////////////////////////////////////////////////////////////////////////
	// basic initialization of pipeline
	//////////////////////////////////////////////////////////////////////////

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

	// make projection matrix
	mat4 view, proj, proj_view;
	mcemaths_make_proj_perspective(proj, 0.1f, 5.0f, (float)W / H, 2 * PI * (45.0f / 360.0f));

	//////////////////////////////////////////////////////////////////////////
	// load scene
	//////////////////////////////////////////////////////////////////////////

	scene_desc sceneDesc;
	SceneObject::SceneObjects scene;
	SceneObject::loadScene("plane.objx", &pipeline, scene, sceneDesc);
	SceneObject& root = SceneObject::getRoot(scene);

	vec4 light1from, light1to, light1RDir;
	SceneObject::getLightSource1(scene, light1from, light1to);
	mcemaths_sub_3_4(light1RDir, light1from, light1to);
	mcemaths_norm_3_4(light1RDir);

	vec4 cameraPos, cameraYPR;
	cameraPos = sceneDesc.camera_pos;
	cameraYPR = sceneDesc.camera_ypr;

	//////////////////////////////////////////////////////////////////////////
	// shadow
	//////////////////////////////////////////////////////////////////////////

	mat4 light1View, light1Proj, light1pv, light1pvb;
	mcemaths_make_proj_perspective(light1Proj, 0.1f, 5.0f, (float)SHDW_W / SHDW_H, 2 * PI * (90.0f / 360.0f));
	mcemaths_make_view_traditional(light1View, light1from, light1to, vec4(0, 1.0f, 0, 0));
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
		pipeline.addProcessor(new VP_Shadow), 
		pipeline.addProcessor(new IP_Null), 
		pipeline.addProcessor(new FP_Null));

	//////////////////////////////////////////////////////////////////////////
	// run main window's message loop
	//////////////////////////////////////////////////////////////////////////

	DWORD time0 = GetTickCount(), fcount = 0;
	mat4 rootTransform;
	
	while (true)
	{
		bool quit = false;
		MSG msg;
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if(WM_QUIT == msg.message)
				quit = true;
		}
		if(quit)
			break;

		//////////////////////////////////////////////////////////////////////////
		// render shadow map
		//////////////////////////////////////////////////////////////////////////

		// place objects by the view matrix of light source
		root.update((float)highTimer.span() / 1000.0f, rootTransform, light1pv);

		// draw call for shadow map
		pipeline.setUniform(2, light1View, sizeof(mat4));
		pipeline.setUniform(3, light1Proj, sizeof(mat4));
		pipeline.setUniform(4, light1pv, sizeof(mat4));
		pipeline.setDepth(texShadow);
		pipeline.clearDepth();
		pipeline.setViewport(SHDW_W, SHDW_H);
		SceneObject::m_usePrivateProgramme = false;
		pipeline.useProgramme(progShadow);

		for(SceneObject::SceneObjects::iterator it = scene.begin(); it != scene.end(); it++)
		{
			if(-1 != it->first.find("@noshadow"))
				continue;

			it->second.draw(pipeline);
		}

		// un-comment these if you wanna see shadow map
//		pipeline.saveTexture(-1, L"c:\\shadow.bmp", true);
//		break;

		//////////////////////////////////////////////////////////////////////////
		// render real scene
		//////////////////////////////////////////////////////////////////////////

		// place objects by the camera matrix
		view.view(cameraPos, cameraYPR);
		mcemaths_transform_m4m4(proj_view, proj, view);
		root.update(0, rootTransform, proj_view);

		// draw call for real scene
		pipeline.setUniform(2, view, sizeof(mat4));
		pipeline.setUniform(3, proj, sizeof(mat4));
		pipeline.setUniform(4, proj_view, sizeof(mat4));
		pipeline.setUniform(6, light1pvb, sizeof(mat4));
		pipeline.setUniform(20, light1from, sizeof(vec4));
		pipeline.setUniform(21, light1RDir, sizeof(vec4));
		pipeline.setUniform(22, cameraPos, sizeof(vec4));
		pipeline.setUniform(23, &texShadow, sizeof(int));
		pipeline.setDepth();
		pipeline.clearDepth();
		pipeline.clearColour();
		pipeline.setViewport(W, H);
		SceneObject::m_usePrivateProgramme = true;

		// draw meshes in -unsorted- way
		for(SceneObject::SceneObjects::iterator it = scene.begin(); it != scene.end(); it++)
		{
			it->second.draw(pipeline);
		}

		// flip back buffer to screen, and front buffer to back buffer for next drawing
		pipeline.swapBuffers();

		// calculate and display frame rate
		fcount++;
		DWORD timeSpan = GetTickCount() - time0;
		if(timeSpan > 2000)
		{
			char frate[1024];
			sprintf_s(frate, 1024, "frate=%.1f, campos=(%.2f, %.2f, %.2f) cam-ypr=(%.2f, %.2f, %.2f)", 
				1000.0f * (float)fcount / (float)timeSpan, 
				cameraPos.x, cameraPos.y, cameraPos.z, 
				cameraYPR.x, cameraYPR.y, cameraYPR.z);
			SetWindowTextA(hWnd, frate);

			fcount = 0;
			time0 = GetTickCount();
		}

		//////////////////////////////////////////////////////////////////////////
		// process user input and remake view matrix
		// sorry I really don't have time to keep the following code tidy
		//////////////////////////////////////////////////////////////////////////

		float dyaw, dpitch, deltaMouse = 0.2f * (float)highTimer.span() / 1000.0f;
		input.getRelPos(&dyaw, &dpitch);
		cameraYPR.x += dyaw * deltaMouse;
		cameraYPR.y += dpitch * deltaMouse;

		float movement = 0.6f * (float)highTimer.span() / 1000.0f;

		vec4 baseVec;
		if(input.keyDown('A'))
		{
			baseVec.set(-1, 0, 0, 0);
		}
		else if(input.keyDown('D'))
		{
			baseVec.set(1, 0, 0, 0);
		}

		if(input.keyDown('W'))
		{
			baseVec.set(0, 0, -1, 0);
		}
		else if(input.keyDown('S'))
		{
			baseVec.set(0, 0, 1, 0);
		}

		if(input.keyDown(VK_SPACE))
		{
			cameraPos.y += movement;
		}
		else if(input.keyDown('C'))
		{
			cameraPos.y -= movement;
		}

		mat4 baseRotate;
		baseRotate.rotation(vec4(0, 1.0f, 0, 0), -cameraYPR.x);
		mcemaths_transform_m4v4_ip(baseVec, baseRotate);
		mcemaths_mul_3_4(baseVec, movement);

		mcemaths_add_3_4_ip(cameraPos, baseVec);
		
		input.frameUpdate();

		highTimer.reset();

		if(input.keyDown(VK_F12))
		{
			pipeline.saveTexture(-2, L"c:\\scene.bmp", false);
			pipeline.saveTextureAsRaw(-1, L"c:\\depth.raw");
			Sleep(500);
		}
	}

	input.shutdown();

	return 0;
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