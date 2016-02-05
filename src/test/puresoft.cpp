#include <windowsx.h>
#include <atlbase.h>
#include <atlconv.h>
#include <WTypes.h>
#include <tchar.h>
#include <GdiPlus.h>

#include "fixvec.hpp"
#include <map>

#include "pipeline.h"
#include "rndrddraw.h"
#include "testvp.h"
#include "testip.h"
#include "testfp.h"
#include "libobjx.h"
#include "samplr2d.h"

using namespace Gdiplus;
using namespace std;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

const int W = 640;
const int H = 480;

const float PI = 3.1415927f;

mat4 proj, view, model;
PuresoftVAO vao1, vao2;

int APIENTRY _tWinMain(HINSTANCE inst, HINSTANCE, LPTSTR, int nCmdShow)
{
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

	HWND hWnd = CreateWindow(_T("mainwnd"), _T("test"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, 
		CW_USEDEFAULT, 0, NULL, NULL, inst, NULL);
	ShowWindow(hWnd, nCmdShow);

	//////////////////////////////////////////////////////////////////////////
	// initialize renderer
	//////////////////////////////////////////////////////////////////////////
	USES_CONVERSION;

	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	PuresoftPipeline pipeline((uintptr_t)hWnd, W, H, new PuresoftDDrawRenderer);

	PuresoftProcessor proc(&MyTestVertexProcesser::createInstance, &MyTestInterpolationProcessor::createInstance, &MyTestFragmentProcessor::createInstance, sizeof(MYTESTPROCDATA));
	pipeline.setProcessor(&proc);

	mcemaths_make_proj_perspective(proj, 1.0f, 300.0f, (float)W / H, 2 * PI * (30.0f / 360.0f));

	mat4 proj_view;
	mcemaths_transform_m4m4(proj_view, proj, view);
	pipeline.setUniform(0, proj_view, sizeof(proj_view.elem));
	mat4 rot, tran;
	rot.rotation(vec4(0, 1.0f, 0, 0), 0);//-PI/4.0f);
	tran.translation(0, 0, -2.0f);
	mcemaths_transform_m4m4(model, tran, rot);
	pipeline.setUniform(1, model, sizeof(model.elem));
	mat4 modelRotate = rot;
	pipeline.setUniform(2, modelRotate, sizeof(modelRotate.elem));
	RGBQUAD singleColour = {200, 100, 150, 0};
	pipeline.setUniform(3, &singleColour, sizeof(singleColour));
	vec4 lightPos(-0.5f, 0, 0, 0);
	pipeline.setUniform(4, &lightPos, sizeof(lightPos));
	vec4 cameraPos;
	pipeline.setUniform(5, &cameraPos, sizeof(cameraPos));
	int diffuseTex = 0;
	pipeline.setUniform(6, &diffuseTex, sizeof(diffuseTex));
	//	HOBJXIO objx = open_objx(_T("box.objx"));
	//	HOBJXIO objx = open_objx(_T("sphere1.objx"));
	HOBJXIO objx = open_objx(_T("greek_vase2.objx"));
	mesh_info mi = {0};
	read_mesh_header(objx, mi);
	mi.vertices = new vec4[mi.num_vertices];
	mi.normals = new vec4[mi.num_vertices];
	mi.texcoords = new vec2[mi.num_vertices];
	read_mesh(objx, mi);
	close_objx(objx);

	PuresoftVBO vertices(16, mi.num_vertices), normals(16, mi.num_vertices), texcoords(8, mi.num_vertices);
	//	PuresoftVBO vertices(16, 3), normals(16, 3), texcoords(8, 3);
	vao1.attachVBO(0, &vertices);
	vao1.attachVBO(1, &normals);
	vao1.attachVBO(2, &texcoords);

	for(unsigned int i = 0; i < mi.num_vertices; i++)
	{
		mi.vertices[i].w = 1.0f;
		mi.normals[i].w = 1.0f;
	}

	const float testTriangleVerts[] = {
		-1.0f, -1.0f, 0.0f, 1.0f, 
		1.0f,  1.0f, 0.0f, 1.0f,
		-1.0f,  1.0f, 0.0f, 1.0f
	};
	const float testTriangleNorms[] = {
		0.0f, 0.0f, 1.0f, 1.0f, 
		0.0f, 0.0f, 1.0f, 1.0f, 
		0.0f, 0.0f, 1.0f, 1.0f, 
	};
	const float testTriangleTxcds[] = {
		0.0f, 0.0f, 
		1.0f, 1.0f, 
		0.0f, 1.0f, 
	};
	vertices.updateContent(mi.vertices);
	normals.updateContent(mi.normals);
	texcoords.updateContent(mi.texcoords);
	// 	vertices.updateContent(testTriangleVerts);
	// 	normals.updateContent(testTriangleNorms);
	// 	texcoords.updateContent(testTriangleTxcds);

	delete[] mi.vertices;
	delete[] mi.normals;
	delete[] mi.texcoords;

	//	Bitmap* diffusePic = Bitmap::FromFile(L"icon.png");
	//	Bitmap* diffusePic = Bitmap::FromFile(L"earth.jpg");
	Bitmap* diffusePic = Bitmap::FromFile(CA2W(mi.tex_file.c_str()));
	diffusePic->RotateFlip(RotateNoneFlipY);
	Rect r(0, 0, diffusePic->GetWidth(), diffusePic->GetHeight());
	BitmapData bmpdata;
	diffusePic->LockBits(&r, ImageLockModeRead, PixelFormat32bppRGB, &bmpdata);
	PuresoftSampler2D diffuseSmplr(bmpdata.Width, bmpdata.Stride, bmpdata.Height, 4, bmpdata.Scan0);
	pipeline.setTexture(0, &diffuseSmplr);

	//////////////////////////////////////////////////////////////////////////
	// run main window's message loop
	//////////////////////////////////////////////////////////////////////////
	DWORD time0 = GetTickCount(), fcount = 0;
	MSG msg;
	while (true)
	{
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if(WM_QUIT == msg.message)
			break;

		static float rotRad = 0;
		rotRad += 0.05f;
		if(rotRad > 2 * PI)
		{
			rotRad = 0;
		}

		mat4 rot, tran, scale;
		rot.rotation(vec4(0, 1.0f, 0, 0), rotRad);
		//scale.scaling(1.0f, 1.0f, 1.0f);
		tran.translation(0, 0, -150.0f);
		//tran.translation(0, 0, -1.0f);
		mcemaths_transform_m4m4(model, rot, scale);
		mcemaths_transform_m4m4_r_ip(tran, model);
		pipeline.setUniform(1, model, sizeof(model.elem));
		mat4 modelRotate = rot;
		pipeline.setUniform(2, modelRotate, sizeof(modelRotate.elem));

		pipeline.clearDepth();
		pipeline.clearColour();
		pipeline.drawVAO(&vao1);

		pipeline.swapBuffers();

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

	diffusePic->UnlockBits(&bmpdata);
	delete diffusePic;
	GdiplusShutdown(gdiplusToken);
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
