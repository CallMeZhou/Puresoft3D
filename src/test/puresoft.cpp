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

using namespace Gdiplus;
using namespace std;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static void updateTexMatrix(mat4& m, float rad);

const int W = 1024;
const int H = 576;

const int SHDW_W = 640;
const int SHDW_H = 640;

const float PI = 3.1415927f;

int tex;

class HighResolutionTimeCounter
{
	LARGE_INTEGER start;
	LARGE_INTEGER freq;
public:
	HighResolutionTimeCounter() 
	{ QueryPerformanceFrequency(&freq); Start();}
	void Start() 
	{ QueryPerformanceCounter(&start); }
	__int64 Now()
	{ 
		LARGE_INTEGER end = { 0 };
		QueryPerformanceCounter(&end);
		return (end.QuadPart - start.QuadPart)*1000 / freq.QuadPart;
	}
};

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

	int skyProc = pipeline.createProgramme(
		pipeline.addProcessor(new VertexProcesserDEF04), 
		pipeline.addProcessor(new InterpolationProcessorDEF04), 
		pipeline.addProcessor(new FragmentProcessorDEF04));

	int fullProc = pipeline.createProgramme(
		pipeline.addProcessor(new VertexProcesserTEST), 
		pipeline.addProcessor(new InterpolationProcessorTEST), 
		pipeline.addProcessor(new FragmentProcessorTEST));

	int simpTexProc = pipeline.createProgramme(
		pipeline.addProcessor(new VertexProcesserDEF03), 
		pipeline.addProcessor(new InterpolationProcessorDEF03), 
		pipeline.addProcessor(new FragmentProcessorDEF03));

	mat4 model, view, proj;
	mcemaths_make_proj_perspective(proj, 1.0f, 300.0f, (float)W / H, 2 * PI * (30.0f / 360.0f));

	//view.rotation(vec4(0, 1.0f, 0, 0), PI/4.0f);

	pipeline.setUniform(0, proj, sizeof(proj.elem));
	pipeline.setUniform(1, view, sizeof(view.elem));

	mat4 proj_view;
	mcemaths_transform_m4m4(proj_view, proj, view);
	pipeline.setUniform(3, proj_view, sizeof(proj_view.elem));

	mat4 rot, tran, scale;
	rot.rotation(vec4(0, 1.0f, 0, 0), 0);//-PI/4.0f);
	tran.translation(0, 0, -2.0f);
	mcemaths_transform_m4m4(model, tran, rot);
	pipeline.setUniform(4, model, sizeof(model.elem));

	mat4 modelRotate = rot;
	pipeline.setUniform(5, modelRotate, sizeof(modelRotate.elem));

	RGBQUAD singleColour = {200, 100, 150, 0};
	pipeline.setUniform(6, &singleColour, sizeof(singleColour));

	vec4 lightPos(-6.0f, 0, 1.0f, 0);
	pipeline.setUniform(7, &lightPos, sizeof(lightPos));

	vec4 cameraPos;
	pipeline.setUniform(8, &cameraPos, sizeof(cameraPos));

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

	__declspec(align(16)) float fullsquare[] = 
	{
		-1.0f,  1.0f,  0,  1.0f, 
		-1.0f, -1.0f,  0,  1.0f, 
		 1.0f, -1.0f,  0,  1.0f, 
		 1.0f, -1.0f,  0,  1.0f, 
		 1.0f,  1.0f,  0,  1.0f, 
		-1.0f,  1.0f,  0,  1.0f, 
	};
	PuresoftVBO vertices2(16, 6);
	vertices2.updateContent(fullsquare);
	vao2.attachVBO(0, &vertices2);

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
	int texEarthDiff = pipeline.createTexture(&image);
	pipeline.getTexture(texEarthDiff, &image);
	picLoader.retrievePixel(&image);
	picLoader.close();

	picLoader.loadFromFile(L"earth.dot3.png", &image);
	int texEarthBump = pipeline.createTexture(&image);
	pipeline.getTexture(texEarthBump, &image);
	picLoader.retrievePixel(&image);
	picLoader.close();

	picLoader.loadFromFile(L"earth.spac.png", &image);
	int tex = pipeline.createTexture(&image);
	pipeline.getTexture(tex, &image);
	picLoader.retrievePixel(&image);
	picLoader.close();
	pipeline.setUniform(11, &tex, sizeof(tex));

	picLoader.loadFromFile(L"earth.night.png", &image);
	tex = pipeline.createTexture(&image);
	pipeline.getTexture(tex, &image);
	picLoader.retrievePixel(&image);
	picLoader.close();
	pipeline.setUniform(12, &tex, sizeof(tex));

	picLoader.loadFromFile(L"earth.cloud.png", &image);
	tex = pipeline.createTexture(&image);
	pipeline.getTexture(tex, &image);
	picLoader.retrievePixel(&image);
	picLoader.close();
	pipeline.setUniform(13, &tex, sizeof(tex));

	picLoader.loadFromFile(L"moon.jpg", &image);
	int texMoonDiff = pipeline.createTexture(&image);
	pipeline.getTexture(texMoonDiff, &image);
	picLoader.retrievePixel(&image);
	picLoader.close();

	picLoader.loadFromFile(L"earth.dot3.png", &image);
	int texMoonBump = pipeline.createTexture(&image);
	pipeline.getTexture(texMoonBump, &image);
	picLoader.retrievePixel(&image);
	picLoader.close();

	const wchar_t* skyboxFiles[] = 
	{
		  L"purplenebula_rt.png" // xpos
		, L"purplenebula_lf.png" // xneg
		, L"purplenebula_up.png" // ypos
		, L"purplenebula_dn.png" // yneg
		, L"purplenebula_ft.png" // zpos
		, L"purplenebula_bk.png" // zneg
	};

	picLoader.loadFromFile(skyboxFiles[0], &image);
	image.pixels = NULL;
	tex = pipeline.createTexture(&image, 5);
	pipeline.setUniform(2, &tex, sizeof(tex));
	pipeline.getTexture(tex, &image, PuresoftFBO::LAYER_XPOS);
	picLoader.retrievePixel(&image);
	picLoader.close();
	for(int i = 1; i < 6; i++)
	{
		picLoader.loadFromFile(skyboxFiles[i], &image);
		pipeline.getTexture(tex, &image, (PuresoftFBO::LAYER)i);
		picLoader.retrievePixel(&image);
		picLoader.close();
	}

	mat4 texMatrix;
	//updateTexMatrix(texMatrix, 0);
	pipeline.setUniform(14, texMatrix, sizeof(texMatrix.elem));

	mat4 light1View, light1Proj;
	mcemaths_make_proj_perspective(light1Proj, 1.0f, 300.0f, (float)SHDW_W / SHDW_H, PI / 2.0f);
	mcemaths_make_view_traditional(light1View, cameraPos, tran, vec4(0, 1.0f, 0, 0));
	image.width = SHDW_W;
	image.height = SHDW_H;
	image.elemLen = 4;
	image.scanline = SHDW_W * 4;
	image.pixels = NULL;
	int texShadow = pipeline.createTexture(&image);

	//////////////////////////////////////////////////////////////////////////
	// run main window's message loop
	//////////////////////////////////////////////////////////////////////////
	HighResolutionTimeCounter highTimer;
	highTimer.Start();

	float rotRad = 0, trotrad = 0, rotRad3 = 2 * PI;

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

		pipeline.setUniform(0, light1Proj, sizeof(light1Proj.elem));
		pipeline.setUniform(1, light1View, sizeof(light1View.elem));
		pipeline.setDepth(texShadow);
		pipeline.setViewport(SHDW_W, SHDW_H);


		pipeline.setUniform(0, proj, sizeof(proj.elem));
		pipeline.setUniform(1, view, sizeof(view.elem));
		pipeline.setDepth();
		pipeline.setViewport(W, H);

		pipeline.useProgramme(skyProc);

		pipeline.disable(BEHAVIOR_UPDATE_DEPTH | BEHAVIOR_TEST_DEPTH);
		pipeline.drawVAO(&vao2, true);
		pipeline.enable(BEHAVIOR_UPDATE_DEPTH | BEHAVIOR_TEST_DEPTH);

		pipeline.useProgramme(fullProc);

		rot.rotation(vec4(0, 1.0f, 0, 0), rotRad);
		//scale.scaling(1.0f, 1.0f, 1.0f);
		//tran.translation(0, 0, -150.0f);
		tran.translation(0, 0, -1.2f);
		mcemaths_transform_m4m4(model, rot, scale);
		mcemaths_transform_m4m4_r_ip(tran, model);
		pipeline.setUniform(4, model, sizeof(model.elem));
		mat4 modelRotate = rot;
		pipeline.setUniform(5, modelRotate, sizeof(modelRotate.elem));

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

static void updateTexMatrix(mat4& m, float rad)
{
	mat4 push, pop, rot, temp;
	push.translation(vec4(0.5f, 0.5f, 0, 0));
	pop.translation(vec4(-0.5f, -0.5f, 0, 0));
	rot.rotation(vec4(0, 0, 1.0f, 0), rad);
	
	mcemaths_transform_m4m4(temp, rot, pop);
	mcemaths_transform_m4m4(m, push, temp);
}