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
#include "defproc.h"
#include "picldr.h"
#include "rndrddraw.h"
#include "libobjx.h"

using namespace Gdiplus;
using namespace std;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

const int W = 800;
const int H = 600;

const float PI = 3.1415927f;


int tex;

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

	PuresoftPipeline pipeline((uintptr_t)hWnd, W, H, new PuresoftDDrawRenderer);

/*
	pipeline.useProgramme(pipeline.createProgramme(
		pipeline.addProcessor(new VertexProcesserDEF01), 
		pipeline.addProcessor(new InterpolationProcessorDEF01), 
		pipeline.addProcessor(new FragmentProcessorDEF01)));
*/

	pipeline.useProgramme(pipeline.createProgramme(
		pipeline.addProcessor(new VertexProcesserDEF03), 
		pipeline.addProcessor(new InterpolationProcessorDEF03), 
		pipeline.addProcessor(new FragmentProcessorDEF03)));

	mat4 model, view, proj;
	mcemaths_make_proj_perspective(proj, 1.0f, 300.0f, (float)W / H, 2 * PI * (30.0f / 360.0f));

	mat4 proj_view;
	mcemaths_transform_m4m4(proj_view, proj, view);
	pipeline.setUniform(0, proj_view, sizeof(proj_view.elem));

	mat4 rot, tran, scale;
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

	//	HOBJXIO objx = open_objx(_T("box.objx"));
		HOBJXIO objx = open_objx(_T("sphere1.objx"));
	//	HOBJXIO objx = open_objx(_T("greek_vase2.objx"));
	mesh_info mi = {0};
	read_mesh_header(objx, mi);
	mi.vertices = new vec4[mi.num_vertices];
	mi.normals = new vec4[mi.num_vertices];
	mi.texcoords = new vec2[mi.num_vertices];
	if(mi.has_texcoords) // force tangent generation if having texcoords
		mi.tangents = new vec4[mi.num_vertices];
	read_mesh(objx, mi);
	close_objx(objx);

	vec4* binormal = NULL;
	if(mi.tangents)
	{
		binormal = new vec4[mi.num_vertices];
		for(unsigned int i = 0; i < mi.num_vertices; i++)
		{
			mcemaths_cross_3(binormal[i], mi.normals[i], mi.tangents[i]);
			mcemaths_norm_3_4(binormal[i]);
		}
	}

	PuresoftVBO vertices(16, mi.num_vertices), normals(16, mi.num_vertices), binormals(16, mi.num_vertices), tangents(16, mi.num_vertices), texcoords(8, mi.num_vertices);
	PuresoftVAO vao1, vao2;
	vao1.attachVBO(0, &vertices);
	vao1.attachVBO(1, &tangents);
	vao1.attachVBO(2, &binormals);
	vao1.attachVBO(3, &normals);
	vao1.attachVBO(4, &texcoords);

	for(unsigned int i = 0; i < mi.num_vertices; i++)
	{
		mi.vertices[i].w = 1.0f;
	}

	vertices.updateContent(mi.vertices);
	normals.updateContent(mi.normals);
	texcoords.updateContent(mi.texcoords);
	if(mi.tangents)
		tangents.updateContent(mi.tangents);
	if(binormal)
		binormals.updateContent(binormal);

	delete[] mi.vertices;
	delete[] mi.normals;
	delete[] mi.texcoords;
	if(mi.tangents)
		delete[] mi.tangents;
	if(binormal)
		delete[] binormal;

	PURESOFTIMGBUFF32 image;
	PuresoftDefaultPictureLoader picLoader;
	picLoader.loadFromFile(CA2W(mi.tex_file.c_str()), &image);
	image.pixels = malloc(image.scanline * image.height);
	picLoader.retrievePixel(&image);
	int tex = pipeline.createTexture(&image);
	free(image.pixels);
	picLoader.close();
	pipeline.setUniform(6, &tex, sizeof(tex));

	picLoader.loadFromFile(L"earth.dot3.png", &image);
//	picLoader.loadFromFile(CA2W(mi.bump_file.c_str()), &image);
	image.pixels = malloc(image.scanline * image.height);
	picLoader.retrievePixel(&image);
	tex = pipeline.createTexture(&image);
	free(image.pixels);
	picLoader.close();
	pipeline.setUniform(7, &tex, sizeof(tex));

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

		rot.rotation(vec4(0, 1.0f, 0, 0), rotRad);
		//scale.scaling(1.0f, 1.0f, 1.0f);
		//tran.translation(0, 0, -150.0f);
		tran.translation(0, 0, -1.2f);
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
